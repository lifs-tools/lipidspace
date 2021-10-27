#include "LipidSpace/lambda_lanczos.hpp"

#include <vector>
#include <math.h>
#include <iostream>
#include <chrono>
#include <cassert>
#include <cblas.h>

using lambda_lanczos::LambdaLanczos;
using namespace std;

#define sq(x) ((x) * (x))


class Array : public vector<double>{
    Array(Array &a);
};


class Mat {
public:
    Array m;
    int rows;
    int cols;
    
    Mat();
    Mat(const Array &copy, int _rows, int _cols);
    Mat(vector<Array> &copy);
    Mat(int _rows, int _cols);
    void reset(int _rows, int _cols);
    void rewrite(vector<Array> &copy);
    void rand_fill();
    void scale();
    void transpose();
    double pairwise_sum(Mat &m);
    void mult(Mat& A, Mat& B, bool transA = false, bool transB = false, double alpha = 1.0);
    void covariance_matrix(Mat &covar);
    void compute_eigen_data(Array &eigenvalues, Mat& eigenvectors, int top_n);
    void PCA(Mat &pca, int dimensions);
    void add_column(Array &col);
    inline double* data();
    friend ostream& operator << (ostream& os, const Mat& m){
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
