#include "lipidspace/globaldata.h"


int GlobalData::alpha = DEFAULT_ALPHA;
bool GlobalData::showQuant = true;
int GlobalData::color_counter = 0;
map<string, QColor> GlobalData::colorMap;
int GlobalData::PC1 = 0;
int GlobalData::PC2 = 1;
Linkage GlobalData::linkage = AverageLinkage;
int GlobalData::feature_counter = 0;
string GlobalData::LipidSpace_version = "v0.x.x";
string GlobalData::normalization = "absolute";
QString GlobalData::last_folder = "";
map<string, QColor> GlobalData::colorMapFeatures;
map<string, string> GlobalData::gui_string_var{{"species_selection", ""}, {"study_var", ""}};
map<string, double> GlobalData::gui_num_var{{"pie_tree_depth", 3}, {"dendrogram_height", 100}, {"pie_size", 100}, {"translate", 0}};

const vector<QColor> GlobalData::COLORS{QColor("#1f77b4"), QColor("#ff7f0e"), QColor("#2ca02c"), QColor("#d62728"), QColor("#9467bd"), QColor("#8c564b"), QColor("#e377c2"), QColor("#bcbd22"), QColor("#17becf")};
