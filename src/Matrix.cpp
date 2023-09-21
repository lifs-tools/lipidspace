#include "lipidspace/Matrix.h"


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
    int i, n8 = n & ~7;
    double sum;
    __m256d vector_sum = {0., 0., 0., 0.};

    for (i = 0; i < n8; i += 8) {
        __m256d vector_x1 = *(__m256d*)(x + i);
        __m256d vector_y1 = *(__m256d*)(y + i);
        __m256d vector_x2 = *(__m256d*)(x + i + 4);
        __m256d vector_y2 = *(__m256d*)(y + i + 4);
        vector_sum += vector_x1 * vector_y1;
        vector_sum += vector_x2 * vector_y2;
    }
    vector_sum[0] += vector_sum[1];
    vector_sum[2] += vector_sum[3];
    vector_sum[0] += vector_sum[2];
    for (sum = 0.0; i < n; ++i) sum += x[i] * y[i];
    sum += vector_sum[0];
	return sum;
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


    #pragma omp parallel for collapse(2)
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


double* Matrix::data(){
    return m.data();
}
