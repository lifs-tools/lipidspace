#include <QtTest/QtTest>
#include <QElapsedTimer>
#include <QThread>
#include <vector>
#include <random>
#include <cmath>
#include <algorithm>
#include "lipidspace/Matrix.h"

/*
 * Parity tests for the BLAS/GEMM reformulations in Matrix.
 *
 * The new BLAS paths must reproduce the existing scalar implementations within
 * floating-point tolerance. The scalar implementation is the golden reference;
 * it stays in the codebase and is exercised here as the oracle.
 */
class TestMatrixBlas : public QObject {
    Q_OBJECT

private:
    // Deterministic random fingerprints normalized onto the simplex (rows sum to 1).
    static std::vector<Array> makeFingerprints(int n, int K, unsigned seed) {
        std::mt19937 gen(seed);
        std::uniform_real_distribution<double> u(0.0, 1.0);
        std::vector<Array> fps;
        fps.reserve(n);
        for (int i = 0; i < n; ++i) {
            Array fp(K, 0.0);
            double sum = 0.0;
            for (int k = 0; k < K; ++k) { fp[k] = u(gen); sum += fp[k]; }
            for (int k = 0; k < K; ++k) fp[k] /= sum;
            fps.push_back(fp);
        }
        return fps;
    }

    static double maxAbsDiff(Matrix& a, Matrix& b) {
        double md = 0.0;
        for (int i = 0; i < a.rows; ++i)
            for (int j = 0; j < a.cols; ++j)
                md = std::max(md, std::fabs(a(i, j) - b(i, j)));
        return md;
    }

    // Deterministic random UN-normalized fingerprints (raw positive weights that
    // do NOT sum to 1). Distinguishes the general Gram form from the sqrt(1-BC)
    // shortcut, which is only valid on the simplex.
    static std::vector<Array> makeRawFingerprints(int n, int K, unsigned seed) {
        std::mt19937 gen(seed);
        std::uniform_real_distribution<double> u(0.0, 5.0);
        std::vector<Array> fps;
        fps.reserve(n);
        for (int i = 0; i < n; ++i) {
            Array fp(K, 0.0);
            for (int k = 0; k < K; ++k) fp[k] = u(gen);
            fps.push_back(fp);
        }
        return fps;
    }

    // Golden reference: the existing scalar pairwise loop (serial).
    static Matrix scalarHellingerMatrix(std::vector<Array>& fps) {
        int n = (int)fps.size();
        Matrix ref;
        ref.reset(n, n);
        for (int i = 0; i < n; ++i) {
            ref(i, i) = 0.0;
            for (int j = i + 1; j < n; ++j) {
                double d = Matrix::hellinger_distance(fps[i], fps[j]);
                ref(i, j) = d;
                ref(j, i) = d;
            }
        }
        return ref;
    }

    // Production-representative baseline: the same scalar loop parallelized with
    // OpenMP exactly like compute_fingerprint_distance_matrix() does.
    static Matrix ompScalarHellingerMatrix(std::vector<Array>& fps) {
        int n = (int)fps.size();
        Matrix ref;
        ref.reset(n, n);
        #pragma omp parallel for
        for (int i = 0; i < n; ++i) {
            for (int j = i + 1; j < n; ++j) {
                double d = Matrix::hellinger_distance(fps[i], fps[j]);
                ref(i, j) = d;
                ref(j, i) = d;
            }
            ref(i, i) = 0.0;
        }
        return ref;
    }

    // Median wall-clock (ms) of a callable over R runs, using a volatile sink to
    // defeat dead-code elimination.
    template <typename F>
    static double timeMedianMs(F&& f, int R) {
        std::vector<double> ts;
        ts.reserve(R);
        volatile double sink = 0.0;
        for (int r = 0; r < R; ++r) {
            QElapsedTimer t; t.start();
            Matrix m = f();
            double ms = t.nsecsElapsed() / 1e6;
            sink = m.rows > 1 ? m(0, m.rows - 1) : 0.0;
            ts.push_back(ms);
        }
        (void)sink;
        std::sort(ts.begin(), ts.end());
        return ts[ts.size() / 2];
    }

private slots:
    void hellinger_matrix_matches_scalar_pairwise();
    void hellinger_matrix_identical_fingerprints_are_zero();
    void hellinger_matrix_disjoint_fingerprints_are_unit();
    void hellinger_matrix_matches_scalar_when_unnormalized();
    void hellinger_matrix_handles_single_and_empty();
    void hellinger_matrix_scalar_matches_oracle();
    void hellinger_matrix_auto_matches_oracle_across_K();
    void benchmark_hellinger_backends();
    void covariance_syrk_matches_dgemm();
    void benchmark_covariance_backends();
};

void TestMatrixBlas::hellinger_matrix_matches_scalar_pairwise() {
    const int n = 17, K = 20;
    std::vector<Array> fps = makeFingerprints(n, K, 12345u);

    Matrix ref = scalarHellingerMatrix(fps);

    Matrix cand;
    cand.hellinger_matrix(fps);

    QCOMPARE(cand.rows, n);
    QCOMPARE(cand.cols, n);
    const double diff = maxAbsDiff(ref, cand);
    QVERIFY2(diff < 1e-9,
             qPrintable(QString("max|scalar - gemm| = %1 (>= 1e-9)").arg(diff, 0, 'g', 17)));
}

void TestMatrixBlas::hellinger_matrix_identical_fingerprints_are_zero() {
    // Two identical fingerprints must have distance 0 (and self-distance 0).
    Array fp(5, 0.0);
    fp[0] = 0.4; fp[1] = 0.1; fp[2] = 0.2; fp[3] = 0.25; fp[4] = 0.05;
    std::vector<Array> fps = { fp, fp, fp };

    Matrix cand;
    cand.hellinger_matrix(fps);

    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            QVERIFY2(std::fabs(cand(i, j)) < 1e-12,
                     qPrintable(QString("d(%1,%2)=%3").arg(i).arg(j).arg(cand(i, j))));
}

void TestMatrixBlas::hellinger_matrix_disjoint_fingerprints_are_unit() {
    // Disjoint supports => Bhattacharyya coefficient 0 => Hellinger distance 1.
    Array a(3, 0.0); a[0] = 1.0;
    Array b(3, 0.0); b[1] = 1.0;
    std::vector<Array> fps = { a, b };

    Matrix cand;
    cand.hellinger_matrix(fps);

    QVERIFY(std::fabs(cand(0, 1) - 1.0) < 1e-12);
    QVERIFY(std::fabs(cand(1, 0) - 1.0) < 1e-12);
}

void TestMatrixBlas::hellinger_matrix_matches_scalar_when_unnormalized() {
    // Differential guard: the general Gram form must still match the scalar loop
    // when rows do NOT sum to 1. A sqrt(1 - BC) shortcut would fail this.
    const int n = 11, K = 8;
    std::vector<Array> fps = makeRawFingerprints(n, K, 999u);

    Matrix ref = scalarHellingerMatrix(fps);
    Matrix cand;
    cand.hellinger_matrix(fps);

    const double diff = maxAbsDiff(ref, cand);
    QVERIFY2(diff < 1e-9,
             qPrintable(QString("un-normalized max|scalar - gemm| = %1").arg(diff, 0, 'g', 17)));
}

void TestMatrixBlas::hellinger_matrix_handles_single_and_empty() {
    // n = 1 -> 1x1 zero matrix; n = 0 -> 0x0. Must not crash.
    std::vector<Array> one = { Array(4, 0.25) };
    Matrix m1;
    m1.hellinger_matrix(one);
    QCOMPARE(m1.rows, 1);
    QCOMPARE(m1.cols, 1);
    QVERIFY(std::fabs(m1(0, 0)) < 1e-12);

    std::vector<Array> none;
    Matrix m0;
    m0.hellinger_matrix(none);
    QCOMPARE(m0.rows, 0);
    QCOMPARE(m0.cols, 0);
}

void TestMatrixBlas::hellinger_matrix_scalar_matches_oracle() {
    // The packaged OMP-scalar N x N method must equal the pairwise oracle.
    const int n = 23, K = 12;
    std::vector<Array> fps = makeFingerprints(n, K, 555u);

    Matrix ref = scalarHellingerMatrix(fps);
    Matrix cand;
    cand.hellinger_matrix_scalar(fps);

    QCOMPARE(cand.rows, n);
    QCOMPARE(cand.cols, n);
    QVERIFY2(maxAbsDiff(ref, cand) < 1e-12, "packaged scalar diverges from oracle");
}

void TestMatrixBlas::hellinger_matrix_auto_matches_oracle_across_K() {
    // The dispatcher must match the oracle whichever branch it takes:
    // small K -> scalar branch, large K -> GEMM branch.
    for (int K : { 8, 20, 256 }) {
        const int n = 40;
        std::vector<Array> fps = makeFingerprints(n, K, 4242u + K);

        Matrix ref = scalarHellingerMatrix(fps);
        Matrix cand;
        cand.hellinger_matrix_auto(fps);

        QCOMPARE(cand.rows, n);
        QCOMPARE(cand.cols, n);
        const double diff = maxAbsDiff(ref, cand);
        QVERIFY2(diff < 1e-9,
                 qPrintable(QString("auto(K=%1) max|oracle-auto| = %2").arg(K).arg(diff, 0, 'g', 17)));
    }
}

// ---- Benchmark: evidence of the GEMM speed-up, honestly compared ----
// Three backends on identical inputs/machine:
//   (1) serial scalar loop (no BLAS, no threads)
//   (2) OpenMP-parallel scalar loop (== the production compute_fingerprint path)
//   (3) GEMM reformulation via BLAS dgemm (Accelerate here, OpenBLAS on Linux,
//       cuBLAS once the GPU backend is compiled in).
// The K-sweep shows the crossover: at the Atlas default K=20 the op is bandwidth
// bound (skinny rank-K update), so the CPU win is modest; as K grows the GEMM's
// arithmetic intensity rises and it pulls away.

void TestMatrixBlas::benchmark_hellinger_backends() {
    const int N = 1500;                     // atlas-scale number of lipidomes
    const int Ks[] = { 20, 200 };           // 20 = Atlas default; 200 = richer codebook
    const int R = 5;

    qInfo("Pairwise Hellinger, N=%d, median of %d runs (%d hw threads):",
          N, R, QThread::idealThreadCount());
    qInfo("  %-6s %12s %12s %12s   %10s %10s", "K",
          "serial(ms)", "omp(ms)", "gemm(ms)", "gemm/serial", "gemm/omp");

    for (int K : Ks) {
        std::vector<Array> fps = makeFingerprints(N, K, 2024u);

        // Parity at benchmark scale before timing (the gate; speed is evidence only).
        Matrix ref = scalarHellingerMatrix(fps);
        Matrix gemm0; gemm0.hellinger_matrix(fps);
        QVERIFY2(maxAbsDiff(ref, gemm0) < 1e-9, "parity fails at benchmark scale");

        double serial_ms = timeMedianMs([&]{ return scalarHellingerMatrix(fps); }, R);
        double omp_ms    = timeMedianMs([&]{ return ompScalarHellingerMatrix(fps); }, R);
        double gemm_ms   = timeMedianMs([&]{ Matrix g; g.hellinger_matrix(fps); return g; }, R);

        qInfo("  %-6d %12.2f %12.2f %12.2f   %9.1fx %9.1fx", K,
              serial_ms, omp_ms, gemm_ms,
              gemm_ms > 0 ? serial_ms / gemm_ms : 0.0,
              gemm_ms > 0 ? omp_ms / gemm_ms : 0.0);
    }
}

void TestMatrixBlas::covariance_syrk_matches_dgemm() {
    // The SYRK covariance (C = factor * Z^T Z, symmetric, half the flops) must equal
    // the existing dgemm covariance within floating-point tolerance.
    srand(20240724);
    Matrix Z;
    Z.random(64, 40);

    Matrix cov_ref, cov_syrk;
    Z.covariance_matrix(cov_ref);
    Z.covariance_matrix_syrk(cov_syrk);

    QCOMPARE(cov_syrk.rows, cov_ref.rows);
    QCOMPARE(cov_syrk.cols, cov_ref.cols);
    const double diff = maxAbsDiff(cov_ref, cov_syrk);
    QVERIFY2(diff < 1e-9,
             qPrintable(QString("covariance max|dgemm - syrk| = %1").arg(diff, 0, 'g', 17)));
}

void TestMatrixBlas::benchmark_covariance_backends() {
    srand(7);
    Matrix Z;
    Z.random(2000, 800);   // covariance is 800 x 800
    const int R = 5;

    Matrix cov_ref, cov_syrk;
    Z.covariance_matrix(cov_ref);
    Z.covariance_matrix_syrk(cov_syrk);
    QVERIFY2(maxAbsDiff(cov_ref, cov_syrk) < 1e-9, "covariance parity fails at bench scale");

    double dgemm_ms = timeMedianMs([&]{ Matrix c; Z.covariance_matrix(c); return c; }, R);
    double syrk_ms  = timeMedianMs([&]{ Matrix c; Z.covariance_matrix_syrk(c); return c; }, R);
    qInfo("Covariance Z^T Z (2000x800 -> 800x800): dgemm = %.2f ms, syrk = %.2f ms  ->  %.2fx",
          dgemm_ms, syrk_ms, syrk_ms > 0 ? dgemm_ms / syrk_ms : 0.0);
}

QTEST_MAIN(TestMatrixBlas)
#include "tst_matrix_blas.moc"
