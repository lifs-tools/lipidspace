#include "LipidSpace/Matrix.h"

Array::Array() : vector<double>(){
}
    
Array::Array(Array *a, int l) : vector<double>() {
    if (l != -1){
        assert(0 < l && l <= a->size());
        reserve(l);
        for (int i = 0; i < l; ++i) push_back(a->at(i));
    }
    else {
        reserve(a->size());
        for (auto val : *a) push_back(val);
    }
}


double Array::mean(){
    double mn;
    for (int i = 0; i < size(); ++i) mn += at(i);
    return mn / (double)size();
}


double Array::stdev(){
    double mn = mean(), st = 0;
    for (int i = 0; i < size(); ++i) st += sq(at(i) - mn);
    return sqrt(st / (double)size());
}

void Array::add(Array &a){
    reserve(size() + a.size());
    for (auto val : a) push_back(val);
}

void Array::add(vector<double> &a){
    reserve(size() + a.size());
    for (auto val : a) push_back(val);
}


void Array::reset(Array &a){
    clear();
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

Mat::Mat(vector<vector<double>> &copy){
    rewrite(copy);
}


Mat::Mat(Mat &mat, bool transpose){
    if (transpose) rewrite_transpose(mat);
    else rewrite(mat);
}

    
void Mat::rewrite_transpose(Mat &mat){
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


void Mat::rewrite(Mat &copy, const Indexes &ri, const Indexes &ci){
    assert(ri.size() > 0 || ci.size() > 0);
    m.clear();
    if (ci.size() == 0){
        cols = copy.cols;
        rows = ri.size();
        m.reserve(cols * rows);
        for (int c = 0; c < cols; c++){
            for (auto r : ri){
                m.push_back(copy(r, c));
            }
        }
    }
    else if (ri.size() == 0){
        cols = ci.size();
        rows = copy.rows;
        m.reserve(cols * rows);
        for (auto c : ci){
            for (int r = 0; r < rows; r++){
                m.push_back(copy(r, c));
            }
        }
    }
    else {
        cols = ci.size();
        rows = ri.size();
        m.reserve(cols * rows);
        for (auto c : ci){
            for (auto r : ri){
                m.push_back(copy(r, c));
            }
        }
    }
        
}



void Mat::add_column(Array &col){
    if (cols > 0){
        assert(rows == col.size());
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



double Mat::col_min(int c){
    assert(0 <= c && c < cols && rows > 0);
    double min_val = m[c * rows];
    for (int it = c * rows + 1; it < (c + 1) * rows; it++){
        min_val = mmin(min_val, m[it]);
    }
    return min_val;
}



double Mat::col_max(int c){
    assert(0 <= c && c < cols && rows > 0);
    double max_val = m[c * rows];
    for (int it = c * rows + 1; it < (c + 1) * rows; it++){
        max_val = mmax(max_val, m[it]);
    }
    return max_val;
}


    
void Mat::rewrite(vector<vector<double>> &copy){
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
    vector<double> evals(top_n);
    vector<vector<double>> eigvecs(top_n);
    int itern = engine.run(evals, eigvecs);
    
    eigenvalues.add(evals);
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
