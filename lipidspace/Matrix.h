#ifndef MATRIX_H
#define MATRIX_H

#include "lipidspace/lambda_lanczos.hpp"
#include "cppgoslin/cppgoslin.h"
#include "nlohmann/json.hpp"

#include <vector>
#include <math.h>
#include <iostream>
#include <cassert>
#include <QtCore>
#include <cmath>

using lambda_lanczos::LambdaLanczos;
using namespace std;
using json = nlohmann::json;

#define sq(x) ((x) * (x))
#define mmin(x, y) ((x) < (y) ? (x) : (y))
#define mmax(x, y) ((x) > (y) ? (x) : (y))


class Matrix;

class Array : public vector<double> {

public:
    Array();
    Array(int len, double val);
    Array(const Array &a, int l = -1);
    void reset(Array &a);
    double mean();
    double stdev();
    double sample_stdev();
    double sum();
    void mult(Matrix &m, Array &a);
    void add(Array &a);
    double median(int begin_v = -1, int end_v = -1, bool sorted = false);
    void add(vector<double> &a);
    int greatest_less(double key, int L = -1, int R = -1);
    void compute_distances(Array &x, double dx, Array &y, double dy);
    void compute_coefficiants(Matrix &data, Array &values);
    void remove_below(double val = 0);
    friend Array& operator+=(Array &me, const double val);
    friend Array& operator-=(Array &me, const double val);
    friend Array& operator*=(Array &me, const double val);
    friend Array& operator/=(Array &me, const double val);
    friend ostream& operator << (ostream& os, const Array& a){
        for (auto val : a) os << val << " ";
        os << endl;
        return os;
    }

    inline double& operator ()(int i){
        if (i < 0 || (int)size() <= i){
            throw "Constrain violation, 0 <= i <=" + std::to_string(size());
        }
        return at(i);
    }
};
typedef Array Indexes;


class Matrix : public QObject {
    Q_OBJECT

public:
    Array m;
    int rows;
    int cols;

    Matrix();
    Matrix(const Array &copy, int _rows, int _cols);
    Matrix(vector<vector<double>> &copy);
    Matrix(int _rows, int _cols);
    Matrix(Matrix &mat, bool transpose = false);
    Matrix(json &container);
    friend Matrix& operator+=(Matrix &me, const double val);
    friend Matrix& operator-=(Matrix &me, const double val);
    friend Matrix& operator*=(Matrix &me, const double val);
    friend Matrix& operator/=(Matrix &me, const double val);
    void clear();
    void reset(int _rows, int _cols);
    void rewrite_transpose(Matrix &copy);
    void rewrite(vector<vector<double>> &copy);
    void rewrite(Matrix &copy, const Indexes &ri = {}, const Indexes &ci = {});
    void scale();
    double col_min(int c);
    double col_max(int c);
    void transpose();
    void inverse(Matrix &, bool symmetric = false);
    void mult_vector(const vector<double> &in, vector<double> &out);
    static double vector_vector_mult(int n, const double *x, const double *y);
    double pairwise_sum(Matrix &m);
    void pad_cols_4();
    void mult(Matrix& A, Matrix& B, bool transA = false, bool transB = false, double alpha = 1.0);
    void covariance_matrix(Matrix &covar);
    // SYRK covariance: C = Z^T Z / (cols-1), symmetric, via a single BLAS dsyrk
    // (half the flops of the dgemm path). Parity-tested vs covariance_matrix().
    void covariance_matrix_syrk(Matrix &covar);
    void compute_eigen_data(Array &eigenvalues, Matrix& eigenvectors, int top_n);
    void PCA(Matrix &pca, int dimensions = 2);
    void add_column(Array &col);
    double* data();
    void load(json &container);
    void save(json &container);
    
    // Fingerprint methods for Atlas
    void kmeans(int K, Matrix& centers, int max_iter = 100, unsigned long seed = 42);
    void generate_fingerprint(Matrix& centers, Array& weights, Array& fingerprint,
                              double s = 1.0, bool soft = true, Matrix* contributions = nullptr);
    static double hellinger_distance(const Array& a, const Array& b);
    // BLAS/GEMM pairwise Hellinger distance matrix. Fills *this (n x n) with the
    // Hellinger distance between every pair of fingerprints. Numerically equivalent
    // to looping hellinger_distance() over all pairs (see tests/tst_matrix_blas).
    void hellinger_matrix(const vector<Array>& fingerprints);
    // Packaged OpenMP-parallel scalar pairwise Hellinger matrix (the pre-BLAS path),
    // retained as the CPU fast path at small K and as the parity oracle's sibling.
    void hellinger_matrix_scalar(const vector<Array>& fingerprints);
    // Adaptive dispatch: GEMM when a GPU BLAS backend is active or K is large enough
    // to amortize the N*N Gram; otherwise the OpenMP scalar path (faster at small K).
    void hellinger_matrix_auto(const vector<Array>& fingerprints);
    void compute_fingerprint_distance_matrix(vector<Matrix*>& lipidome_matrixes,
                                              vector<Array*>& lipidome_weights, 
                                              Matrix& frame, 
                                              int K = 20, 
                                              double temperature = 1.0, 
                                              bool soft = true);

    friend ostream& operator << (ostream& os, const Matrix& m){
        for (int r = 0; r < m.rows; ++r){
            for (int c = 0; c < m.cols; c++){
                os << m.m[c * m.rows + r] << " ";
            } os << endl;
        }
        return os;
    }

    inline double at(int r, int c) const {
        if (r < 0 || rows <= r || c < 0 || cols <= c){
            throw "Constrain violation, 0 <= r <=" + std::to_string(rows) + ", 0 <= c <= " + std::to_string(cols);
        }
        return m[c * rows + r];
    }

    inline double& operator ()(int r, int c){
        if (r < 0 || rows <= r || c < 0 || cols <= c){
            throw "Constrain violation, 0 <= r <=" + std::to_string(rows) + ", 0 <= c <= " + std::to_string(cols);
        }
        return m[c * rows + r];
    }

    void random(int r, int c){
        reset(r, c);
        for (int i = 0; i < r; ++i){
            for (int j = 0; j < c; ++j) {
                (*this)(i, j) = (double)rand() / 10000.;
            }
        }
    }

signals:
    void increment();
};

#endif /* MATRIX_H */
