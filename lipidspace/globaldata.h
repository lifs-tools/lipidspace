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
    static int color_counter;
    static const vector<QColor> COLORS;
    static bool showQuant;
    static int alpha;
    static map<string, QColor> colorMap;
    static int PC1;
    static int PC2;
    static int feature_counter;
    static map<string, QColor> colorMapFeatures;
    static int pie_tree_depth;
    static int dendrogram_height;
    static string normalization;
    static Linkage linkage;
    static string LipidSpace_version;
};


#endif // GLOBALDATA_H
