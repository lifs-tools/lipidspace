#ifndef GLOBALDATA_H
#define GLOBALDATA_H

#include <map>
#include <vector>
#include <string>
#include <QColor>
#include <lipidspace/AssistanceFunctions.h>

using namespace std;

#define DEFAULT_ALPHA 80

class GlobalData {
public:
    static const vector<QColor> COLORS;
    static bool showQuant;
    static bool ctrl_pressed;
    static bool debug;
    static bool selected_view;
    static bool benford_warning;
    static int alpha;
    static map<string, QColor> colorMap;
    static int PC1;
    static int PC2;
    static string rest_temp_folder;
    static map<string, QColor> colorMapStudyVariables;
    static map<string, QColor> colorMapCategories;
    static string normalization;
    static Linkage linkage;
    static string LipidSpace_version;
    static QString last_folder;
    static map<string, string> gui_string_var;
    static map<string, double> gui_num_var;
    static bool in_tutorial;
    static string enrichment_correction;
    static string enrichment_test;
    static double enrichment_sig;
    static double enrichment_log_fc;
    static StatLevel stat_level;
    static set<string> FAD_lipid_classes;
    static set<string> first_enrichment_classes;
    static set<string> second_enrichment_classes;
};


#endif // GLOBALDATA_H
