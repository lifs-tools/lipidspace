#include "lipidspace/globaldata.h"


int GlobalData::alpha = DEFAULT_ALPHA;
bool GlobalData::showQuant = true;
bool GlobalData::debug = false;
bool GlobalData::ctrl_pressed = false;
bool GlobalData::in_tutorial = false;
bool GlobalData::benford_warning = false;
int GlobalData::color_counter = 0;
map<string, QColor> GlobalData::colorMap;
int GlobalData::PC1 = 0;
int GlobalData::PC2 = 1;
bool GlobalData::selected_view = false;
Linkage GlobalData::linkage = AverageLinkage;
int GlobalData::study_variable_counter = 0;
string GlobalData::LipidSpace_version = "v0.9.9";
string GlobalData::rest_temp_folder = ".";
string GlobalData::normalization = "absolute";
NormalizationType GlobalData::normalization_type = AbsoluteNormalization;
QString GlobalData::last_folder = "";
bool GlobalData::stat_level_lipidomes = true;
map<string, QColor> GlobalData::colorMapStudyVariables;
string GlobalData::vocano_multiple = "bh";
string GlobalData::volcano_test = "student";
map<string, string> GlobalData::gui_string_var{{"species_selection", ""}, {"study_var", ""}, {"study_var_stat", ""}, {"secondary_var", ""}};
map<string, double> GlobalData::gui_num_var{{"pie_tree_depth", 3}, {"dendrogram_height", 100}, {"pie_size", 100}, {"translate", 1}, {"label_size", 18}, {"tick_size", 10}, {"legend_size", 14}, {"dont_less_3_message", 0}, {"bar_number", 20}, {"table_zoom", 10}};

const vector<QColor> GlobalData::COLORS{QColor("#1f77b4"), QColor("#ff7f0e"), QColor("#2ca02c"), QColor("#d62728"), QColor("#9467bd"), QColor("#8c564b"), QColor("#e377c2"), QColor("#bcbd22"), QColor("#17becf")};

map<string, QColor> GlobalData::colorMapCategories{{"GL", QColor("#3B73B8")}, {"GP", QColor("#93C01F")}, {"SP", QColor("#F6A611")}, {"ST",  QColor("#86134E")}, {"FA", QColor("#ca26d3")}, {"SL", QColor("#ff7f0e")}};
