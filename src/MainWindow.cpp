#include "lipidspace/lipidspacegui.h"
#include "lipidspace/lipidspace.h"
#include "lipidspace/logging.h"
#include "lipidspace/globaldata.h"
#include <QApplication>
#include <string>

using namespace std;



void print_help(){
    cerr << "usage: " << endl;
    cerr << "  > ./lipidspace [options] output_folder table lipid_table[csv]" << endl;
    cerr << "  > ./lipidspace [options] output_folder lists lipid_list[csv], ..." << endl;
    cerr << "modes either 'table' or 'lists'." << endl << endl;
    cerr << "options:" << endl;
    cerr << " -h\t\tshow this help message" << endl;
    cerr << " -sn\t\tignore sn-positions, compare all pairwise fatty acyl chains" << endl;
    cerr << " -i\t\tignore unknown lipids (default: exit with error)" << endl;
    cerr << " -q\t\tignore quantitative data, if present (default: guess what, consider them)" << endl;
    cerr << " -d\t\tstore distance tables" << endl;
    cerr << " -p\t\tplot only figure for global principal component analysis" << endl;
    cerr << " -u\t\tunbounded lipid distance to inf (default: 0 <= distance <= 1)" << endl;
}



int main(int argc, char** argv) {

    // parameters to change
    /*
    bool keep_sn_position = true;
    bool ignore_unknown_lipids = false;
    bool plot_pca = true;
    bool plot_pca_lipidomes = true;
    bool storing_distance_table = false;
    bool unboundend_distance = false;
    bool without_quant = false;
    */



    if (argc == 1){

        QApplication application(argc, argv);
        Logging::write_log("LipidSpace " + GlobalData::LipidSpace_version + " was launched in window mode");
        LipidSpace lipid_space;
        LipidSpaceGUI lipid_space_gui(&lipid_space);
        lipid_space_gui.show();
        return application.exec();
    }

    else if (argc > 1 && string(argv[1]) == "a"){
        vector<double> ff = {3.98, 3.99, 4.07, 4.25, 4.27, 4.29, 4.31, 4.32, 4.34, 4.34, 4.35, 4.37, 4.39, 4.4, 4.4, 4.4, 4.41, 4.42, 4.43, 4.52, 4.54, 4.56, 4.58, 4.59, 4.59, 4.59, 4.65, 4.65, 4.7, 4.77, 4.83, 4.89, 4.91, 4.95, 5.14};
        Array f;
        f.add(ff);

        vector<double> mm = {4.32, 4.71, 4.72, 4.76, 4.79, 4.8, 4.87, 4.88, 4.9, 4.92, 4.95, 4.95, 4.96, 4.96, 4.97, 4.97, 5.02, 5.06, 5.07, 5.07, 5.1, 5.11, 5.13, 5.13, 5.14, 5.15, 5.17, 5.18, 5.2, 5.23, 5.32, 5.34, 5.36, 5.47, 5.52, 5.62};
        Array m;
        m.add(mm);

        double d, sep;
        ks_separation_value(m, f, d, sep);

        cout << d << " " << sep << endl;

        return 0;
    }

    else if (argc > 1 && string(argv[1]) == "mem"){

        int a;

        //QApplication application(argc, argv);

        vector<TableColumnType> *ct = new vector<TableColumnType>();
        for (int i = 0; i < 296; ++i) ct->push_back(LipidColumn);
        ct->at(0) = SampleColumn;
        ct->at(3) = StudyVariableColumnNominal;
        ct->at(4) = StudyVariableColumnNominal;
        ct->at(13) = StudyVariableColumnNominal;

        ct->at(1) = StudyVariableColumnNumerical;
        ct->at(2) = StudyVariableColumnNumerical;
        ct->at(5) = StudyVariableColumnNumerical;
        ct->at(6) = StudyVariableColumnNumerical;
        ct->at(7) = StudyVariableColumnNumerical;
        ct->at(8) = StudyVariableColumnNumerical;
        ct->at(9) = StudyVariableColumnNumerical;
        ct->at(10) = StudyVariableColumnNumerical;
        ct->at(11) = StudyVariableColumnNumerical;
        ct->at(12) = StudyVariableColumnNumerical;
        ImportData *import_data = new ImportData("examples/Tablesets/Plasma-Singapore.csv", "", COLUMN_PIVOT_TABLE, ct);

        LipidSpace *ll = new LipidSpace();
        ll->ignore_unknown_lipids = true;
        ll->ignore_doublette_lipids = true;
        ll->load_table(import_data);
        ll->analysis_finished = false;
        ll->process_id = 1;
        ll->start();
        delete import_data;
        cout << ll->lipidomes.size() << endl;

        cin >> a;
        cout << a << " " << endl;

        delete ll;
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
    else {
        QApplication application(argc, argv);
        LipidSpace lipid_space;
        lipid_space.ignore_unknown_lipids = true;
        lipid_space.ignore_doublette_lipids = true;
        //lipid_space.load_mzTabM("examples/mzTabM/singaporean-plasma-site1.mzTab");
        lipid_space.load_mzTabM("Lab1-Phase1-ShotgunMS.mzTab");
    }


    /*

    if (argc < 4) {
        print_help();
        exit(-1);
    }

    map<string, int> options = {{"-sn", 0}, {"-i", 1}, {"-d", 2}, {"-p", 3}, {"-h", 4}, {"--help", 4}, {"-u", 5}, {"-q", 6}};


    int num_opt = 0;
    while (true){
        if (uncontains_val(options, string(argv[1 + num_opt]))){
            if (argv[1 + num_opt][0] == '-'){
                cerr << "Error: option " << argv[1 + num_opt] << " unknown." << endl;
                exit(-1);
            }
            break;
        }
        int opt = options.at(argv[1 + num_opt++]);

        switch (opt){
            case 0: keep_sn_position = false; break;
            case 1: ignore_unknown_lipids = true; break;
            case 2: storing_distance_table = true; break;
            case 3: plot_pca_lipidomes = false; break;
            case 4: print_help(); return 0;
            case 5: unboundend_distance = true; break;
            case 6: without_quant = true; break;
        }
    }


    // getting input
    string output_folder = argv[1 + num_opt];
    string mode = argv[2 + num_opt];

    // check if output folder exists
    struct stat buffer;
    if (stat (output_folder.c_str(), &buffer) != 0){
        cerr << "Error: output folder '" << output_folder << "' does not exist." << endl;
        exit(-1);
    }


    if (mode != "table" && mode != "lists") {
        print_help();
        exit(-1);
    }

    Logging::write_log("LipidSpace v.1.0.1 was launched in terminal mode");
    LipidSpace lipid_space;


    lipid_space.keep_sn_position = keep_sn_position;
    lipid_space.ignore_unknown_lipids = ignore_unknown_lipids;
    lipid_space.unboundend_distance = unboundend_distance;
    lipid_space.without_quant = without_quant;
    lipid_space.cols_for_pca = 7;


    // loadig each lipidome
    if (mode == "lists"){
        for (int i = 3 + num_opt; i < argc; ++i) lipid_space.lipidomes.push_back(lipid_space.load_list(argv[i]));
    }
    // loading lipid matrix
    else {
        lipid_space.load_row_table(argv[3 + num_opt]);
    }

    // compute PCA matrixes for the complete lipidome
    lipid_space.compute_global_distance_matrix();


    // storing the distance matrix
    if (storing_distance_table){
        lipid_space.store_distance_table(lipid_space.global_lipidome, output_folder);
    }

    // perform the principal component analysis
    {
        Matrix pca;
        cout << "Running principal component analysis" << endl;
        lipid_space.global_lipidome->m.PCA(pca, lipid_space.cols_for_pca);
        lipid_space.global_lipidome->m.rewrite(pca);
    }


    // cutting the global PCA matrix back to a matrix for each lipidome
    lipid_space.separate_matrixes();

    // plotting all lipidome PCAs
    if (plot_pca && lipidomes.size() > 1){
        lipid_space.plot_PCA(global_lipidome, output_folder);
    }
    if (plot_pca_lipidomes){
        for (auto table : lipidomes){
            lipid_space.plot_PCA(table, output_folder);
        }
    }


    // normalize and incorporate intensities
    lipid_space.normalize_intensities();



    if (lipid_space.lipidomes.size() > 1){

        // computing the hausdorff distance matrix for all lipidomes
        lipid_space.compute_hausdorff_matrix();

        // storing hausdorff distance matrix into file
        if (storing_distance_table)
            lipid_space.report_hausdorff_matrix(output_folder);

        // ploting the dendrogram
        //lipid_space.plot_dendrogram(&lipidomes, distance_matrix, output_folder);
    }

    */
    return 0;
}
