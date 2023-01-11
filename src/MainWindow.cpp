#include "lipidspace/lipidspacegui.h"
#include "lipidspace/lipidspace.h"
#include "lipidspace/logging.h"
#include "lipidspace/globaldata.h"
#include <QApplication>
#include <QMessageBox>
#include <string>

using namespace std;



int main(int argc, char** argv) {
    if (argc == 1){
        QApplication application(argc, argv);
        Logging::write_log("LipidSpace " + GlobalData::LipidSpace_version + " was launched in window mode");

        LipidSpace lipid_space;
        LipidSpaceGUI lipid_space_gui(&lipid_space);
        lipid_space_gui.show();
        analytics("launch");
        return application.exec();
    }

    else if (argc > 1 && string(argv[1]) == "compare"){
        if (argc < 4) {
            cout << "not enough arguments for lipid comparison" << endl;
            return 0;
        }

        QApplication application(argc, argv);
        LipidSpace lipid_space;
        LipidParser parser;
        LipidAdduct *l1 = 0, *l2 = 0;

        try {
            l1 = parser.parse(argv[2]);
        }
        catch(exception &e){
            cout << "Error: could not parse lipid '" << argv[2] << "'." << endl;
            exit(-1);
        }
        try {
            l2 = parser.parse(argv[3]);
        }
        catch(exception &e){
            cout << "Error: could not parse lipid '" << argv[3] << "'." << endl;
            exit(-1);
        }


        int union_num = 0, inter_num = 0;
        lipid_space.lipid_similarity(l1, l2, union_num, inter_num);

        cout << argv[2] << "(" << l1->get_lipid_string() << ")" << " vs. " << argv[3] << "(" << l2->get_lipid_string() << ")" << endl;
        cout << "union: " << union_num << endl;
        cout << "inter: " << inter_num << endl;
    }

    else if (argc > 1 && string(argv[1]) == "test"){
        QApplication application(argc, argv);
        LipidSpace lipid_space;

        string test_file = "data/UnitTests.csv";
        ifstream infile(test_file);
        if (!infile.good()){
            cout << "Testfile not found. Exit." << endl;
            exit(-1);
        }

        string line;
        int line_num = 0;
        LipidParser parser;
        while (getline(infile, line)){
            if (line_num++ == 0 || line.length() == 0) continue;
            vector<string> *tokens = split_string(line, '\t', '"', true);

            if (tokens->size() != 4){
                cout << "Error: line '" << line << "' has not four elements." << endl;
                exit(-1);
            }

            LipidAdduct *l1 = 0, *l2 = 0;
            try {
                l1 = parser.parse(tokens->at(0));
            }
            catch(exception &e){
                cout << "Error: could not parse lipid '" << tokens->at(0) << "'." << endl;
                exit(-1);
            }
            try {
                l2 = parser.parse(tokens->at(1));
            }
            catch(exception &e){
                cout << "Error: could not parse lipid '" << tokens->at(1) << "'." << endl;
                exit(-1);
            }
            int ref_inter = atoi(tokens->at(2).c_str());
            int ref_union = atoi(tokens->at(3).c_str());

            int union_num = 0, inter_num = 0;
            lipid_space.lipid_similarity(l1, l2, union_num, inter_num);
            if (ref_inter != inter_num || ref_union != union_num){
                cout << "Test failed:" << endl;
                cout << "Lipid1: " << tokens->at(0) << endl;
                cout << "Lipid2: " << tokens->at(1) << endl;
                cout << "Expected values; union: " << ref_union << ", inter: " << ref_inter << endl;
                cout << "Computed values; union: " << union_num << ", inter: " << inter_num << endl;
                cout << "--------------------------" << endl;
                string key = l1->get_extended_class() + "/" + l2->get_extended_class();

                if (uncontains_val(lipid_space.class_matrix, key)){
                    cout << "Error: key '" << key << "' not in precomputed class matrix." << endl;
                    exit(-1);
                }
                cout << "Head group values; union: " << lipid_space.class_matrix[key].first << ", inter: " << lipid_space.class_matrix[key].second << endl;
                exit(-1);
            }

            delete l1;
            delete l2;
            delete tokens;
        }
    }
    else if (argc > 2 && string(argv[1]) == "identify"){
        QApplication application(argc, argv);
        LipidSpace lipid_space;
        string lipid_name = goslin::strip(argv[2], '"');
        LipidParser p;
        try {
            LipidAdduct *l = p.parse(lipid_name);
            cout << "Lipid name: " << l->get_lipid_string() << endl;
            cout << "Lipid sum formula: " << l->get_sum_formula() << endl;
            cout << "Lipid mass: " << l->get_mass() << endl;
            cout << "Lipid class is " << (contains_val(lipid_space.registered_lipid_classes, l->get_extended_class()) ? "" : "NOT ") << "registered in LipidSpace" << endl;
            delete l;
        }
        catch (exception &e){
            cout << "Lipid could not be parsed:" << endl;
            cout << e.what() << endl;
        }
    }

    return 0;
}
