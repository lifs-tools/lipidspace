#include <iostream>
#include <Eigen/Dense>
#include "LipidSpace/matplotlibcpp.h"
#include <vector>
#include <cmath>
#include <iostream>
#include <string>
#include <cppgoslin/cppgoslin.h>
 
using namespace std;
using namespace Eigen;



class LipidSpace {
public:
    LipidParser parser;
    vector<LipidAdduct*>* create_table(string lipid_list_file);
    ArrayXd compute_PCA_variances(MatrixXd m);
    MatrixXd compute_PCA(MatrixXd m);
};


vector<LipidAdduct*>* LipidSpace::create_table(string lipid_list_file){
    cout << "reading '" << lipid_list_file << "'" << endl;
    
    // load and parse lipids
    ifstream infile(lipid_list_file);
    if (!infile.good()){
        cout << "Error: file '" << lipid_list_file << "' not found." << endl;
        exit(-1);
    }
    string line;
    
    vector<LipidAdduct*>* lipid_list = new vector<LipidAdduct*>();
    while (getline(infile, line)){
        if (line.length() == 0) continue;
        
        vector<string>* tokens = goslin::split_string(line, '\n', '"');
        line = strip(tokens->at(0), '"');
        delete tokens;
        
        try {
            lipid_list->push_back(parser.parse(line));
        }
        catch (exception &e) {
            cout << "Error: lipid '" << line << "' cannot be parsed" << endl;
        }
    }
    
    int n = lipid_list->size();

    return lipid_list;
}


ArrayXd LipidSpace::compute_PCA_variances(MatrixXd m){
    ArrayXd var = m.array().square().colwise().sum();
    return var / var.sum();
}


MatrixXd LipidSpace::compute_PCA(MatrixXd m){
    // scale and transform the matrix
    int r = m.rows();
    m = m.rowwise() - (m.rowwise().mean()).transpose();
    VectorXd norm = (r / m.array().square().colwise().sum()).sqrt();
    for (int i = 0; i < r; ++i) m.block(0, i, r, 1) *= norm[i];
    
    // calculate the covariance matrix
    MatrixXd centered = m.rowwise() - m.colwise().mean();
    MatrixXd cov = (centered.adjoint() * centered) / double(m.rows() - 1);
    
    // calculate eigenvectors of the covariance matrix
    SelfAdjointEigenSolver<MatrixXd> eigensolver(cov);
    
    // reverse the order of columns
    MatrixXd evecs = eigensolver.eigenvectors();
    MatrixXd rev = evecs.rowwise().reverse();
    
    // carry out the transformation on the data using eigenvectors
    return (rev.transpose() * m.transpose()).transpose();
}


int main(int argc, char** argv) {
    bool plot_pca = true;
    bool store_tables = true;
    
    if (argc < 3) {
        cout << "usage: python3 " << argv[0] << " output_folder lipid_list[csv], ..." << endl;
        exit(-1);
    }
        
    string output_folder = argv[1];
    vector<string> input_lists;
    for (int i = 2; i < argc; ++i) input_lists.push_back(argv[i]);
    
    LipidSpace lipid_space;
    vector<LipidAdduct*>* lipids = lipid_space.create_table(input_lists.at(0));
    cout << lipids->size() << endl;
    
}
