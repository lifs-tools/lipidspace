#include "lipidspace/hausdorff_cuda.cuh"

#include <cuda_runtime.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <vector>

// ---------------------------------------------------------------------------
// Device helper: atomic min for double via compare-and-swap.
// CUDA provides atomicMin only for integer types; doubles need a CAS loop.
// ---------------------------------------------------------------------------
__device__ static void atomic_min_double(double* addr, double val)
{
    unsigned long long* addr_ull =
        reinterpret_cast<unsigned long long*>(addr);
    unsigned long long assumed;
    unsigned long long old = *addr_ull;
    do {
        assumed = old;
        if (__longlong_as_double(assumed) <= val) return;
        old = atomicCAS(addr_ull, assumed, __double_as_longlong(val));
    } while (assumed != old);
}

// ---------------------------------------------------------------------------
// Hausdorff kernel — one CUDA block per pair (i, j).
//
// Dynamic shared memory layout (both halves sized max_ncols):
//   [ row_min[0..p-1] | col_min[0..q-1] ]
// where p = ncols[li], q = ncols[lj].
//
// Each thread handles a stripe of the p×q squared-distance matrix via a
// grid-stride loop, updating row and column minimums atomically in shared
// memory.  Thread 0 then finalises the scalar Hausdorff value.
// ---------------------------------------------------------------------------
__global__ void hausdorff_kernel(
    const double* __restrict__ all_data,
    const int*    __restrict__ offsets,
    const int*    __restrict__ ncols,
    int                        nrows,
    const int*    __restrict__ pair_i,
    const int*    __restrict__ pair_j,
    double*                    result)
{
    const int pair = blockIdx.x;
    const int li   = pair_i[pair];
    const int lj   = pair_j[pair];

    const double* A = all_data + offsets[li];   // nrows × ncols[li]
    const double* B = all_data + offsets[lj];   // nrows × ncols[lj]
    const int p = ncols[li];
    const int q = ncols[lj];

    extern __shared__ double sdata[];
    double* row_min = sdata;
    double* col_min = sdata + p;

    for (int t = threadIdx.x; t < p; t += blockDim.x) row_min[t] = 1e300;
    for (int t = threadIdx.x; t < q; t += blockDim.x) col_min[t] = 1e300;
    __syncthreads();

    // Grid-stride loop over all (a, b) combinations
    const int total = p * q;
    for (int k = threadIdx.x; k < total; k += blockDim.x) {
        const int a = k / q;
        const int b = k % q;

        const double* Aa = A + a * nrows;
        const double* Bb = B + b * nrows;
        double d = 0.0;
        for (int r = 0; r < nrows; ++r) {
            const double diff = Aa[r] - Bb[r];
            d += diff * diff;
        }
        atomic_min_double(&row_min[a], d);
        atomic_min_double(&col_min[b], d);
    }
    __syncthreads();

    // Thread 0 reduces row_min and col_min to a single Hausdorff value.
    // p and q are small (<<1000 lipids/lipidome) so the serial loop is fast.
    if (threadIdx.x == 0) {
        double h = 0.0;
        for (int a = 0; a < p; ++a) h = fmax(h, row_min[a]);
        for (int b = 0; b < q; ++b) h = fmax(h, col_min[b]);
        result[pair] = sqrt(h);
    }
}

// ---------------------------------------------------------------------------
// Public interface
// ---------------------------------------------------------------------------

bool cuda_is_available()
{
    int count = 0;
    return cudaGetDeviceCount(&count) == cudaSuccess && count > 0;
}

bool compute_hausdorff_matrix_cuda(
    const std::vector<const double*>& h_matrices,
    const std::vector<int>&           ncols,
    int                               nrows,
    double*                           h_out,
    int                               n)
{
    if (n <= 1) return true;

    // ---- Layout: concatenate all lipidome matrices into one flat buffer ----
    std::vector<int> h_offsets(n);
    int total_elements = 0;
    int max_ncols      = 0;
    for (int i = 0; i < n; ++i) {
        h_offsets[i]   = total_elements;
        total_elements += nrows * ncols[i];
        if (ncols[i] > max_ncols) max_ncols = ncols[i];
    }

    // Sanity check: ensure per-block shared memory requirement fits the device.
    // Each block needs (2 × max_ncols) doubles of dynamic shared memory.
    const size_t smem_bytes = 2 * static_cast<size_t>(max_ncols) * sizeof(double);
    {
        int dev = 0;
        cudaDeviceProp prop{};
        if (cudaGetDevice(&dev)              != cudaSuccess ||
            cudaGetDeviceProperties(&prop, dev) != cudaSuccess ||
            smem_bytes > static_cast<size_t>(prop.sharedMemPerBlockOptin))
        {
            fprintf(stderr,
                    "hausdorff_cuda: shared memory required (%zu B) exceeds "
                    "device limit; falling back to CPU.\n", smem_bytes);
            return false;
        }
    }

    // ---- Build pair index arrays ----
    const int n_pairs = n * (n - 1) / 2;
    std::vector<int> h_pair_i(n_pairs), h_pair_j(n_pairs);
    {
        int k = 0;
        for (int i = 0; i < n - 1; ++i)
            for (int j = i + 1; j < n; ++j) {
                h_pair_i[k] = i;
                h_pair_j[k] = j;
                ++k;
            }
    }

    // ---- Flatten all matrix data ----
    std::vector<double> h_data(total_elements);
    for (int i = 0; i < n; ++i)
        std::copy(h_matrices[i],
                  h_matrices[i] + nrows * ncols[i],
                  h_data.data() + h_offsets[i]);

    // ---- Allocate device buffers ----
    double* d_data   = nullptr;
    int*    d_off    = nullptr;
    int*    d_ncols  = nullptr;
    int*    d_pair_i = nullptr;
    int*    d_pair_j = nullptr;
    double* d_result = nullptr;

    bool ok = true;

    auto cleanup = [&]() {
        if (d_data)   cudaFree(d_data);
        if (d_off)    cudaFree(d_off);
        if (d_ncols)  cudaFree(d_ncols);
        if (d_pair_i) cudaFree(d_pair_i);
        if (d_pair_j) cudaFree(d_pair_j);
        if (d_result) cudaFree(d_result);
    };

#define CU(call)                                                              \
    do {                                                                      \
        cudaError_t _e = (call);                                             \
        if (_e != cudaSuccess) {                                             \
            fprintf(stderr, "hausdorff_cuda error %s:%d — %s\n",            \
                    __FILE__, __LINE__, cudaGetErrorString(_e));             \
            ok = false;                                                      \
        }                                                                    \
    } while (0)

    if (ok) CU(cudaMalloc(&d_data,   total_elements * sizeof(double)));
    if (ok) CU(cudaMalloc(&d_off,    n              * sizeof(int)));
    if (ok) CU(cudaMalloc(&d_ncols,  n              * sizeof(int)));
    if (ok) CU(cudaMalloc(&d_pair_i, n_pairs        * sizeof(int)));
    if (ok) CU(cudaMalloc(&d_pair_j, n_pairs        * sizeof(int)));
    if (ok) CU(cudaMalloc(&d_result, n_pairs        * sizeof(double)));

    if (ok) CU(cudaMemcpy(d_data,   h_data.data(),    total_elements * sizeof(double), cudaMemcpyHostToDevice));
    if (ok) CU(cudaMemcpy(d_off,    h_offsets.data(), n              * sizeof(int),    cudaMemcpyHostToDevice));
    if (ok) CU(cudaMemcpy(d_ncols,  ncols.data(),     n              * sizeof(int),    cudaMemcpyHostToDevice));
    if (ok) CU(cudaMemcpy(d_pair_i, h_pair_i.data(),  n_pairs        * sizeof(int),    cudaMemcpyHostToDevice));
    if (ok) CU(cudaMemcpy(d_pair_j, h_pair_j.data(),  n_pairs        * sizeof(int),    cudaMemcpyHostToDevice));

    if (ok) {
        // Request maximum dynamic shared memory if the device supports it.
        CU(cudaFuncSetAttribute(hausdorff_kernel,
                                cudaFuncAttributeMaxDynamicSharedMemorySize,
                                static_cast<int>(smem_bytes)));

        hausdorff_kernel<<<n_pairs, 256, smem_bytes>>>(
            d_data, d_off, d_ncols, nrows,
            d_pair_i, d_pair_j,
            d_result);
        CU(cudaGetLastError());
        CU(cudaDeviceSynchronize());
    }

    // ---- Copy results back and fill symmetric output (column-major) ----
    if (ok) {
        std::vector<double> h_result(n_pairs);
        CU(cudaMemcpy(h_result.data(), d_result,
                      n_pairs * sizeof(double), cudaMemcpyDeviceToHost));
        if (ok) {
            int k = 0;
            for (int i = 0; i < n - 1; ++i)
                for (int j = i + 1; j < n; ++j) {
                    const double hd = h_result[k++];
                    h_out[i + j * n] = hd;  // column-major: row=i, col=j
                    h_out[j + i * n] = hd;  // column-major: row=j, col=i
                }
            for (int i = 0; i < n; ++i) h_out[i + i * n] = 0.0;
        }
    }

#undef CU

    cleanup();
    return ok;
}
