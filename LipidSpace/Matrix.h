#include "LipidSpace/lambda_lanczos.hpp"

#include <vector>
#include <math.h>
#include <iostream>
#include <chrono>
#include <cassert>
#include <cblas.h>

using namespace std;

#define Array vector< double >
#define sq(x) ((x) * (x))

class Matrix {
public:
    Array m;
    int rows;
    int cols;
    
    Matrix();
    Matrix(const Array &copy, int _rows, int _cols);
    Matrix(vector<Array> &copy);
    Matrix(int _rows, int _cols);
    void reset(int _rows, int _cols);
    void rewrite(vector<Array> &copy);
    void rand_fill();
    void scale();
    void transpose();
    void mult(Matrix& A, Matrix& B, bool transA = false, bool transB = false, double alpha = 1.0);
    void covariance_matrix(Matrix &covar);
    void compute_eigen_data(Matrix &M, Array &eigenvalues, Matrix& eigenvectors, int top_n);
    void PCA(Matrix &pca, int dimensions);
    inline double* data();
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
    
    
};
