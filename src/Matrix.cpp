#include "LipidSpace/Matrix.h"

    
Array::Array(Array &a) : vector<double>() {
    reserve(a.size());
    for (auto val : a) push_back(val);
}
    
    
Mat::Mat(){
    rows = 0;
    cols = 0;
}

Mat::Mat(const Array &copy, int _rows, int _cols){
    cols = _cols;
    rows = _rows;
    m.clear();
    m.reserve(cols * rows);
    for (double v : copy) m.push_back(v);
}

Mat::Mat(vector<Array> &copy){
    rewrite(copy);
}



void Mat::add_column(Array &col){
    assert(cols == col.size());
    cols += 1;
    m.reserve(cols * rows);
    for (auto val : col) m.push_back(val);
}


void Mat::rewrite(vector<Array> &copy){
    cols = copy.size();
    rows = copy[0].size();
    m.clear();
    m.reserve(cols * rows);
    for (auto col : copy){
        for (auto val : col) m.push_back(val);
    }
}


Mat::Mat(int _rows, int _cols){
    reset(_rows, _cols);
}

void Mat::reset(int _rows, int _cols){
    rows = _rows;
    cols = _cols;
    m.clear();
    m.reserve(cols * rows);
    for (int i = 0; i < cols * rows; ++i) m.push_back(0);
}


void Mat::rand_fill(){
    for (int i = 0; i < cols * rows; ++i) m[i] = 2 * (rand() / (double)RAND_MAX) - 1.;
}


double Mat::pairwise_sum(Mat &m){
    assert(m.cols == 2);
    Mat tm(m, true);
    
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




void Mat::scale(){
    for (int c = 0; c < cols; c++){
        // estimating the mean
        double mean = 0;
        #pragma omp parallel for default(shared) reduction(+:mean)
        for (int it = c * rows; it < (c + 1) * rows; ++it){
            mean += m[it];
        }
        mean /= (double)rows;
        
        // estimating the standard deviation
        double stdev_inv = 0;
        #pragma omp parallel for default(shared) reduction(+:stdev_inv)
        for (int it = c * rows; it < (c + 1) * rows; ++it){
            stdev_inv += sq(mean - m[it]);
        }
        stdev_inv = sqrt((double)rows / stdev_inv);
        
        // performing z transformation, aka (x - mean) / st_dev
        #pragma omp parallel for
        for (int it = c * rows; it < (c + 1) * rows; ++it){
            m[it] = (m[it] - mean) * stdev_inv;
        }
    }
}


void Mat::transpose(){
    double *tmp = new double[cols * rows];
    #pragma omp parallel for
    for (int i = 0; i < cols * rows; ++i) tmp[i] = m[i];
    
    #pragma omp parallel for collapse(2)
    for (int c = 0; c < cols; c++){
        for (int r = 0; r < rows; ++r){
            m[r * cols + c] = tmp[c * rows + r];
        }
    }
    swap(rows, cols);
        
    delete tmp;
}


void Mat::compute_eigen_data(Array &eigenvalues, Mat& eigenvectors, int top_n){
    assert(rows == cols);
        
    // Prepare matrix-vector multiplication routine used in Lanczos algorithm
    auto mv_mul = [&](const vector<double>& in, vector<double>& out) {
        cblas_dgemv(CblasColMajor, CblasNoTrans, rows, cols, 1.0, data(), rows, in.data(), 1, 0, out.data(), 1);
    };
    
    // Execute Lanczos algorithm
    LambdaLanczos<double> engine(mv_mul, rows, true); // true means to calculate the largest eigenvalue.
    eigenvalues.resize(top_n);
    vector<Array> eigvecs(top_n);
    int itern = engine.run(eigenvalues, eigvecs);
    eigenvectors.rewrite(eigvecs);
}


void Mat::mult(Mat& A, Mat& B, bool transA, bool transB, double alpha){
    assert((transA ? A.rows : A.cols) == (transB ? B.cols : B.rows));
    
    int mm = transA ? A.cols : A.rows;
    int kk = transA ? A.rows : A.cols;
    int nn = transB ? B.rows : B.cols;
    reset(mm, nn);
    cblas_dgemm(CblasColMajor, transA ? CblasTrans : CblasNoTrans, transB ? CblasTrans : CblasNoTrans, mm, nn, kk, alpha, A.data(), A.rows, B.data(), B.rows, 0.0, data(), rows);
}


void Mat::PCA(Mat &pca, int dimensions){
        scale();
        Mat cov_matrix;
        covariance_matrix(cov_matrix);
        Array eigenvalues;
        Mat eigenvectors;
        cov_matrix.compute_eigen_data(eigenvalues, eigenvectors, dimensions);
        pca.mult(eigenvectors, *this, true, true);
        pca.transpose();
    }


void Mat::covariance_matrix(Mat &covar){
    covar.reset(cols, cols);
    double factor = 1. / ((double)cols - 1);
    covar.mult(*this, *this, true, false, factor);
}


double* Mat::data(){
    return m.data();
}
