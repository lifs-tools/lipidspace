#include "lipidspace/statistics.h"




Statistics::Statistics(QWidget *parent) : QChartView(parent) {
    lipid_space = 0;
    chart = new QChart();
    chart->setAnimationOptions(QChart::SeriesAnimations);
    chart->createDefaultAxes();
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);
    setChart(chart);
    chart->legend()->setFont(QFont("Helvetica", 14));
}


void Statistics::set_lipid_space(LipidSpace *_lipid_space){
    lipid_space = _lipid_space;
}


void Statistics::updateChart(){
    chart->removeAllSeries();
    
    string target_variable = GlobalData::gui_string_var["study_var"];
    if (!lipid_space || uncontains_val(lipid_space->feature_values, target_variable) || lipid_space->feature_values[target_variable].feature_type != NominalFeature || !lipid_space->analysis_finished) return;
        
    
    bool is_nominal = lipid_space->feature_values[target_variable].feature_type == NominalFeature;
    
    
    // setup array for target variable values, if nominal then each with incrementing number
    map<string, double> nominal_target_values;
    vector<double> target_values;
    int nom_counter = 0;
    vector<QBoxPlotSeries*> plot_series;
    if (is_nominal){
        for (auto lipidome : lipid_space->selected_lipidomes){
            string nominal_value = lipidome->features[target_variable].nominal_value;
            if (uncontains_val(nominal_target_values, nominal_value)){
                nominal_target_values.insert({nominal_value, nom_counter++});
                plot_series.push_back(new QBoxPlotSeries());
                plot_series.back()->setName(nominal_value.c_str());
                plot_series.back()->append(new QBoxSet());
                string color_key = target_variable + "_" + nominal_value;
                if (contains_val(GlobalData::colorMapFeatures, color_key)){
                    QBrush brush(GlobalData::colorMapFeatures[color_key]);
                    plot_series.back()->setBrush(brush);
                }
            }
            target_values.push_back(nominal_target_values[nominal_value]);
        }
    }
    else {
        for (auto lipidome : lipid_space->selected_lipidomes){
            target_values.push_back(lipidome->features[target_variable].numerical_value);
        }
    }
    Matrix global_matrix;
    map<LipidAdduct*, int> lipid_map;
    map<string, int> lipid_name_map;
    // setting up lipid to column in matrix map
    for (uint i = 0; i < lipid_space->global_lipidome->lipids.size(); ++i){
        LipidAdduct* lipid = lipid_space->global_lipidome->lipids[i];
        if (uncontains_val(lipid_map, lipid)){
            lipid_map.insert({lipid, lipid_map.size()});
            lipid_name_map.insert({lipid_space->global_lipidome->species[i], lipid_name_map.size()});
        }
    }
    
    // set up matrix for multiple linear regression
    global_matrix.reset(lipid_space->selected_lipidomes.size(), lipid_map.size());
    for (uint r = 0; r < lipid_space->selected_lipidomes.size(); ++r){
        Lipidome* lipidome = lipid_space->selected_lipidomes[r];
        for (uint i = 0; i < lipidome->lipids.size(); ++i){
            if (contains_val(lipid_map, lipidome->lipids[i])){
                global_matrix(r, lipid_map[lipidome->lipids[i]]) = lipidome->original_intensities[i];
            }
        }
    }
    if (global_matrix.cols > 1) global_matrix.scale();
    
    
    for (int r = 0; r < global_matrix.rows; ++r){
        double sum = 0;
        for (int c = 0; c < global_matrix.cols; c++) sum += global_matrix(r, c);
        if (global_matrix.cols > 1 || sum > 1e-15) plot_series[target_values[r]]->boxSets()[0]->append(sum);
    }
    for (auto series : plot_series) chart->addSeries(series);
    chart->createDefaultAxes();
    for (auto axis : chart->axes()){
        if (axis->orientation() == Qt::Horizontal) chart->removeAxis(axis);
    }
}
