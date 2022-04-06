#include "lipidspace/globaldata.h"


int GlobalData::alpha = DEFAULT_ALPHA;
bool GlobalData::showQuant = true;
int GlobalData::color_counter = 0;
map<string, QColor> GlobalData::colorMap;
int GlobalData::PC1 = 0;
int GlobalData::PC2 = 1;
int GlobalData::pie_tree_depth = 3;
int GlobalData::feature_counter = 0;
int GlobalData::dendrogram_height = 100;
string GlobalData::normalization = "absolute";
map<string, QColor> GlobalData::colorMapFeatures;

const vector<QColor> GlobalData::COLORS{QColor("#1f77b4"), QColor("#ff7f0e"), QColor("#2ca02c"), QColor("#d62728"), QColor("#9467bd"), QColor("#8c564b"), QColor("#e377c2"), QColor("#bcbd22"), QColor("#17becf")};
