#include "lipidspace/lambda_lanczos.hpp"

#include <vector>
#include <math.h>
#include <iostream>
#include <cassert>
//#ifndef _WIN32
#include <cblas.h>
//#endif
#include <QtCore>

using lambda_lanczos::LambdaLanczos;
using namespace std;

#define sq(x) ((x) * (x))
#define mmin(x, y) ((x) < (y) ? (x) : (y))
#define mmax(x, y) ((x) > (y) ? (x) : (y))

extern "C" {
    // LU decomoposition of a general matrix
    void dgetrf_(int* M, int *N, double* A, int* lda, int* IPIV, int* INFO);

    // generate inverse of a matrix given its LU decomposition
    void dgetri_(int* N, double* A, int* lda, int* IPIV, double* WORK, int* lwork, int* INFO);
}


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
    friend Matrix& operator+=(Matrix &me, const double val);
    friend Matrix& operator-=(Matrix &me, const double val);
    friend Matrix& operator*=(Matrix &me, const double val);
    friend Matrix& operator/=(Matrix &me, const double val);
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
    void compute_eigen_data(Array &eigenvalues, Matrix& eigenvectors, int top_n);
    void PCA(Matrix &pca, int dimensions = 2);
    void add_column(Array &col);
    double* data();
    friend ostream& operator << (ostream& os, const Matrix& m){
        for (int r = 0; r < m.rows; ++r){
            for (int c = 0; c < m.cols; c++){
                os << m.m[c * m.rows + r] << " ";
            } os << endl;
        }
        return os;
    }
    
    inline double& operator ()(int r, int c){
        if (r < 0 || rows <= r || c < 0 || cols <= c){
            throw "Constrain violation, 0 <= r <=" + std::to_string(rows) + ", 0 <= c <= " + std::to_string(cols);
        }
        return m[c * rows + r];
    }
    
signals:
    void set_step();
};
