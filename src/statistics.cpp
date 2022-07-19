#include "lipidspace/statistics.h"




Statistics::Statistics() {
    chart = 0;
    lipid_space = 0;
    log_scale = false;
}


void Statistics::load_data(LipidSpace *_lipid_space, Chart *_chart){
    lipid_space = _lipid_space;
    chart = _chart;
	chart->setRenderHint(QPainter::Antialiasing);
    chart->set_tick_size(GlobalData::gui_num_var["tick_size"]);
    chart->set_title_size(GlobalData::gui_num_var["legend_size"]);

}



void Statistics::set_log_scale(){
    log_scale = !log_scale;
    updateBarPlot();
}


void Statistics::setLegendSizeBoxPlot(int font_size){
    GlobalData::gui_num_var["legend_size"] = font_size;
    chart->set_title_size(font_size);
    updateBoxPlot();
}



void Statistics::setLegendSizeBarPlot(int font_size){
    GlobalData::gui_num_var["legend_size"] = font_size;
    chart->set_title_size(font_size);
    updateBarPlot();
}



void Statistics::setLegendSizeHistogram(int font_size){
    GlobalData::gui_num_var["legend_size"] = font_size;
    chart->set_title_size(font_size);
    updateHistogram();
}



void Statistics::setLegendSizeROCCurve(int font_size){
    GlobalData::gui_num_var["legend_size"] = font_size;
    chart->set_title_size(font_size);
    updateROCCurve();
}



void Statistics::setTickSizeROCCurve(int font_size){
    GlobalData::gui_num_var["tick_size"] = font_size;
    chart->set_tick_size(font_size);
    updateROCCurve();
}



void Statistics::setTickSizeBoxPlot(int font_size){
    GlobalData::gui_num_var["tick_size"] = font_size;
    chart->set_tick_size(font_size);
    updateBoxPlot();
}



void Statistics::setTickSizeBarPlot(int font_size){
    GlobalData::gui_num_var["tick_size"] = font_size;
    chart->set_tick_size(font_size);
    updateBarPlot();
}



void Statistics::setTickSizeHistogram(int font_size){
    GlobalData::gui_num_var["tick_size"] = font_size;
    chart->set_tick_size(font_size);
    updateHistogram();
}



void Statistics::setBarNumber(int bar_number){
    GlobalData::gui_num_var["bar_number"] = bar_number;
    updateHistogram();
}



void Statistics::exportData(){
    if (chart->chart_plots.size() == 0) return;
    QString file_name = QFileDialog::getSaveFileName(chart, "Export data", GlobalData::last_folder, "Worksheet *.xlsx (*.xlsx);;Data Table *.csv (*.csv);;Data Table *.tsv (*.tsv)");
    if (!file_name.length()) return;

    try {
        if (QFile::exists(file_name)){
            QFile::remove(file_name);
        }

        if (file_name.toLower().endsWith("csv") || file_name.toLower().endsWith("tsv")){
            string sep = file_name.toLower().endsWith("csv") ? "," : "\t";

            ofstream off(file_name.toStdString().c_str());
            for (uint i = 0; i < series_titles.size(); ++i){
                off << series_titles[i];
                if (i < series_titles.size() - 1) off << sep;
            }
            off << endl;

            uint num_rows = 0;
            for (auto s : series) num_rows = max(num_rows, (uint)s.size());
            for (uint i = 0; i < num_rows; ++i){
                for (uint j = 0; j < series.size(); ++j){
                    if (i < series[j].size()){
                        off << series[j][i];
                        if (j < series.size() - 1) off << sep;
                    }
                    else if (j < series.size() - 1) off << sep;
                }
                off << endl;
            }
        }
        else if (file_name.toLower().endsWith("xlsx")) {
            XLDocument doc;
            doc.create(file_name.toStdString());
            auto wbk = doc.workbook();
            wbk.addWorksheet("Data");
            wbk.addWorksheet("Statistics");
            wbk.deleteSheet("Sheet1");
            auto wks_data = doc.workbook().worksheet("Data");
            uint col = 1;
            for (auto t : series_titles) wks_data.cell(1, col++).value() = t;

            for (col = 0; col < series.size(); ++col){
                int row = 2;
                for (auto s : series[col]) wks_data.cell(row++, col + 1).value() = s;
            }

            auto wks_stat = doc.workbook().worksheet("Statistics");
            int row = 1;
            for (auto kv : stat_results){
                wks_stat.cell(row, 1).value() = kv.first;
                wks_stat.cell(row++, 2).value() = kv.second;
            }

            doc.save();
        }
        else {
            QMessageBox::information(chart, "Export error", "Unknown export format for file '" + file_name + "'.");
            return;
        }
        QMessageBox::information(chart, "Export completed", "The export is completed into the file '" + file_name + "'.");
    }
    catch(exception &){
        QMessageBox::critical(chart, "Error during export", "An error occurred during the export into the file '" + file_name + "'. Is your hard disk drive full by chance or do you have enough permissions to write files into this folder?");
    }
}



void Statistics::exportAsPdf(){
    if (chart->chart_plots.size() == 0) return;
    QString file_name = QFileDialog::getSaveFileName(chart, "Export as pdf", GlobalData::last_folder, "*.pdf (*.pdf)");
    if (!file_name.length()) return;

    try {
        QFileInfo fi(file_name);
        GlobalData::last_folder = fi.absoluteDir().absolutePath();

        QPrinter printer(QPrinter::ScreenResolution);
        printer.setOutputFormat(QPrinter::PdfFormat);
        printer.setFontEmbeddingEnabled(true);
        QPageSize pageSize(QSizeF(chart->viewport()->width(), chart->viewport()->height()) , QPageSize::Point);
        printer.setPageSize(pageSize);

        printer.setOutputFileName(file_name);

        // set margins to 0
    #if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        QMarginsF margins;
        margins.setTop(0);
        margins.setLeft(0);
        margins.setRight(0);
        margins.setBottom(0);
        printer.setPageMargins(margins);
    #else
        QPagedPaintDevice::Margins margins;
        margins.top = 0;
        margins.left = 0;
        margins.right = 0;
        margins.bottom = 0;
        printer.setMargins(margins);
    #endif


        QPainter p(&printer);
        chart->render(&p);
        p.end();

        QMessageBox::information(chart, "Export completed", "The export is completed into the file '" + file_name + "'.");
    }
    catch(exception &){
        QMessageBox::critical(chart, "Error during export", "An error occurred during the export into the file '" + file_name + "'. Is your hard disk drive full by chance or do you have enough permissions to write files into this folder?");
    }
}




void Statistics::updateBarPlot(){
    /*
    chart->removeAllSeries();
    for (auto axis : chart->axes()){
        chart->removeAxis(axis);
    }
    series_titles.clear();
    series.clear();
    chart->setTitle("");
    stat_results.clear();

    string target_variable = GlobalData::gui_string_var["study_var_stat"];
    if (!lipid_space || uncontains_val(lipid_space->feature_values, target_variable) || !lipid_space->analysis_finished) return;

    bool is_nominal = lipid_space->feature_values[target_variable].feature_type == NominalFeature;
    map<string, string> &translations = lipid_space->lipid_name_translations[GlobalData::gui_num_var["translate"]];



    // setup array for target variable values, if nominal then each with incrementing number
    map<string, double> nominal_target_values;
    Array target_values;
    vector<string> target_titles;
    int nom_counter = 0;
    vector<QBoxPlotSeries*> plot_series;
    vector<vector<Array>> data_series;

    if (is_nominal){
        for (auto lipidome : lipid_space->selected_lipidomes){
            string nominal_value = lipidome->features[target_variable].nominal_value;
            if (uncontains_val(nominal_target_values, nominal_value)){
                nominal_target_values.insert({nominal_value, nom_counter++});
                QBoxPlotSeries *box_plot_series = new QBoxPlotSeries();
                plot_series.push_back(box_plot_series);
                data_series.push_back(vector<Array>());
                connect(box_plot_series, SIGNAL(hovered(bool, QBoxSet*)), this, SLOT(bar_plot_hovered(bool, QBoxSet*)));
                box_plot_series->setName(nominal_value.c_str());
                target_titles.push_back(nominal_value);
                string color_key = target_variable + "_" + nominal_value;
                if (contains_val(GlobalData::colorMapFeatures, color_key)){
                    QBrush brush(GlobalData::colorMapFeatures[color_key]);
                    box_plot_series->setBrush(brush);
                }
            }
            target_values.push_back(nominal_target_values[nominal_value]);
        }
    }
    else {
        string lipid_label = "Selected lipids";
        nominal_target_values.insert({lipid_label, nom_counter++});
        QBoxPlotSeries *box_plot_series = new QBoxPlotSeries();
        plot_series.push_back(box_plot_series);
        data_series.push_back(vector<Array>());
        connect(box_plot_series, SIGNAL(hovered(bool, QBoxSet*)), this, SLOT(bar_plot_hovered(bool, QBoxSet*)));
        box_plot_series->setName(lipid_label.c_str());
        target_titles.push_back(lipid_label);
        target_values.push_back(nominal_target_values[lipid_label]);
        QBrush brush(QColor("#F6A611"));
        box_plot_series->setBrush(brush);
    }

    Matrix statistics_matrix;
    map<LipidAdduct*, int> lipid_map;
    map<string, int> lipid_name_map;
    // setting up lipid to column in matrix map
    for (uint i = 0; i < lipid_space->global_lipidome->lipids.size(); ++i){
        LipidAdduct* lipid = lipid_space->global_lipidome->lipids[i];
        if (uncontains_val(lipid_map, lipid)){
            lipid_map.insert({lipid, lipid_map.size()});
            lipid_name_map.insert({translations[lipid_space->global_lipidome->species[i]], lipid_name_map.size()});
        }
    }

    // set up matrix for multiple linear regression
    statistics_matrix.reset(lipid_space->selected_lipidomes.size(), lipid_map.size());
    for (uint r = 0; r < lipid_space->selected_lipidomes.size(); ++r){
        Lipidome* lipidome = lipid_space->selected_lipidomes[r];
        for (uint i = 0; i < lipidome->selected_lipid_indexes.size(); ++i){
            int index = lipidome->selected_lipid_indexes[i];
            if (contains_val(lipid_map, lipidome->lipids[index])){
                statistics_matrix(r, lipid_map[lipidome->lipids[index]]) = lipidome->normalized_intensities[index];
            }
        }
    }

    vector<string> lipid_names(lipid_map.size(), "");
    for (auto kv : lipid_name_map) lipid_names[kv.second] = kv.first;

    for (int t = 0; t < nom_counter; ++t){
        for (int c = 0; c < statistics_matrix.cols; c++){
            series_titles.push_back(lipid_names[c] + " / " + target_titles[t]);
            data_series[t].push_back(Array());
        }
    }

    if (is_nominal){
        for (int r = 0; r < statistics_matrix.rows; ++r){ // for all values of a study variable
            for (int c = 0; c < statistics_matrix.cols; c++){
                if (statistics_matrix(r, c) > 1e-15){
                    data_series[target_values[r]][c].push_back(statistics_matrix(r, c));
                }
            }
        }
    }
    else {
        for (uint r = 0; r < lipid_space->selected_lipidomes.size(); ++r){
            Lipidome* lipidome = lipid_space->selected_lipidomes[r];
            for (uint i = 0; i < lipidome->selected_lipid_indexes.size(); ++i){
                int index = lipidome->selected_lipid_indexes[i];
                if (contains_val(lipid_map, lipidome->lipids[index])){
                    string lipid_category = lipidome->lipids[index]->get_lipid_string(CATEGORY);
                    int c = lipid_map[lipidome->lipids[index]];
                    data_series[0][c].push_back(statistics_matrix(r, c));
                }
            }
        }
        plot_series[0]->setName(QString("Selected lipids (%2)").arg(data_series[0].size()));
    }


    series.resize(series_titles.size());
    int series_iter = 0;
    QLogValueAxis *axisY = 0;
    QBarCategoryAxis *axisX = 0;
    if (log_scale){
        axisY = new QLogValueAxis();
        axisY->setLabelFormat("%g");
        axisY->setBase(10.0);
        chart->addAxis(axisY, Qt::AlignLeft);

        axisX = new QBarCategoryAxis();
        for (auto lipid_name : lipid_names){
            QString category = lipid_name.c_str();
            axisX->append(category);
        }
        chart->addAxis(axisX, Qt::AlignBottom);
    }
    double min_y_val = 1e100;
    double max_y_val = 0;


    for (uint plot_it = 0; plot_it < data_series.size(); ++plot_it){
        auto single_plot_series = plot_series[plot_it];
        auto single_data_series = data_series[plot_it];

        for (uint box_it = 0; box_it < single_data_series.size(); ++box_it){
            Array &vals = single_data_series[box_it];
            auto box = new QBoxSet(lipid_names[box_it].c_str());

            for (uint i = 0; i < vals.size(); ++i){
                series[series_iter].push_back(vals[i]);
            }
            ++series_iter;

            double mean = vals.mean();
            if (log_scale && !isnan(mean) && !isinf(mean)) min_y_val = min(min_y_val, mean);
            if (isnan(mean) || isinf(mean) || mean < 1e-19) mean = log_scale ? 1e-19 : 0;
            double std = vals.sample_stdev();
            if (isnan(std) || isinf(std)) std = 0;
            max_y_val = max(max_y_val, mean + std);

            box->setValue(QBoxSet::LowerExtreme, log_scale ? 1e-19 : 0);
            box->setValue(QBoxSet::UpperExtreme, mean + std);
            box->setValue(QBoxSet::Median, log_scale ? 1e-19 : 0);
            box->setValue(QBoxSet::LowerQuartile, log_scale ? 1e-19 : 0);
            box->setValue(QBoxSet::UpperQuartile, mean);
            single_plot_series->append(box);
        }
        single_plot_series->setBoxWidth(1.5);
        chart->addSeries(single_plot_series);
        if (log_scale){
            single_plot_series->attachAxis(axisY);
        }
    }

    if (log_scale){
        axisY->setMin(min_y_val / 10.);
        axisY->setMax(max_y_val * 2.);
    }
    else {
        chart->createDefaultAxes();
    }


    for (auto axis : chart->axes()){
        axis->setLabelsFont(QFont("Helvetica", GlobalData::gui_num_var["tick_size"]));
    }
    */
}







void Statistics::updateHistogram(){
    /*

    chart->removeAllSeries();
    for (auto axis : chart->axes()){
        chart->removeAxis(axis);
    }
    series_titles.clear();
    series.clear();
    chart->setTitle("");
    stat_results.clear();

    string target_variable = GlobalData::gui_string_var["study_var_stat"];
    if (!lipid_space || uncontains_val(lipid_space->feature_values, target_variable) || !lipid_space->analysis_finished) return;

    bool do_continue = (lipid_space->feature_values[target_variable].feature_type == NominalFeature) && (lipid_space->selected_lipidomes.size() > 1);
    setVisible(do_continue);

    if (!do_continue) return;

    // setup array for target variable values, if nominal then each with incrementing number
    map<string, double> nominal_target_values;
    Indexes target_indexes;
    Array target_values;
    int nom_counter = 0;
    vector<QBarSet*> plot_series;

    string secondary_target_variable = GlobalData::gui_string_var["secondary_var"];
    bool has_secondary = contains_val(lipid_space->feature_values, secondary_target_variable);

    for (auto lipidome : lipid_space->selected_lipidomes){
        string nominal_value = lipidome->features[target_variable].nominal_value;
        if (uncontains_val(nominal_target_values, nominal_value)){
            nominal_target_values.insert({nominal_value, nom_counter++});
            plot_series.push_back(new QBarSet(nominal_value.c_str()));
            series_titles.push_back(nominal_value);
            string color_key = target_variable + "_" + nominal_value;
            if (contains_val(GlobalData::colorMapFeatures, color_key)){
                QColor color(GlobalData::colorMapFeatures[color_key]);
                color.setAlpha(128);
                plot_series.back()->setBrush(QBrush(color));
                plot_series.back()->setPen(Qt::NoPen);
            }
        }
        target_indexes.push_back(nominal_target_values[nominal_value]);
        if (has_secondary) target_values.push_back(lipidome->features[secondary_target_variable].numerical_value);
    }

    double all_min = 1e9;
    double all_max = -1e9;
    series.resize(nom_counter);
    if (has_secondary){
        for (uint r = 0; r < target_values.size(); ++r){
            double value = target_values[r];
            series[target_indexes[r]].push_back(value);
            all_min = min(all_min, value);
            all_max = max(all_max, value);
        }
    }
    else {
        Matrix statistics_matrix(lipid_space->statistics_matrix);
        if (statistics_matrix.cols > 1) statistics_matrix.scale();

        for (int r = 0; r < statistics_matrix.rows; ++r){
            double sum = 0;
            for (int c = 0; c < statistics_matrix.cols; c++) sum += statistics_matrix(r, c);
            if (statistics_matrix.cols > 1 || sum > 1e-15){
                series[target_indexes[r]].push_back(sum);
                all_min = min(all_min, sum);
                all_max = max(all_max, sum);
            }
        }
    }

    double num_bars = GlobalData::gui_num_var["bar_number"];
    double bar_size = (all_max - all_min) / num_bars;

    QValueAxis *axisX = new QValueAxis();
    QValueAxis *axisY = new QValueAxis();
    axisX->setRange(all_min, all_max);
    axisY->setRange(0, 1);
    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);
    int max_hist = 0;
    for (uint i = 0; i < plot_series.size(); ++i){
        plot_series[i]->setLabel((plot_series[i]->label() + " (%1)").arg(series[i].size()));
        Array &single_series = series[i];

        vector<int> counts(num_bars + 1, 0);
        for (auto val : single_series){
            max_hist = max(max_hist, ++counts[int((val - all_min) / bar_size)]);
        }

        QBarSeries *bar_series = new QBarSeries();
        bar_series->setBarWidth(plot_series.size());
        QBarSet *set = plot_series[i];
        for (auto cnt : counts) set->append(cnt);
        bar_series->append(set);
        chart->addSeries(bar_series);
        bar_series->attachAxis(axisY);
    }
    axisY->setRange(0, max_hist);

    for (auto axis : chart->axes()){
        axis->setLabelsFont(QFont("Helvetica", GlobalData::gui_num_var["tick_size"]));
    }
    */
}








void Statistics::updateROCCurve(){
    /*

    chart->removeAllSeries();
    for (auto axis : chart->axes()){
        chart->removeAxis(axis);
    }
    series_titles.clear();
    series.clear();
    chart->setTitle("");
    stat_results.clear();

    string target_variable = GlobalData::gui_string_var["study_var_stat"];
    if (!lipid_space || uncontains_val(lipid_space->feature_values, target_variable) || !lipid_space->analysis_finished) return;

    bool do_continue = (lipid_space->feature_values[target_variable].feature_type == NominalFeature) && (lipid_space->selected_lipidomes.size() > 1);
    setVisible(do_continue);

    if (!do_continue) return;


    // setup array for target variable values, if nominal then each with incrementing number
    map<string, double> nominal_target_values;
    Array target_values;
    int nom_counter = 0;
    for (auto lipidome : lipid_space->selected_lipidomes){
        string nominal_value = lipidome->features[target_variable].nominal_value;
        if (uncontains_val(nominal_target_values, nominal_value)){
            nominal_target_values.insert({nominal_value, nom_counter++});
            series_titles.push_back(nominal_value);
            string color_key = target_variable + "_" + nominal_value;
            if (contains_val(GlobalData::colorMapFeatures, color_key)){
                QBrush brush(GlobalData::colorMapFeatures[color_key]);
            }
        }
        target_values.push_back(nominal_target_values[nominal_value]);
    }

    if (nom_counter != 2){
        setVisible(false);
        return;
    }



    Matrix statistics_matrix(lipid_space->statistics_matrix);

    if (statistics_matrix.cols > 1) statistics_matrix.scale();

    series.resize(nom_counter);
    for (int r = 0; r < statistics_matrix.rows; ++r){
        double sum = 0;
        for (int c = 0; c < statistics_matrix.cols; c++) sum += statistics_matrix(r, c);
        if (statistics_matrix.cols > 1 || sum > 1e-15){
            series[target_values[r]].push_back(sum);
        }
    }

    pair<vector<double>, vector<double>> ROC;
    double dist = 0;
    double pos_max = 0;
    double sep_score = 0;

    ks_separation_value(series[0], series[1], dist, pos_max, sep_score, &ROC);

    QLineSeries *line_series = new QLineSeries();
    QLineSeries *random_series = new QLineSeries();
    *random_series << QPointF(0, 0) << QPointF(1, 1);
    double auc = 0, fp = 0;

    bool flip = median(series[0], 0, series[0].size()) > median(series[1], 0, series[1].size());
    if (flip) swap(ROC.first, ROC.second);



    if (ROC.second[0] != 1){
        ROC.first.insert(ROC.first.begin(), ROC.first[0]);
        ROC.second.insert(ROC.second.begin(), 1);
    }
    if (ROC.first[0] != 1){
        ROC.first.insert(ROC.first.begin(), 1);
        ROC.second.insert(ROC.second.begin(), 1);
    }
    if (ROC.first[ROC.first.size() - 1] != 0){
        ROC.first.push_back(0);
        ROC.second.push_back(ROC.second.back());
    }
    if (ROC.second[ROC.second.size() - 1] != 0){
        ROC.first.push_back(0);
        ROC.second.push_back(0);
    }

    for (uint i = 0; i < ROC.first.size(); ++i){
        line_series->append(ROC.first[i], ROC.second[i]);
        auc += (ROC.second[i] - fp) * ROC.first[i];
        fp = ROC.second[i];
    }

    stat_results.insert({"AUC", auc});

    QValueAxis *axisX = new QValueAxis;
    axisX->setRange(-0.001, 1.001);
    axisX->setTitleText("False positive Rate");
    axisX->setLabelsFont(QFont("Helvetica", GlobalData::gui_num_var["tick_size"]));
    chart->addAxis(axisX, Qt::AlignBottom);

    QValueAxis *axisY = new QValueAxis;
    axisY->setRange(-0.001, 1.001);
    axisY->setTitleText("Sensetivity");
    axisY->setLabelsFont(QFont("Helvetica", GlobalData::gui_num_var["tick_size"]));
    chart->addAxis(axisY, Qt::AlignLeft);

    chart->addSeries(line_series);
    chart->addSeries(random_series);
    chart->setTitle(QString("ROC Curve, AUC = %1 %").arg(QString::number(auc * 100., 'g', 4)));
    chart->legend()->hide();
    */
}










void Statistics::updateBoxPlot(){
    chart->clear();
    chart->setTitle("");
    chart->setVisible(true);

    series_titles.clear();
    series.clear();
    stat_results.clear();

    string target_variable = GlobalData::gui_string_var["study_var_stat"];
    if (!lipid_space || uncontains_val(lipid_space->feature_values, target_variable) || !lipid_space->analysis_finished) return;

    bool is_nominal = lipid_space->feature_values[target_variable].feature_type == NominalFeature;

    if (lipid_space->selected_lipidomes.size() <= 1){
        chart->setVisible(false);
        return;
    }

    Matrix statistics_matrix(lipid_space->statistics_matrix);

    // setup array for target variable values, if nominal then each with incrementing number
    map<string, double> nominal_target_values;
    Indexes target_indexes;
    Array target_values;
    int nom_counter = 0;
    vector<string> nominal_values;

    string secondary_target_variable = GlobalData::gui_string_var["secondary_var"];
    bool has_secondary = contains_val(lipid_space->feature_values, secondary_target_variable);

    if (is_nominal){
        for (auto lipidome : lipid_space->selected_lipidomes){
            if (lipidome->features[target_variable].missing) continue;
            if (has_secondary && lipidome->features[secondary_target_variable].missing) continue;

            string nominal_value = lipidome->features[target_variable].nominal_value;
            if (uncontains_val(nominal_target_values, nominal_value)){
                nominal_target_values.insert({nominal_value, nom_counter++});
                nominal_values.push_back(nominal_value);
                series_titles.push_back(nominal_value);
            }
            target_indexes.push_back(nominal_target_values[nominal_value]);
            if (has_secondary) target_values.push_back(lipidome->features[secondary_target_variable].numerical_value);


        }
    }
    else {
        for (auto lipidome : lipid_space->selected_lipidomes){
            if (lipidome->features[target_variable].missing) continue;
            if (has_secondary && lipidome->features[secondary_target_variable].missing) continue;

            target_values.push_back(lipidome->features[target_variable].numerical_value);

            if (has_secondary){
                string nominal_value = lipidome->features[secondary_target_variable].nominal_value;
                if (uncontains_val(nominal_target_values, nominal_value)){
                    nominal_target_values.insert({nominal_value, nom_counter++});
                    nominal_values.push_back(nominal_value);
                    series_titles.push_back(nominal_value);
                }
                target_indexes.push_back(nominal_target_values[nominal_value]);
            }
        }
    }

    // if any lipidome has a missing study variable, discard the lipidome from the statistic
    Indexes lipidomes_to_keep;
    for (int r = 0; r < statistics_matrix.rows; ++r){
        if (!lipid_space->selected_lipidomes[r]->features[target_variable].missing) lipidomes_to_keep.push_back(r);
    }
    Matrix tmp;
    tmp.rewrite(statistics_matrix, lipidomes_to_keep);
    statistics_matrix.rewrite(tmp);


    if (is_nominal){

        series.resize(nom_counter);
        if (has_secondary){
            for (uint r = 0; r < target_indexes.size(); ++r){
                series[target_indexes[r]].push_back(target_values[r]);
            }
        }
        else {
            if (statistics_matrix.cols > 1) statistics_matrix.scale();
            for (int r = 0; r < statistics_matrix.rows; ++r){
                double sum = 0;
                for (int c = 0; c < statistics_matrix.cols; c++) sum += statistics_matrix(r, c);
                if (statistics_matrix.cols > 1 || sum > 1e-15){
                    series[target_indexes[r]].push_back(sum);
                }
            }
        }
        Boxplot* boxplot = new Boxplot(chart);
        for (uint i = 0; i < nominal_values.size(); ++i){
            Array &single_series = series[i];
            QString category = QString(nominal_values[i].c_str()) + QString(" (%1)").arg(series[i].size());
            string color_key = target_variable + "_" + nominal_values[i];
            QColor color = contains_val(GlobalData::colorMapFeatures, color_key) ? GlobalData::colorMapFeatures[color_key] : Qt::white;

            boxplot->add(single_series, category, color);
        }
        chart->add(boxplot);

        double accuracy = compute_accuracy(series);
        stat_results.insert({"accuracy", accuracy});
        if (nom_counter == 2){
            double p_student = p_value_student(series[0], series[1]);
            double p_welch = p_value_welch(series[0], series[1]);
            double p_ks = p_value_kolmogorov_smirnov(series[0], series[1]);
            double accuracy = compute_accuracy(series);
            stat_results.insert({"p_value(Student)", p_student});
            stat_results.insert({"p_value(Welch)", p_welch});
            stat_results.insert({"p_value(KS)", p_ks});
            chart->setTitle(QString("Statistics: accuracy = %1,   <i>p</i>-value<sub>Student</sub> = %2,   <i>p</i>-value<sub>Welch</sub> = %3,   <i>p</i>-value<sub>KS</sub> = %4").arg(QString::number(accuracy, 'g', 3)).arg(QString::number(p_student, 'g', 3)).arg(QString::number(p_welch, 'g', 3)).arg(QString::number(p_ks, 'g', 3)));
        }
        else if (nom_counter > 2){
            double p_anova = p_value_anova(series);
            stat_results.insert({"p_value(ANOVA)", p_anova});
            chart->setTitle(QString("Statistics: accuracy = %1,   <i>p</i>-value<sub>ANOVA</sub> = %2").arg(QString::number(accuracy, 'g', 3)).arg(QString::number(p_anova, 'g', 3)));
        }
    }
    else {
        Array constants(statistics_matrix.rows, 1);
        statistics_matrix.add_column(constants);

        Array coefficiants;

        coefficiants.compute_coefficiants(statistics_matrix, target_values);    // estimating coefficiants
        Array S;
        S.mult(statistics_matrix, coefficiants);

        Scatterplot *scatterplot = new Scatterplot(chart);
        if (!has_secondary){
            vector< pair<double, double> > data;
            for (uint i = 0; i < S.size(); ++i){
                data.push_back({S[i], target_values[i]});
            }
            scatterplot->add(data, target_variable.c_str());

        }
        else {
            vector< vector< pair<double, double> > > data(S.size());
            for (uint i = 0; i < S.size(); ++i){
                data[target_indexes[i]].push_back({S[i], target_values[i]});
            }

            for (auto nominal_value : nominal_values){
                string color_key = secondary_target_variable + "_" + nominal_value;
                QColor c = contains_val(GlobalData::colorMapFeatures, color_key) ? GlobalData::colorMapFeatures[color_key] : QColor("209fdf");
                scatterplot->add(data[nominal_target_values[nominal_value]], nominal_value.c_str(), c);
            }

        }
        chart->add(scatterplot);



        double mx = 0, my = 0, ny = 0;
        // computing the study variable mean based on missing values of lipids
        for (uint r = 0; r < S.size(); ++r){
            if (S[r] <= 1e-15) continue;
            mx += S[r];
            my += target_values[r];
            ny += 1;
        }
        mx /= ny;
        my /= ny;

        // estimate slope and intercept factors for linear regression
        double slope_num = 0, slope_denom = 0;
        for (uint r = 0; r < S.size(); ++r){
            if (S[r] <= 1e-15) continue;
            slope_num += (S[r] - mx) * (target_values[r] - my);
            slope_denom += sq(S[r] - mx);
        }
        double slope = slope_num / slope_denom;
        double intercept = my - slope * mx;

        double SQR = 0, SQT = 0;
        for (uint r = 0; r < S.size(); ++r){
            if (S[r] <= 1e-15) continue;
            SQR += sq(target_values[r] - (slope * S[r] + intercept));
            SQT += sq(target_values[r] - my);
        }
        double R2 = 1. - SQR / SQT;
        stat_results.insert({"R^2", R2});

        Lineplot *regression_line = new Lineplot(chart);
        QString rg_name = QString().asprintf("Regression model (R<sup>2</sup> = %0.3f)", R2);
        vector< pair<pair<double, double>, pair<double, double> > > data;

        double min_x = chart->xrange.x();
        double max_x = chart->xrange.y();
        data.push_back({{min_x, slope * min_x + intercept}, {max_x, slope * max_x + intercept}});
        regression_line->add(data, rg_name, QColor("#99ca53"));
        chart->add(regression_line);

        QString sign = intercept >= 0 ? "+" : "-";
        chart->setTitle(QString("Linear regression model"));
    }
}
