#include "lipidspace/Matrix.h"
#include <random>
#include <algorithm>
#include <vector>

// BLAS and LAPACK are only consumed by this translation unit.
#ifdef Q_OS_MACOS
// Accelerate provides cblas and LAPACK (dgetrf_/dgetri_) with identical
// calling conventions to OpenBLAS, and uses AMX hardware on Apple Silicon.
#  include <Accelerate/Accelerate.h>
#else
#  include <cblas.h>
extern "C" {
    void dgetrf_(int* M, int* N, double* A, int* lda, int* IPIV, int* INFO);
    void dgetri_(int* N, double* A, int* lda, int* IPIV, double* WORK, int* lwork, int* INFO);
}
#endif


Array::Array() : vector<double>(){
}

Array::Array(int len, double val) : vector<double> (len, val){
}

Array::Array(const Array &a, int l) : vector<double>() {
    if (l != -1){
        assert(0 < l && l <= (int)a.size());
        reserve(l);
        for (int i = 0; i < l; ++i) push_back(a.at(i));
    }
    else {
        reserve(a.size());
        for (auto val : a) push_back(val);
    }
}


double Array::sum(){
    double val = 0;
    for (int i = 0; i < (int)size(); ++i) val += at(i);
    return val;
}

Array& operator+=(Array &me, const double val){
    for (int i = 0; i < (int)me.size(); ++i) me.at(i) += val;
    return me;
}

Array& operator-=(Array &me, const double val){
    for (int i = 0; i < (int)me.size(); ++i) me.at(i) -= val;
    return me;
}

Array& operator*=(Array &me, const double val){
    for (int i = 0; i < (int)me.size(); ++i) me.at(i) *= val;
    return me;
}

Array& operator/=(Array &me, const double val){
    for (int i = 0; i < (int)me.size(); ++i) me.at(i) /= val;
    return me;
}


void Array::remove_below(double val){
    Array tmp;
    tmp.reset(*this);
    clear();
    for (double v : tmp){
        if (v >= val) push_back(v);
    }
}

void Array::mult(Matrix &m, Array &a){
    assert(m.cols == (int)a.size());
    clear();
    for (int r = 0; r < m.rows; ++r){
        double sum = 0;
        for (int c = 0; c < (int)a.size(); c++) sum += m(r, c) * a[c];
        push_back(sum);
    }
}


double Array::median(int begin_v, int end_v, bool sorted){
    assert(size() >= 1);
    if (size() == 1) return at(0);

    if (begin_v == -1) begin_v = 0;
    if (end_v == -1) end_v = size() - 1;
    if (!sorted) sort(begin(), end());

    int count = end_v - begin_v;
    if (count & 1) {
        return at((count >> 1) + begin_v);
    }
    else {
        double right = at((count >> 1) + begin_v);
        double left = at((count >> 1) - 1 + begin_v);
        return (right + left) / 2.0;
    }
}


int Array::greatest_less(double key, int L, int R) {
    assert(size() > 0);

    if (L < 0) L = 0;
    if (R < 0) R = size() - 1;
    int hit = L - 1;

    while (L <= R) {
        int mid = L + ((R - L + 1) >> 1);

        if (at(mid) <= key) {
            hit = mid;
            L = mid + 1;
        }
        else if (at(mid) > key) {
            R = mid - 1;
        }
    }

    return hit;
}


double Array::mean(){
    double mn = 0;
    for (int i = 0; i < (int)size(); ++i) mn += at(i);
    return mn / (double)size();
}


double Array::stdev(){
    double mn = mean(), st = 0;
    for (int i = 0; i < (int)size(); ++i) st += sq(at(i) - mn);
    return sqrt(st / (double)size());
}


double Array::sample_stdev(){
    double mn = mean(), st = 0;
    for (int i = 0; i < (int)size(); ++i) st += sq(at(i) - mn);
    return sqrt(st / ((double)size() - 1));
}

void Array::add(Array &a){
    reserve(size() + a.size());
    for (auto val : a) push_back(val);
}

void Array::add(vector<double> &a){
    reserve(size() + a.size());
    for (auto val : a) push_back(val);
}


void Array::compute_coefficiants(Matrix &data, Array &values){
    Matrix data_transpose;
    data_transpose.rewrite(data);
    data_transpose.transpose();

    Matrix dTd;
    dTd.mult(data_transpose, data);
    Matrix inversed;
    inversed.inverse(dTd, true);

    Matrix dTdidT;
    dTdidT.mult(inversed, data_transpose);

    mult(dTdidT, values);
}


void Array::reset(Array &a){
    clear();
    reserve(a.size());
    for (auto val : a) push_back(val);
}


void Array::compute_distances(Array &x, double dx, Array &y, double dy){
    assert(x.size() == y.size());
    resize(x.size());
    for (int i = 0 ; i < (int)x.size(); ++i) at(i) = sq(x[i] - dx) + sq(y[i] - dy);
}


Matrix::Matrix() : QObject(){
    rows = 0;
    cols = 0;
}

Matrix::Matrix(const Array &a, int _rows, int _cols) : QObject() {
    cols = _cols;
    rows = _rows;
    m.clear();
    m.reserve(cols * rows);
    for (double v : a) m.push_back(v);
}

Matrix& operator+=(Matrix &me, const double val){
    for (int i = 0; i < (int)me.m.size(); ++i) me.m.at(i) += val;
    return me;
}

Matrix& operator-=(Matrix &me, const double val){
    for (int i = 0; i < (int)me.m.size(); ++i) me.m.at(i) -= val;
    return me;
}

Matrix& operator*=(Matrix &me, const double val){
    for (int i = 0; i < (int)me.m.size(); ++i) me.m.at(i) *= val;
    return me;
}

Matrix& operator/=(Matrix &me, const double val){
    for (int i = 0; i < (int)me.m.size(); ++i) me.m.at(i) /= val;
    return me;
}

Matrix::Matrix(vector<vector<double>> &mat) : QObject(){
    rewrite(mat);
}


Matrix::Matrix(Matrix &mat, bool transpose) : QObject(){
    if (transpose) rewrite_transpose(mat);
    else rewrite(mat);
}


Matrix::Matrix(json &container) : QObject(){
    for (string field : {"rows", "cols", "m"}){
        if (container.find(field) == container.end()){
            throw LipidException("Error: field '" + field + "' not found in class 'Matrix'.");
        }
    }
    rows = container["rows"];
    cols = container["cols"];

    for (auto val : container["m"]) m.push_back(val);

    if (rows * cols != (int)m.size()){
        throw LipidException("Error: matrix dimensions do not fit with content.");
    }
}


void Matrix::clear(){
    m.clear();
    cols = 0;
    rows = 0;
}


void Matrix::load(json &container){
    for (string field : {"rows", "cols", "m"}){
        if (container.find(field) == container.end()){
            throw LipidException("Error: field '" + field + "' not found in class 'Matrix'.");
        }
    }
    rows = container["rows"];
    cols = container["cols"];

    m.clear();
    for (auto val : container["m"]) m.push_back(val);

    if (rows * cols != (int)m.size()){
        throw LipidException("Error: matrix dimensions do not fit with content.");
    }
}



void Matrix::save(json &container){
    container["rows"] = rows;
    container["cols"] = cols;
    for (auto val : m) container["m"] += val;
}


void Matrix::rewrite_transpose(Matrix &mat){
    cols = mat.rows;
    rows = mat.cols;
    m.clear();
    m.resize(cols * rows);
    for (int c = 0; c < mat.cols; c++){
        for (int r = 0; r < mat.rows; r++){
            (*this)(c, r) = mat(r, c);
        }
    }
}


void Matrix::rewrite(Matrix &mat, const Indexes &ri, const Indexes &ci){
    m.clear();
    if (ci.size() == 0 && ri.size() == 0){
        cols = mat.cols;
        rows = mat.rows;
        m.reserve(cols * rows);
        for (auto val : mat.m) m.push_back(val);
    }
    else if (ci.size() == 0){
        cols = mat.cols;
        rows = ri.size();
        m.reserve(cols * rows);
        for (int c = 0; c < cols; c++){
            for (auto r : ri){
                m.push_back(mat(r, c));
            }
        }
    }
    else if (ri.size() == 0){
        cols = ci.size();
        rows = mat.rows;
        m.reserve(cols * rows);
        for (auto c : ci){
            for (int r = 0; r < rows; r++){
                m.push_back(mat(r, c));
            }
        }
    }
    else {
        cols = ci.size();
        rows = ri.size();
        m.reserve(cols * rows);
        for (auto c : ci){
            for (auto r : ri){
                m.push_back(mat(r, c));
            }
        }
    }

}






void Matrix::add_column(Array &col){
    if (cols > 0){
        assert(rows == (int)col.size());
        cols += 1;
        m.reserve(cols * rows);
        for (auto val : col) m.push_back(val);
    }
    else {
        cols = 1;
        rows = col.size();
        m.reserve(rows);
        for (auto val : col) m.push_back(val);
    }
}



double Matrix::col_min(int c){
    assert(0 <= c && c < cols && rows > 0);
    double min_val = m[c * rows];
    for (int it = c * rows + 1; it < (c + 1) * rows; it++){
        min_val = mmin(min_val, m[it]);
    }
    return min_val;
}



double Matrix::col_max(int c){
    assert(0 <= c && c < cols && rows > 0);
    double max_val = m[c * rows];
    for (int it = c * rows + 1; it < (c + 1) * rows; it++){
        max_val = mmax(max_val, m[it]);
    }
    return max_val;
}



void Matrix::rewrite(vector<vector<double>> &mat){
    cols = mat.size();
    rows = mat[0].size();
    m.clear();
    m.reserve(cols * rows);
    for (auto col : mat){
        for (auto val : col) m.push_back(val);
    }
}


Matrix::Matrix(int _rows, int _cols){
    reset(_rows, _cols);
}

void Matrix::reset(int _rows, int _cols){
    rows = _rows;
    cols = _cols;
    m.clear();
    m.reserve(cols * rows);
    for (int i = 0; i < cols * rows; ++i) m.push_back(0);
}


double Matrix::pairwise_sum(Matrix &m){
    assert(m.cols == 2);
    Matrix tm(m, true);

    double dist_sum = 0;
    for (int tm2c = 0; tm2c < tm.cols; tm2c++){
        double* tm2col = tm.data() + (tm2c * tm.rows);
        for (int tm1c = 0; tm1c < tm.cols; ++tm1c){
            double* tm1col = tm.data() + (tm1c * tm.rows);
            double dist = sq(tm1col[0] - tm2col[0]);
            dist += sq(tm1col[1] - tm2col[1]);
            dist_sum += sqrt(dist);
        }
    }
    return dist_sum;
}




void Matrix::pad_cols_4(){
    if ((cols & 3) == 0) return;
    int add_cols = (4 - (cols & 3));
    cols += add_cols;
    m.reserve(cols * rows);
    for (int i = 0; i < add_cols * rows; ++i) m.push_back(0);
}




void Matrix::scale(){
    for (int c = 0; c < cols; c++){
        // estimating the mean
        double mean = 0;
        for (int it = c * rows; it < (c + 1) * rows; ++it){
            mean += m[it];
        }
        mean /= (double)rows;

        // estimating the standard deviation
        double stdev_inv = 0;
        for (int it = c * rows; it < (c + 1) * rows; ++it){
            stdev_inv += sq(mean - m[it]);
        }
        stdev_inv = sqrt((double)rows / stdev_inv);

        // performing z transformation, aka (x - mean) / st_dev
        for (int it = c * rows; it < (c + 1) * rows; ++it){
            m[it] = (m[it] - mean) * stdev_inv;
        }
    }
}


void Matrix::transpose(){
    double *tmp = new double[cols * rows];
    //#pragma omp parallel for
    for (int i = 0; i < cols * rows; ++i) tmp[i] = m[i];

    //#pragma omp parallel for collapse(2)
    for (int c = 0; c < cols; c++){
        for (int r = 0; r < rows; ++r){
            m[r * cols + c] = tmp[c * rows + r];
        }
    }
    swap(rows, cols);

    delete []tmp;
}


void Matrix::compute_eigen_data(Array &eigenvalues, Matrix& eigenvectors, int top_n){
    Matrix trans(*this, true);

    // Prepare matrix-vector multiplication routine used in Lanczos algorithm
    auto mv_mul = [&](const vector<double>& in, vector<double>& out) {
	cblas_dgemv(CblasColMajor, CblasNoTrans, rows, cols, 1.0, data(), rows, in.data(), 1, 0, out.data(), 1);
    };
    // Execute Lanczos algorithm
    LambdaLanczos<double> engine(mv_mul, rows, true); // true means to calculate the largest eigenvalue.
    vector<double> evals(top_n);
    vector<vector<double>> eigvecs(top_n);
    engine.run(evals, eigvecs);

    eigenvalues.add(evals);
    eigenvectors.rewrite(eigvecs);
}


void Matrix::inverse(Matrix &X, bool symmetric){
    assert(X.rows == X.cols);
    rewrite(X);
    if (!symmetric) transpose();


    int N = rows;
    int *IPIV = new int[N];
    int LWORK = N*N;
    double *WORK = new double[LWORK];
    int INFO;

    dgetrf_(&N, &N, m.data(), &N, IPIV, &INFO);
    dgetri_(&N, m.data(), &N, IPIV, WORK, &LWORK, &INFO);

    delete[] IPIV;
    delete[] WORK;
}


double Matrix::vector_vector_mult(int n, const double *x, const double *y){
    return cblas_ddot(n, x, 1, y, 1);
}



void Matrix::mult_vector(const vector<double> &in, vector<double> &out){
    assert(rows == (int)in.size() && cols == (int)out.size());
    for (int i = 0; i < (int)cols; ++i) out[i] = vector_vector_mult(rows, m.data() + i * rows, in.data());
}



void Matrix::mult(Matrix& A, Matrix& B, bool transA, bool transB, double alpha){

#ifndef _WIN32
    assert((transA ? A.rows : A.cols) == (transB ? B.cols : B.rows));

    int mm = transA ? A.cols : A.rows;
    int kk = transA ? A.rows : A.cols;
    int nn = transB ? B.rows : B.cols;
    reset(mm, nn);
    cblas_dgemm(CblasColMajor, transA ? CblasTrans : CblasNoTrans, transB ? CblasTrans : CblasNoTrans, mm, nn, kk, alpha, A.data(), A.rows, B.data(), B.rows, 0.0, data(), rows);

#else

    int tile_size = 16;
    Matrix mA(A, !transA);
    Matrix mB(B, transB);

    assert(mA.rows == mB.rows);

    int num_cols_A = mA.cols;
    int num_cols_B = mB.cols;
    reset(num_cols_A, num_cols_B);


#ifdef _MSC_VER
    #pragma omp parallel for
#else
    #pragma omp parallel for collapse(2)
#endif
    for (int i = 0; i < num_cols_A; i += tile_size) {
        for (int j = 0; j < num_cols_B; j += tile_size) {
            int it = min(num_cols_A, i + tile_size);
            int jt = min(num_cols_B, j + tile_size);
            for (int jj = j; jj < jt; ++jj){
                double *mm = m.data() + jj * rows;
                for (int ii = i; ii < it; ++ii){
                    mm[ii] += vector_vector_mult(mA.rows, mA.data() + ii * mA.rows, mB.data() + jj * mA.rows);
                }
            }
        }
    }
    if (alpha != 1.) *this *= alpha;
#endif
}


void Matrix::PCA(Matrix &pca, int dimensions){
    // Scale data
    scale();
    increment();

    // compute covariance matrix
    Matrix cov_matrix;
    covariance_matrix(cov_matrix);
    increment();

    // compute eigenvectors
    Array eigenvalues;
    Matrix eigenvectors;
    cov_matrix.compute_eigen_data(eigenvalues, eigenvectors, dimensions);
    increment();

    // multiply the eigenvectors with the original matrix
    pca.mult(eigenvectors, *this, true, true);
    pca.transpose();
    increment();
}


void Matrix::covariance_matrix(Matrix &covar){
    covar.reset(cols, cols);
    double factor = 1. / ((double)cols - 1);
    covar.mult(*this, *this, true, false, factor);
}


/**
 * SYRK covariance: C = factor * Z^T Z with factor = 1/(cols-1). Since Z^T Z is
 * symmetric, a single BLAS dsyrk computes one triangle at half the flops of the
 * general dgemm in covariance_matrix(); the other triangle is mirrored. Numerically
 * equivalent to covariance_matrix() (parity-tested in tests/tst_matrix_blas).
 * Windows keeps the dgemm path (its BLAS build routes GEMM through a tiled fallback).
 */
void Matrix::covariance_matrix_syrk(Matrix &covar){
    covar.reset(cols, cols);
    double factor = 1. / ((double)cols - 1);
#ifndef _WIN32
    cblas_dsyrk(CblasColMajor, CblasLower, CblasTrans, cols, rows, factor, data(), rows, 0.0, covar.data(), cols);
    // Mirror the computed lower triangle into the upper to complete the symmetric matrix.
    for (int i = 0; i < cols; ++i)
        for (int j = 0; j < i; ++j)
            covar(j, i) = covar(i, j);
#else
    covar.mult(*this, *this, true, false, factor);
#endif
}


double* Matrix::data(){
    return m.data();
}


// ============================================================================
// Fingerprint methods for Atlas
// Based on docs/atlas-phase0/evaluate.py
// ============================================================================

/**
 * K-means clustering on the rows of this matrix.
 * Each row is a data point in cols-dimensional space.
 * 
 * @param K Number of clusters
 * @param centers Output: K x cols matrix of cluster centers
 * @param max_iter Maximum iterations
 * @param seed Random seed for reproducibility
 */
void Matrix::kmeans(int K, Matrix& centers, int max_iter, unsigned long seed) {
    assert(rows > 0 && cols > 0);
    assert(K > 0 && K <= rows);
    
    // Initialize random number generator
    std::mt19937 gen(seed);
    std::uniform_int_distribution<int> dist(0, rows - 1);
    
    // Step 1: k-means++ initialization (Arthur & Vassilvitskii), matching the default
    // init strategy sklearn uses in the Python harness: the first center is a uniformly
    // random point; each subsequent center is sampled with probability proportional to
    // its squared distance to the nearest already-chosen center.
    std::uniform_real_distribution<double> unit(0.0, 1.0);
    centers.reset(K, cols);
    {
        int first = dist(gen);
        for (int c = 0; c < cols; ++c) centers(0, c) = (*this)(first, c);
    }
    std::vector<double> d2(rows, INFINITY);  // squared distance to the nearest center
    for (int k = 1; k < K; ++k) {
        double total = 0.0;
        for (int i = 0; i < rows; ++i) {
            double dist_sq = 0.0;
            for (int c = 0; c < cols; ++c) {
                double diff = (*this)(i, c) - centers(k - 1, c);
                dist_sq += diff * diff;
            }
            if (dist_sq < d2[i]) d2[i] = dist_sq;
            total += d2[i];
        }
        double target = unit(gen) * total;
        double acc = 0.0;
        int chosen = rows - 1;
        for (int i = 0; i < rows; ++i) {
            acc += d2[i];
            if (acc >= target) { chosen = i; break; }
        }
        for (int c = 0; c < cols; ++c) centers(k, c) = (*this)(chosen, c);
    }
    
    // Step 2: Iterate
    std::vector<int> assignments(rows);
    for (int iter = 0; iter < max_iter; ++iter) {
        // Assign each point to nearest center
        bool changed = false;
        for (int i = 0; i < rows; ++i) {
            double min_dist = INFINITY;
            int best_k = 0;
            for (int k = 0; k < K; ++k) {
                double dist = 0.0;
                for (int c = 0; c < cols; ++c) {
                    double diff = (*this)(i, c) - centers(k, c);
                    dist += diff * diff;
                }
                if (dist < min_dist) {
                    min_dist = dist;
                    best_k = k;
                }
            }
            if (assignments[i] != best_k) {
                assignments[i] = best_k;
                changed = true;
            }
        }
        
        if (!changed) break;
        
        // Update centers
        std::vector<int> counts(K, 0);
        std::vector<std::vector<double>> sums(K, std::vector<double>(cols, 0.0));
        
        for (int i = 0; i < rows; ++i) {
            int k = assignments[i];
            counts[k]++;
            for (int c = 0; c < cols; ++c) {
                sums[k][c] += (*this)(i, c);
            }
        }
        
        for (int k = 0; k < K; ++k) {
            if (counts[k] > 0) {
                for (int c = 0; c < cols; ++c) {
                    centers(k, c) = sums[k][c] / counts[k];
                }
            }
            // If empty cluster, reinitialize to random point
            else {
                int idx = dist(gen);
                for (int c = 0; c < cols; ++c) {
                    centers(k, c) = (*this)(idx, c);
                }
            }
        }
    }
}


/**
 * Generate a fingerprint for a single lipidome.
 * 
 * The fingerprint is a histogram over K modules, where each lipid's contribution
 * is weighted by its abundance and soft-assigned to modules based on distance.
 * 
 * @param centers K x dims matrix of module centers (from kmeans on frame)
 * @param weights Array of lipid weights (abundances)
 * @param fingerprint Output: Array of length K (the fingerprint vector)
 * @param s Global temperature parameter (computed from frame)
 * @param soft If true, use soft assignment; if false, use hard assignment
 */
void Matrix::generate_fingerprint(Matrix& centers, Array& weights, Array& fingerprint,
                                  double s, bool soft, Matrix* contributions) {
    int K = centers.rows;
    int D = cols;          // (lipids x dims): columns are PCA dimensions
    int n_lipids = rows;   // rows are lipids (points in the frame)

    fingerprint.resize(K);
    for (int k = 0; k < K; ++k) fingerprint[k] = 0.0;

    if (contributions) contributions->reset(n_lipids, K);   // reset() zero-fills the store

    // For each lipid (row) in this lipidome matrix
    for (int i = 0; i < n_lipids; ++i) {
        double weight = (i < (int)weights.size()) ? weights[i] : 1.0;
        if (weight <= 0) continue;
        
        // Find distances from this lipid's position to all centers
        std::vector<double> dists(K);
        
        for (int k = 0; k < K; ++k) {
            double dist_sq = 0.0;
            for (int c = 0; c < D; ++c) {
                double diff = (*this)(i, c) - centers(k, c);
                dist_sq += diff * diff;
            }
            dists[k] = sqrt(dist_sq);
        }
        
        if (soft) {
            // per-lipid soft assignment (softmax over centers), then abundance-weighted
            double sum_exp = 0.0;
            for (int k = 0; k < K; ++k) {
                dists[k] = exp(-(dists[k] * dists[k]) / (2 * s * s));
                sum_exp += dists[k];
            }
            if (sum_exp > 0) {
                for (int k = 0; k < K; ++k) {
                    double ck = weight * dists[k] / sum_exp;
                    fingerprint[k] += ck;
                    if (contributions) (*contributions)(i, k) = ck;
                }
            }
        } else {
            // Hard assignment: find nearest center
            int best_k = 0;
            for (int k = 1; k < K; ++k) {
                if (dists[k] < dists[best_k]) best_k = k;
            }
            fingerprint[best_k] += weight;
            if (contributions) (*contributions)(i, best_k) = weight;
        }
    }

    // Normalize fingerprint to sum to 1 (compositional)
    double sum = fingerprint.sum();
    if (sum > 0) {
        for (int k = 0; k < K; ++k) {
            fingerprint[k] /= sum;
        }
    }
    if (contributions && sum > 0) {
        for (int i = 0; i < n_lipids; ++i)
            for (int k = 0; k < K; ++k) (*contributions)(i, k) /= sum;
    }
}


/**
 * Hellinger distance between two probability distributions:
 *   H(p, q) = ||sqrt(p) - sqrt(q)||_2 / sqrt(2),  in [0, 1].
 * A proper metric and a numerically stable proxy for the (square-root) Jensen-Shannon
 * distance used by the Atlas Phase 0 fingerprints (docs/atlas-phase0/evaluate.py).
 * 
 * @param a First probability distribution
 * @param b Second probability distribution
 * @return Hellinger distance (a metric in [0, 1])
 */
double Matrix::hellinger_distance(const Array& a, const Array& b) {
    assert(a.size() == b.size());
    
    double sum_sq = 0.0;
    for (size_t i = 0; i < a.size(); ++i) {
        double diff = sqrt(a[i]) - sqrt(b[i]);
        sum_sq += diff * diff;
    }
    
    // Hellinger distance = sqrt(0.5 * sum(...))
    // But we return the Euclidean distance on square roots divided by sqrt(2)
    // which equals the Hellinger distance
    return sqrt(sum_sq) / sqrt(2.0);
}


/**
 * BLAS/GEMM reformulation of the pairwise Hellinger distance matrix.
 *
 * Let S be the (n x K) matrix of element-wise square roots of the fingerprints,
 * S(i,k) = sqrt(p_i[k]). Then the Gram matrix G = S * S^T holds the Bhattacharyya
 * coefficients, G(i,j) = sum_k sqrt(p_i[k] * p_j[k]) = <sqrt(p_i), sqrt(p_j)>, and
 *
 *     sum_k (sqrt(p_i[k]) - sqrt(p_j[k]))^2 = G(i,i) + G(j,j) - 2*G(i,j),
 *     H(i,j) = sqrt( G(i,i) + G(j,j) - 2*G(i,j) ) / sqrt(2).
 *
 * This reproduces hellinger_distance() exactly (the general Gram form, not the
 * sqrt(1 - BC) shortcut, so it also holds for un-normalized fingerprints), while
 * replacing the O(n^2 * K) scalar double-loop with a single BLAS dgemm (via mult()).
 * On macOS this dispatches to Accelerate, on Linux to OpenBLAS, and — once the
 * covariance/GEMM path is compiled with cuBLAS — to the GPU.
 *
 * The scalar hellinger_distance() stays in place as the parity oracle
 * (tests/tst_matrix_blas).
 */
void Matrix::hellinger_matrix(const vector<Array>& fingerprints){
    int n = (int)fingerprints.size();
    reset(n, n);
    if (n == 0) return;
    int K = (int)fingerprints[0].size();

    // S (n x K): element-wise square roots of the fingerprints.
    Matrix S;
    S.reset(n, K);
    for (int i = 0; i < n; ++i){
        const Array& fp = fingerprints[i];
        int kk = mmin(K, (int)fp.size());
        for (int k = 0; k < kk; ++k) S(i, k) = sqrt(fp[k]);
    }

    // Gram matrix G = S * S^T (n x n) through the BLAS dgemm path.
    Matrix G;
    G.mult(S, S, false, true);

    // Recover Hellinger distances from the Gram matrix.
    const double inv_sqrt2 = 1.0 / sqrt(2.0);
    #pragma omp parallel for
    for (int i = 0; i < n; ++i){
        (*this)(i, i) = 0.0;
        for (int j = i + 1; j < n; ++j){
            double v = G(i, i) + G(j, j) - 2.0 * G(i, j);
            if (v < 0.0) v = 0.0;   // clamp tiny negative round-off
            double d = sqrt(v) * inv_sqrt2;
            (*this)(i, j) = d;
            (*this)(j, i) = d;
        }
    }
}


/**
 * Packaged OpenMP-parallel scalar pairwise Hellinger matrix -- the pre-BLAS path,
 * identical in result to hellinger_matrix() but built from the scalar
 * hellinger_distance(). Kept as the CPU fast path at small K (see the benchmark
 * crossover in tests/tst_matrix_blas) and as a parity sibling of the GEMM path.
 */
void Matrix::hellinger_matrix_scalar(const vector<Array>& fingerprints){
    int n = (int)fingerprints.size();
    reset(n, n);
    #pragma omp parallel for
    for (int i = 0; i < n; ++i){
        for (int j = i + 1; j < n; ++j){
            double d = hellinger_distance(fingerprints[i], fingerprints[j]);
            (*this)(i, j) = d;
            (*this)(j, i) = d;
        }
        (*this)(i, i) = 0.0;
    }
}


// Minimum fingerprint dimensionality (K) at which the GEMM path is expected to beat
// the OpenMP scalar loop on CPU. Below this the pairwise matrix is memory-bandwidth
// bound (skinny rank-K Gram) and the scalar loop wins; above it the GEMM's arithmetic
// intensity pays off. The measured crossover is ~K=46 on Apple M-series / 10 threads
// (tests/tst_matrix_blas benchmark_hellinger_backends); 64 leaves a safety margin.
#define HELLINGER_GEMM_MIN_K 64

// Placeholder for a future GPU BLAS backend. Once Matrix::mult() gains a cuBLAS path,
// the dispatcher should prefer the GEMM formulation unconditionally on GPU hosts
// (where the N*N Gram is cheap and no OpenMP scalar equivalent exists).
static inline bool hellinger_gemm_is_gpu_accelerated() {
    return false;
}

/**
 * Adaptive dispatch for the N x N Hellinger matrix. Chooses the GEMM path when a GPU
 * BLAS backend is active or when K is large enough to amortize the Gram; otherwise the
 * OpenMP scalar path, which is faster at the Atlas default K=20 on CPU. Both paths are
 * numerically equivalent and parity-tested against the scalar oracle.
 */
void Matrix::hellinger_matrix_auto(const vector<Array>& fingerprints){
    int n = (int)fingerprints.size();
    int K = n > 0 ? (int)fingerprints[0].size() : 0;
    if (hellinger_gemm_is_gpu_accelerated() || K >= HELLINGER_GEMM_MIN_K) {
        hellinger_matrix(fingerprints);        // BLAS GEMM (Accelerate / OpenBLAS / cuBLAS)
    } else {
        hellinger_matrix_scalar(fingerprints); // OpenMP scalar (faster at small K on CPU)
    }
}


/**
 * Compute pairwise JSD distances between all lipidomes based on fingerprints.
 * 
 * This follows the approach from docs/atlas-phase0/evaluate.py:
 * 1. Build K modules via k-means on the frozen frame
 * 2. For each lipidome, generate a fingerprint (soft-assigned histogram over modules)
 * 3. Compute JSD distance between all pairs of fingerprints
 * 
 * @param lipidome_matrixes Vector of lipidome matrices (each: lipids x dims)
 * @param frame The frozen PCA frame (lipids x dims)
 * @param K Number of modules/clusters
 * @param temperature Softmax temperature for soft assignment
 * @param soft Use soft (true) or hard (false) assignment
 */
void Matrix::compute_fingerprint_distance_matrix(vector<Matrix*>& lipidome_matrixes,
                                              vector<Array*>& lipidome_weights, 
                                              Matrix& frame, 
                                              int K, 
                                              double temperature, 
                                              bool soft) {
    int n = lipidome_matrixes.size();
    
    // Step 1: build modules via k-means on the frame.
    
    // frame is (lipids x dims): rows are lipid points, cols are PCA dimensions.
    // k-means clusters the lipid points directly -- no transpose needed.
    if (K > frame.rows) K = frame.rows;
    if (K < 1) K = 1;
    
    Matrix centers;
    frame.kmeans(K, centers);  // centers: K x dims
    
    // Step 2: Compute global temperature parameter s
    // s = temperature * median of min distances from frame lipids to centers + epsilon
    double s = temperature;
    if (soft) {
        Array min_dists(frame.rows, 0.0);
        for (int i = 0; i < frame.rows; ++i) {
            double min_dist_sq = INFINITY;
            for (int k = 0; k < K; ++k) {
                double dist_sq = 0.0;
                for (int c = 0; c < frame.cols; ++c) {
                    double diff = frame(i, c) - centers(k, c);
                    dist_sq += diff * diff;
                }
                if (dist_sq < min_dist_sq) min_dist_sq = dist_sq;
            }
            min_dists[i] = sqrt(min_dist_sq);
        }
        // Find median
        std::vector<double> sorted_dists(min_dists.begin(), min_dists.end());
        std::sort(sorted_dists.begin(), sorted_dists.end());
        size_t mid = sorted_dists.size() / 2;
        double median_dist = (sorted_dists.size() % 2 == 0) ? 
            (sorted_dists[mid-1] + sorted_dists[mid]) / 2.0 : sorted_dists[mid];
        s = temperature * median_dist + 1e-9;
    }
    
    // Step 3: Generate fingerprints for each lipidome
    vector<Array> fingerprints(n);
    
    for (int i = 0; i < n; ++i) {
        Matrix* lip_matrix = lipidome_matrixes[i];
        // lip_matrix structure: rows=lipids, cols=dims
        
        // per-lipidome abundance weights (fall back to uniform when absent)
        Array uniform;
        Array& weights = (i < (int)lipidome_weights.size() && lipidome_weights[i])
                         ? *lipidome_weights[i] : uniform;
        
        // Generate the fingerprint directly into the output slot (no copy).
        lip_matrix->generate_fingerprint(centers, weights, fingerprints[i], s, soft);
    }
    
    // Step 4: pairwise Hellinger distances via the adaptive scalar/GEMM backend.
    hellinger_matrix_auto(fingerprints);
}
