#include "lipidspace/statistics.h"




Statistics::Statistics() {
    chart = 0;
    lipid_space = 0;
    log_scale = false;
    show_data = false;
    show_pvalues = false;
}


void Statistics::load_data(LipidSpace *_lipid_space, Chart *_chart){
    lipid_space = _lipid_space;
    chart = _chart;
	chart->setRenderHint(QPainter::Antialiasing);
    chart->setTickSize(GlobalData::gui_num_var["tick_size"]);
    chart->setTitleSize(GlobalData::gui_num_var["legend_size"]);
}



void Statistics::setLogScaleBarPlot(){
    log_scale = !log_scale;
    chart->setYLogScale(log_scale);
}



void Statistics::setStatResults(){
    show_pvalues = !show_pvalues;
    chart->setStatResults(show_pvalues);
}



void Statistics::setShowDataBoxPlot(){
    show_data = !show_data;
    chart->setShowDataPoints(show_data);
}



void Statistics::setShowDataBarPlot(){
    show_data = !show_data;
    chart->setShowDataPoints(show_data);
}


void Statistics::setLegendSize(int font_size){
    GlobalData::gui_num_var["legend_size"] = font_size;
    chart->setTitleSize(font_size);
}



void Statistics::setTickSize(int font_size){
    GlobalData::gui_num_var["tick_size"] = font_size;
    chart->setTickSize(font_size);
}



void Statistics::setBarNumberHistogram(int bar_number){
    GlobalData::gui_num_var["bar_number"] = bar_number;
    updateHistogram();
}



void Statistics::setBarNumberSpeciesCV(int bar_number){
    GlobalData::gui_num_var["bar_number"] = bar_number;
    updateSpeciesCV();
}



void Statistics::setBarNumberPvalues(int bar_number){
    GlobalData::gui_num_var["bar_number"] = bar_number;
    updatePVal();
}



void Statistics::highlightPoints(QListWidget* speciesList){
    if (chart->chart_plots.empty()) return;
    if (!instanceof<Scatterplot>(chart->chart_plots[0])) return;

    map<QString, ScPoint*> points = ((Scatterplot*)chart->chart_plots[0])->point_map;
    for (auto &kv : points) kv.second->highlight = false;

    for (auto item : speciesList->selectedItems()){
        QString lipid_name = item->text();
        if (contains_val(points, lipid_name)){
            points[lipid_name]->highlight = true;
        }
    }
    chart->update_chart();
}



void Statistics::highlightBars(QListWidget* speciesList){
    if (chart->chart_plots.empty()) return;
    if (!instanceof<Barplot>(chart->chart_plots[0])) return;

    map<QString, vector<BarBox*>*> bars = ((Barplot*)chart->chart_plots[0])->bar_map;
    for (auto &kv : bars){
        for (auto bar : *(kv.second)){
            bar->highlight = false;
        }
    }

    for (auto item : speciesList->selectedItems()){
        QString lipid_name = item->text();
        if (contains_val(bars, lipid_name)){
            for (auto bar : *(bars[lipid_name])){
                bar->highlight = true;
            }
        }
    }
    chart->update_chart();
}



void Statistics::exportData(){
    if (chart->chart_plots.size() == 0) return;
    QString file_name = QFileDialog::getSaveFileName(chart, "Export data", GlobalData::last_folder, "Worksheet *.xlsx (*.xlsx);;Data Table *.csv (*.csv);;Data Table *.tsv (*.tsv)");
    if (!file_name.length()) return;

    try {
        if (QFile::exists(file_name)){
            QFile::remove(file_name);
        }

        SortVector<string, Array*> sorted_series;
        for (uint i = 0; i < series_titles.size(); ++i){
            sorted_series.push_back({series_titles[i], &(series[i])});
        }
        sorted_series.sort_asc();


        if (file_name.toLower().endsWith("csv") || file_name.toLower().endsWith("tsv")){
            string sep = file_name.toLower().endsWith("csv") ? "," : "\t";

            ofstream off(file_name.toStdString().c_str());
            for (uint i = 0; i < sorted_series.size(); ++i){
                off << sorted_series[i].first;
                if (i < sorted_series.size() - 1) off << sep;
            }
            off << endl;

            uint num_rows = 0;
            for (auto s : sorted_series) num_rows = max(num_rows, (uint)s.second->size());
            for (uint i = 0; i < num_rows; ++i){
                for (uint j = 0; j < sorted_series.size(); ++j){
                    if (i < sorted_series[j].second->size()){
                        off << sorted_series[j].second->at(i);
                        if (j < sorted_series.size() - 1) off << sep;
                    }
                    else if (j < sorted_series.size() - 1) off << sep;
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
            wbk.addWorksheet("Flat-data");
            wbk.deleteSheet("Sheet1");
            auto wks_data = doc.workbook().worksheet("Data");
            uint col = 1;
            for (auto t : sorted_series) wks_data.cell(1, col++).value() = t.first;

            for (col = 0; col < sorted_series.size(); ++col){
                int row = 2;
                for (auto s : *(sorted_series[col].second)) wks_data.cell(row++, col + 1).value() = s;
            }

            auto wks_stat = doc.workbook().worksheet("Statistics");
            int row = 1;
            for (auto kv : stat_results){
                wks_stat.cell(row, 1).value() = kv.first;
                wks_stat.cell(row++, 2).value() = kv.second;
            }

            if (flat_data.size() > 0){
                auto wks_flat = doc.workbook().worksheet("Flat-data");
                col = 1;
                for (auto kv : flat_data){
                    wks_flat.cell(1, col).value() = kv.first;
                    for (uint row = 0; row < kv.second.size(); ++row){
                        QVariant var = kv.second[row];
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
                        if (var.typeId() == QMetaType::QString){
#else
                        if (var.type() == QVariant::String){
#endif
                            wks_flat.cell(row + 2, col).value() = var.toString().toStdString();
                        }
                        else {
                            wks_flat.cell(row + 2, col).value() = var.toDouble();
                        }
                    }

                    ++col;
                }
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



void Statistics::exportAsSvg(){
    if (chart->chart_plots.size() == 0) return;
    QString file_name = QFileDialog::getSaveFileName(chart, "Export as svg", GlobalData::last_folder, "*.svg (*.svg)");
    if (!file_name.length()) return;

    try {
        QFileInfo fi(file_name);
        GlobalData::last_folder = fi.absoluteDir().absolutePath();

        QSvgGenerator generator;
        generator.setFileName(file_name);
        generator.setSize(QSize(chart->viewport()->width(), chart->viewport()->height()));
        generator.setViewBox(QRect(0, 0, chart->viewport()->width(), chart->viewport()->height()));
        QPainter painter;
        painter.begin(&generator);
        chart->render(&painter);
        painter.end();

        QMessageBox::information(chart, "Export completed", "The export is completed into the file '" + file_name + "'.");
    }
    catch(exception &){
        QMessageBox::critical(chart, "Error during export", "An error occurred during the export into the file '" + file_name + "'. Is your hard disk drive full by chance or do you have enough permissions to write files into this folder?");
    }
}





void Statistics::updateBarPlotClasses(){

    chart->clear();
    chart->setTitle("");
    chart->setVisible(true);

    series_titles.clear();
    series.clear();
    flat_data.clear();
    stat_results.clear();

    string target_variable = GlobalData::gui_string_var["study_var"];
    if (!lipid_space || uncontains_val(lipid_space->study_variable_values, target_variable) || !lipid_space->analysis_finished){
        return;
    }


    string secondary_target_variable = GlobalData::gui_string_var["secondary_var"];
    SecondaryType secondary_type = NoSecondary;
    if (contains_val(lipid_space->study_variable_values, secondary_target_variable)){
        secondary_type = (lipid_space->study_variable_values[secondary_target_variable].study_variable_type == NominalStudyVariable) ? NominalSecondary : NumericalSecondary;
    }

    bool is_nominal = lipid_space->study_variable_values[target_variable].study_variable_type == NominalStudyVariable;

    if (lipid_space->selected_lipidomes.size() < 1 || secondary_type != NoSecondary || GlobalData::stat_level != LipidClassLevel){
        chart->setVisible(false);
        return;
    }



    // setup array for target variable values, if nominal then each with incrementing number
    map<string, double> nominal_target_values;
    Indexes target_indexes;
    Array target_values;
    int nom_counter = 0;
    vector<string> nominal_values;



    int valid_lipidomes = 0;

    if (is_nominal){
        for (auto lipidome : lipid_space->selected_lipidomes){
            if (lipidome->study_variables[target_variable].missing) continue;
            if (secondary_type != NoSecondary && lipidome->study_variables[secondary_target_variable].missing) continue;

            valid_lipidomes++;

            string nominal_value = lipidome->study_variables[target_variable].nominal_value;
            if (uncontains_val(nominal_target_values, nominal_value)){
                nominal_target_values.insert({nominal_value, nom_counter++});
                nominal_values.push_back(nominal_value);
            }
            target_indexes.push_back(nominal_target_values[nominal_value]);
            if (secondary_type != NoSecondary) target_values.push_back(lipidome->study_variables[secondary_target_variable].numerical_value);


        }
    }
    else {
        for (auto lipidome : lipid_space->selected_lipidomes){
            if (lipidome->study_variables[target_variable].missing) continue;
            if (secondary_type != NoSecondary && lipidome->study_variables[secondary_target_variable].missing) continue;

            valid_lipidomes++;

            target_values.push_back(lipidome->study_variables[target_variable].numerical_value);

            if (secondary_type == NominalSecondary){
                string nominal_value = lipidome->study_variables[secondary_target_variable].nominal_value;
                if (uncontains_val(nominal_target_values, nominal_value)){
                    nominal_target_values.insert({nominal_value, nom_counter++});
                    nominal_values.push_back(nominal_value);
                }
                target_indexes.push_back(nominal_target_values[nominal_value]);
            }
        }
        if (secondary_type != NominalSecondary) nom_counter += 1;
    }

    map<LipidAdduct*, int> lipid_class_pos;
    map<string, int> lipid_class_map;

    // setting up lipid to column in matrix map
    for (uint i = 0; i < lipid_space->global_lipidome->lipids.size(); ++i){
        LipidAdduct* lipid = lipid_space->global_lipidome->lipids[i];
        string ex_lipid_class = lipid->get_extended_class();
        if (uncontains_val(lipid_class_map, ex_lipid_class)){
            lipid_class_pos.insert({lipid, lipid_class_pos.size()});
            lipid_class_map.insert({ex_lipid_class, lipid_class_map.size()});
        }
    }

    // set up matrix for multiple linear regression
    Matrix stat_matrix;
    stat_matrix.reset(valid_lipidomes, lipid_class_pos.size());
    for (uint r = 0, rr = 0; r < lipid_space->selected_lipidomes.size(); ++r){
        Lipidome* lipidome = lipid_space->selected_lipidomes[r];

        if (lipidome->study_variables[target_variable].missing) continue;
        if (secondary_type != NoSecondary && lipidome->study_variables[secondary_target_variable].missing) continue;


        for (uint i = 0; i < lipidome->selected_lipid_indexes.size(); ++i){
            int index = lipidome->selected_lipid_indexes[i];
            if (contains_val(lipid_class_pos, lipidome->lipids[index])){
                stat_matrix(rr, lipid_class_pos[lipidome->lipids[index]]) += lipidome->normalized_intensities[index];
            }
        }
        rr++;
    }

    vector<QString> *lipid_class_names = new vector<QString>(lipid_class_pos.size(), "");
    for (auto kv : lipid_class_map) lipid_class_names->at(kv.second) = kv.first.c_str();
    vector< vector<Array> > data_series(nom_counter);

    for (int t = 0; t < nom_counter; ++t){
        for (int c = 0; c < stat_matrix.cols; c++){
            series_titles.push_back(lipid_class_names->at(c).toStdString() + (nom_counter > 1 ? " / " + nominal_values[t] : ""));
            data_series[t].push_back(Array());
        }
    }


    if (is_nominal || (secondary_type == NominalSecondary)){
        for (int r = 0; r < stat_matrix.rows; ++r){ // for all values of a study variable
            for (int c = 0; c < stat_matrix.cols; c++){
                if (stat_matrix(r, c) > 1e-15){
                    data_series[target_indexes[r]][c].push_back(stat_matrix(r, c));
                }
            }
        }
    }
    else {
        for (uint r = 0, rr = 0; r < lipid_space->selected_lipidomes.size(); ++r){
            Lipidome* lipidome = lipid_space->selected_lipidomes[r];

            if (lipidome->study_variables[target_variable].missing) continue;
            if (secondary_type != NoSecondary && lipidome->study_variables[secondary_target_variable].missing) continue;


            for (int index : lipidome->selected_lipid_indexes){
                if (contains_val(lipid_class_pos, lipidome->lipids[index])){
                    string lipid_category = lipidome->lipids[index]->get_lipid_string(CATEGORY);
                    int c = lipid_class_pos[lipidome->lipids[index]];
                    data_series[0][c].push_back(stat_matrix(rr, c));
                }
            }
            rr++;
        }
    }

    series.resize(series_titles.size());

    vector<QString> *categories = new vector<QString>();
    vector<QColor> *colors = new vector<QColor>();
    for (auto nominal_value : nominal_values) categories->push_back(nominal_value.c_str());
    if (is_nominal || (secondary_type == NominalSecondary)){
        for (auto nominal_value : nominal_values){
            string color_key = ((is_nominal || (secondary_type == NoSecondary)) ? target_variable : secondary_target_variable) + "_" + nominal_value;
            colors->push_back(contains_val(GlobalData::colorMapStudyVariables, color_key) ? GlobalData::colorMapStudyVariables[color_key] : QColor("#F6A611"));
        }
    }
    else {
        categories->push_back(QString("Selected lipids (%1)").arg(lipid_class_names->size()));
        colors->push_back(QColor("#F6A611"));
    }

    vector< vector< Array > > *barplot_data = new vector< vector<Array> >(lipid_class_names->size());

    int series_iter = 0;
    for (uint cat_it = 0; cat_it < data_series.size(); ++cat_it){
        auto category_data_series = data_series[cat_it];
        for (uint lipid_it = 0; lipid_it < category_data_series.size(); ++lipid_it){
            auto single_series = category_data_series[lipid_it];
            barplot_data->at(lipid_it).push_back(Array());

            for (auto value : single_series){
                series[series_iter].push_back(value);
                barplot_data->at(lipid_it).back().push_back(value);
            }
            ++series_iter;
        }

    }

    Barplot *barplot = new Barplot(chart, log_scale, show_data, show_pvalues);
    barplot->add(barplot_data, categories, lipid_class_names, colors);
    chart->add(barplot);

    connect(barplot, &Barplot::enterLipid, this, &Statistics::lipidEntered);
    connect(barplot, &Barplot::exitLipid, this, &Statistics::lipidExited);
}







void Statistics::updateBarPlot(){

    chart->clear();
    chart->setTitle("");
    chart->setVisible(true);

    series_titles.clear();
    series.clear();
    flat_data.clear();
    stat_results.clear();

    string target_variable = GlobalData::gui_string_var["study_var"];
    if (!lipid_space || uncontains_val(lipid_space->study_variable_values, target_variable) || !lipid_space->analysis_finished){
        return;
    }


    string secondary_target_variable = GlobalData::gui_string_var["secondary_var"];
    SecondaryType secondary_type = NoSecondary;
    if (contains_val(lipid_space->study_variable_values, secondary_target_variable)){
        secondary_type = (lipid_space->study_variable_values[secondary_target_variable].study_variable_type == NominalStudyVariable) ? NominalSecondary : NumericalSecondary;
    }

    bool is_nominal = lipid_space->study_variable_values[target_variable].study_variable_type == NominalStudyVariable;

    if (lipid_space->selected_lipidomes.size() < 1 || secondary_type != NoSecondary || GlobalData::stat_level != LipidSpeciesLevel){
        chart->setVisible(false);
        return;
    }



    // setup array for target variable values, if nominal then each with incrementing number
    map<string, double> nominal_target_values;
    Indexes target_indexes;
    Array target_values;
    int nom_counter = 0;
    vector<string> nominal_values;



    int valid_lipidomes = 0;

    if (is_nominal){
        for (auto lipidome : lipid_space->selected_lipidomes){
            if (lipidome->study_variables[target_variable].missing) continue;
            if (secondary_type != NoSecondary && lipidome->study_variables[secondary_target_variable].missing) continue;

            valid_lipidomes++;

            string nominal_value = lipidome->study_variables[target_variable].nominal_value;
            if (uncontains_val(nominal_target_values, nominal_value)){
                nominal_target_values.insert({nominal_value, nom_counter++});
                nominal_values.push_back(nominal_value);
            }
            target_indexes.push_back(nominal_target_values[nominal_value]);
            if (secondary_type != NoSecondary) target_values.push_back(lipidome->study_variables[secondary_target_variable].numerical_value);


        }
    }
    else {
        for (auto lipidome : lipid_space->selected_lipidomes){
            if (lipidome->study_variables[target_variable].missing) continue;
            if (secondary_type != NoSecondary && lipidome->study_variables[secondary_target_variable].missing) continue;

            valid_lipidomes++;

            target_values.push_back(lipidome->study_variables[target_variable].numerical_value);

            if (secondary_type == NominalSecondary){
                string nominal_value = lipidome->study_variables[secondary_target_variable].nominal_value;
                if (uncontains_val(nominal_target_values, nominal_value)){
                    nominal_target_values.insert({nominal_value, nom_counter++});
                    nominal_values.push_back(nominal_value);
                }
                target_indexes.push_back(nominal_target_values[nominal_value]);
            }
        }
        if (secondary_type != NominalSecondary) nom_counter += 1;
    }

    map<LipidAdduct*, int> lipid_map;
    map<string, int> lipid_name_map;
    map<string, string> &translations = lipid_space->lipid_name_translations[(int)GlobalData::gui_num_var["translate"]];

    // setting up lipid to column in matrix map
    for (uint i = 0; i < lipid_space->global_lipidome->lipids.size(); ++i){
        LipidAdduct* lipid = lipid_space->global_lipidome->lipids[i];
        if (uncontains_val(lipid_map, lipid)){
            lipid_map.insert({lipid, lipid_map.size()});
            lipid_name_map.insert({translations[lipid_space->global_lipidome->species[i]], lipid_name_map.size()});
        }
    }

    // set up matrix for multiple linear regression
    Matrix stat_matrix;
    stat_matrix.reset(valid_lipidomes, lipid_map.size());
    for (uint r = 0, rr = 0; r < lipid_space->selected_lipidomes.size(); ++r){
        Lipidome* lipidome = lipid_space->selected_lipidomes[r];

        if (lipidome->study_variables[target_variable].missing) continue;
        if (secondary_type != NoSecondary && lipidome->study_variables[secondary_target_variable].missing) continue;


        for (uint i = 0; i < lipidome->selected_lipid_indexes.size(); ++i){
            int index = lipidome->selected_lipid_indexes[i];
            if (contains_val(lipid_map, lipidome->lipids[index])){
                stat_matrix(rr, lipid_map[lipidome->lipids[index]]) = lipidome->normalized_intensities[index];
            }
        }
        rr++;
    }

    vector<QString> *lipid_names = new vector<QString>(lipid_map.size(), "");
    for (auto kv : lipid_name_map) lipid_names->at(kv.second) = kv.first.c_str();
    vector< vector<Array> > data_series(nom_counter);

    for (int t = 0; t < nom_counter; ++t){
        for (int c = 0; c < stat_matrix.cols; c++){
            series_titles.push_back(lipid_names->at(c).toStdString() + (nom_counter > 1 ? " / " + nominal_values[t] : ""));
            data_series[t].push_back(Array());
        }
    }


    if (is_nominal || (secondary_type == NominalSecondary)){
        for (int r = 0; r < stat_matrix.rows; ++r){ // for all values of a study variable
            for (int c = 0; c < stat_matrix.cols; c++){
                if (stat_matrix(r, c) > 1e-15){
                    data_series[target_indexes[r]][c].push_back(stat_matrix(r, c));
                }
            }
        }
    }
    else {
        for (uint r = 0, rr = 0; r < lipid_space->selected_lipidomes.size(); ++r){
            Lipidome* lipidome = lipid_space->selected_lipidomes[r];

            if (lipidome->study_variables[target_variable].missing) continue;
            if (secondary_type != NoSecondary && lipidome->study_variables[secondary_target_variable].missing) continue;


            for (int index : lipidome->selected_lipid_indexes){
                if (contains_val(lipid_map, lipidome->lipids[index])){
                    string lipid_category = lipidome->lipids[index]->get_lipid_string(CATEGORY);
                    int c = lipid_map[lipidome->lipids[index]];
                    data_series[0][c].push_back(stat_matrix(rr, c));
                }
            }
            rr++;
        }
    }

    series.resize(series_titles.size());

    vector<QString> *categories = new vector<QString>();
    vector<QColor> *colors = new vector<QColor>();
    for (auto nominal_value : nominal_values) categories->push_back(nominal_value.c_str());
    if (is_nominal || (secondary_type == NominalSecondary)){
        for (auto nominal_value : nominal_values){
            string color_key = ((is_nominal || (secondary_type == NoSecondary)) ? target_variable : secondary_target_variable) + "_" + nominal_value;
            colors->push_back(contains_val(GlobalData::colorMapStudyVariables, color_key) ? GlobalData::colorMapStudyVariables[color_key] : QColor("#F6A611"));
        }
    }
    else {
        categories->push_back(QString("Selected lipids (%1)").arg(lipid_names->size()));
        colors->push_back(QColor("#F6A611"));
    }

    vector< vector< Array > > *barplot_data = new vector< vector<Array> >(lipid_names->size());

    int series_iter = 0;
    for (uint cat_it = 0; cat_it < data_series.size(); ++cat_it){
        auto category_data_series = data_series[cat_it];
        for (uint lipid_it = 0; lipid_it < category_data_series.size(); ++lipid_it){
            auto single_series = category_data_series[lipid_it];
            barplot_data->at(lipid_it).push_back(Array());

            for (auto value : single_series){
                series[series_iter].push_back(value);
                barplot_data->at(lipid_it).back().push_back(value);
            }
            ++series_iter;
        }

    }

    Barplot *barplot = new Barplot(chart, log_scale, show_data, show_pvalues);
    barplot->add(barplot_data, categories, lipid_names, colors);
    chart->add(barplot);

    connect(barplot, &Barplot::enterLipid, this, &Statistics::lipidEntered);
    connect(barplot, &Barplot::exitLipid, this, &Statistics::lipidExited);
    connect(barplot, &Barplot::markLipid, this, &Statistics::lipidMarked);
}













void Statistics::lipidEntered(string lipid_name){
    emit enterLipid(lipid_name);
}


void Statistics::lipidExited(){
    emit exitLipid();
}


void Statistics::lipidMarked(){
    emit markLipid();
}


void Statistics::lipidsMarked(set<string> *lipids){
    emit markLipids(lipids);
}












void Statistics::updateSpeciesCV(){

    chart->clear();
    chart->setTitle("Lipid species - Coefficients of Variation [%]");
    chart->setVisible(true);

    series_titles.clear();
    series.clear();
    flat_data.clear();
    stat_results.clear();

    string target_variable = GlobalData::gui_string_var["study_var"];
    if (!lipid_space || uncontains_val(lipid_space->study_variable_values, target_variable) || !lipid_space->analysis_finished){
        return;
    }


    string secondary_target_variable = GlobalData::gui_string_var["secondary_var"];
    SecondaryType secondary_type = NoSecondary;
    if (contains_val(lipid_space->study_variable_values, secondary_target_variable)){
        secondary_type = (lipid_space->study_variable_values[secondary_target_variable].study_variable_type == NominalStudyVariable) ? NominalSecondary : NumericalSecondary;
    }

    bool is_nominal = lipid_space->study_variable_values[target_variable].study_variable_type == NominalStudyVariable;

    if (lipid_space->selected_lipidomes.size() < 1 || secondary_type != NoSecondary || GlobalData::stat_level != LipidSpeciesLevel){
        chart->setVisible(false);
        return;
    }



    // setup array for target variable values, if nominal then each with incrementing number
    map<string, double> nominal_target_values;
    int nom_counter = 0;
    vector<string> nominal_values;
    vector< map<LipidAdduct*, Array> > lipid_map;


    if (is_nominal){
        for (auto lipidome : lipid_space->selected_lipidomes){
            if (lipidome->study_variables[target_variable].missing) continue;
            if (secondary_type != NoSecondary && lipidome->study_variables[secondary_target_variable].missing) continue;

            string nominal_value = lipidome->study_variables[target_variable].nominal_value;
            if (uncontains_val(nominal_target_values, nominal_value)){
                nominal_target_values.insert({nominal_value, nom_counter++});
                nominal_values.push_back(nominal_value);
                lipid_map.push_back(map<LipidAdduct*, Array>());
            }
        }
    }
    else {
        lipid_map.push_back(map<LipidAdduct*, Array>());
        nom_counter = 1;
    }


    // set up lipid map for the several nominal values
    for (uint r = 0; r < lipid_space->selected_lipidomes.size(); ++r){
        Lipidome* lipidome = lipid_space->selected_lipidomes[r];

        if (lipidome->study_variables[target_variable].missing) continue;
        if (secondary_type != NoSecondary && lipidome->study_variables[secondary_target_variable].missing) continue;

        for (uint i = 0; i < lipidome->selected_lipid_indexes.size(); ++i){
            int index = lipidome->selected_lipid_indexes[i];
            int rr = is_nominal ? nominal_target_values[lipidome->study_variables[target_variable].nominal_value] : 0;

            if (uncontains_val(lipid_map[rr], lipidome->lipids[index])){
                lipid_map[rr].insert({lipidome->lipids[index], Array()});
            }
            lipid_map[rr][lipidome->lipids[index]].push_back(lipidome->normalized_intensities[index]);
        }
    }

    series.resize(nom_counter);

    vector<QString> categories;
    vector<QColor> colors;
    double min_value = INFINITY;
    double max_value = -INFINITY;
    for (uint i = 0; i < nominal_values.size(); ++i){
        auto nominal_value = nominal_values[i];
        auto lipid_cat_map = lipid_map[i];
        for (auto kv : lipid_cat_map){

            if (kv.second.size() <= 1) continue;
            double value = kv.second.stdev() / kv.second.mean() * 100;
            series[i].push_back(value);
            min_value = __min(min_value, value);
            max_value = __max(max_value, value);
        }

        categories.push_back(QString("%1 (%2)").arg(nominal_value.c_str()).arg(series[i].size()));
        series_titles.push_back(nominal_value);
    }
    if (is_nominal){
        for (auto nominal_value : nominal_values){
            string color_key = ((is_nominal || (secondary_type == NoSecondary)) ? target_variable : secondary_target_variable) + "_" + nominal_value;
            colors.push_back(contains_val(GlobalData::colorMapStudyVariables, color_key) ? GlobalData::colorMapStudyVariables[color_key] : QColor("#F6A611"));
        }
    }
    else {
        categories.push_back(QString("Selected lipids (%1)").arg(lipid_map[0].size()));
        series_titles.push_back("Selected lipids (" + std::to_string(lipid_map[0].size()) + ")");
        colors.push_back(QColor("#F6A611"));
    }


    if (min_value == INFINITY || max_value == -INFINITY || (max_value - min_value < 1e-20)) {
        chart->setVisible(false);
        return;
    }


    Histogramplot* histogramplot = new Histogramplot(chart);
    double num_bars = contains_val(GlobalData::gui_num_var, "bar_number") ? GlobalData::gui_num_var["bar_number"] : 20;
    histogramplot->add(series, categories, &colors, num_bars);
    histogramplot->borders.setX(0);
    chart->xrange.setX(0);
    chart->add(histogramplot);
}

















void Statistics::updateFAD(){
    chart->clear();
    chart->setTitle("");
    chart->setVisible(true);

    series_titles.clear();
    series.clear();
    flat_data.clear();
    stat_results.clear();

    string numerical_string = "Fatty acyls";
    string target_variable = GlobalData::gui_string_var["study_var"];

    bool do_continue = (lipid_space != 0) && contains_val(lipid_space->study_variable_values, target_variable) && lipid_space->analysis_finished && (lipid_space->selected_lipidomes.size() > 1) && (GlobalData::stat_level == FattyAcylLipid);

    chart->setVisible(do_continue);
    if (!do_continue || GlobalData::FAD_lipid_classes.empty()) return;

    bool is_nominal = lipid_space->study_variable_values[target_variable].study_variable_type == NominalStudyVariable;


    // fa->cat->value(s)
    vector< vector< Array > > *fatty_acids_values = new vector< vector<Array> >();
    map<string, int> fa_dict;

    // setup array for target variable values, if nominal then each with incrementing number
    map<string, int> nominal_target_values;
    map<string, int> nominal_target_count;
    map<string, map<Lipidome*, int>> lipidome_map;
    vector<QString> *categories = new vector<QString>();
    vector<QColor> *colors = new vector<QColor>();
    vector<QString> *fa_names_list = new vector<QString>();

    if (is_nominal){
        for (auto &kv : lipid_space->study_variable_values[target_variable].nominal_values){
            if (kv.second){
                nominal_target_values.insert({kv.first, categories->size()});
                nominal_target_count.insert({kv.first, 0});
                categories->push_back(kv.first.c_str());

                lipidome_map.insert({kv.first, map<Lipidome*, int>()});

                string color_key = target_variable + "_" + kv.first;
                colors->push_back(contains_val(GlobalData::colorMapStudyVariables, color_key) ? GlobalData::colorMapStudyVariables[color_key] : QColor("#F6A611"));
            }
        }

    }
    else {
        nominal_target_values.insert({numerical_string, categories->size()});
        categories->push_back(numerical_string.c_str());
        lipidome_map.insert({numerical_string, map<Lipidome*, int>()});
        colors->push_back(QColor("#F6A611"));
    }

    for (auto lipidome : lipid_space->selected_lipidomes){
        if (lipidome->study_variables[target_variable].missing) continue;

        string nominal = is_nominal ? lipidome->study_variables[target_variable].nominal_value : numerical_string;
        lipidome_map[nominal].insert({lipidome, nominal_target_count[nominal]++});
    }


    for (auto lipidome : lipid_space->selected_lipidomes){
        if (uncontains_val(lipidome->study_variables, target_variable) || lipidome->study_variables[target_variable].missing) continue;

        string nominal = is_nominal ? lipidome->study_variables[target_variable].nominal_value : numerical_string;
        if (uncontains_val(lipidome_map, nominal) || uncontains_val(lipidome_map[nominal], lipidome)) continue;

        uint array_pos = lipidome_map[nominal][lipidome];
        for (uint index : lipidome->selected_lipid_indexes){
            if (index >= lipidome->lipids.size() || index >= lipidome->normalized_intensities.size()) continue;
            LipidAdduct *lipid = lipidome->lipids[index];
            double value = lipidome->normalized_intensities[index];

            if (uncontains_val(GlobalData::FAD_lipid_classes, lipidome->classes[index])) continue;

            for (auto fa : lipid->lipid->fa_list){
                if (fa->num_carbon == 0 || fa->lipid_FA_bond_type == LCB_REGULAR || fa->lipid_FA_bond_type == LCB_EXCEPTION) continue;

                string fa_string = fa->to_string(MOLECULAR_SPECIES);
                if (uncontains_val(fa_dict, fa_string)){
                    fa_dict.insert({fa_string, fa_names_list->size()});
                    fa_names_list->push_back(fa_string.c_str());
                    fatty_acids_values->push_back(vector<Array>(categories->size(), Array(nominal_target_count[nominal], 0)));
                }
                if (uncontains_val(fa_dict, fa_string) || fa_dict.at(fa_string) >= (int)fatty_acids_values->size()) continue;
                vector<Array> &vec = fatty_acids_values->at(fa_dict.at(fa_string));
                uint vec_pos = is_nominal ? nominal_target_values.at(lipidome->study_variables[target_variable].nominal_value) : 0;

                if (vec_pos < vec.size() && array_pos < vec.at(vec_pos).size()) vec.at(vec_pos)[array_pos] += value;
            }
        }
    }

    // remove all added zero values by the method
    for (auto &vec : *fatty_acids_values){
        for (auto &array : vec) array.remove_below(1e-15);
    }


    Barplot *barplot = new Barplot(chart, log_scale, show_data, show_pvalues);
    barplot->add(fatty_acids_values, categories, fa_names_list, colors);
    chart->add(barplot);
}










void Statistics::updateHistogram(){

    chart->clear();
    chart->setTitle("");
    chart->setVisible(true);

    series_titles.clear();
    series.clear();
    flat_data.clear();
    stat_results.clear();


    string target_variable = GlobalData::gui_string_var["study_var"];
    bool do_continue = (lipid_space != 0) && contains_val(lipid_space->study_variable_values, target_variable) && lipid_space->analysis_finished && (lipid_space->study_variable_values[target_variable].study_variable_type == NominalStudyVariable) && (lipid_space->selected_lipidomes.size() > 1) && (GlobalData::stat_level == LipidomeLevel);

    chart->setVisible(do_continue);
    if (!do_continue) return;

    // setup array for target variable values, if nominal then each with incrementing number
    map<string, double> nominal_target_values;
    Indexes target_indexes;
    Array target_values;
    int nom_counter = 0;
    vector<string> nominal_values;

    string secondary_target_variable = GlobalData::gui_string_var["secondary_var"];
    bool has_secondary = contains_val(lipid_space->study_variable_values, secondary_target_variable);

    for (auto lipidome : lipid_space->selected_lipidomes){
        if (lipidome->study_variables[target_variable].missing) continue;
        if (has_secondary && lipidome->study_variables[secondary_target_variable].missing) continue;

        string nominal_value = lipidome->study_variables[target_variable].nominal_value;
        if (uncontains_val(nominal_target_values, nominal_value)){
            nominal_target_values.insert({nominal_value, nom_counter++});
            nominal_values.push_back(nominal_value);
            series_titles.push_back(nominal_value);
        }
        target_indexes.push_back(nominal_target_values[nominal_value]);
        if (has_secondary) target_values.push_back(lipidome->study_variables[secondary_target_variable].numerical_value);
    }

    Matrix statistics_matrix(lipid_space->statistics_matrix);

    // if any lipidome has a missing study variable, discard the lipidome from the statistic
    Indexes lipidomes_to_keep;
    for (int r = 0; r < statistics_matrix.rows; ++r){
        if (!lipid_space->selected_lipidomes[r]->study_variables[target_variable].missing) lipidomes_to_keep.push_back(r);
    }
    Matrix tmp;
    tmp.rewrite(statistics_matrix, lipidomes_to_keep);
    statistics_matrix.rewrite(tmp);


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
            for (int c = 0; c < statistics_matrix.cols; c++){
                double val = statistics_matrix(r, c);
                if (!isnan(val) && !isinf(val)) sum += val;
            }
            if (statistics_matrix.cols > 1 || sum > 1e-15){
                series[target_indexes[r]].push_back(sum);
            }
        }
    }

    Histogramplot* histogramplot = new Histogramplot(chart);
    vector<QString> categories;
    vector<QColor> colors;
    for (uint i = 0; i < nominal_values.size(); ++i){
        categories.push_back(QString(nominal_values[i].c_str()) + QString(" (%1)").arg(series[i].size()));
        string color_key = target_variable + "_" + nominal_values[i];
        QColor color = contains_val(GlobalData::colorMapStudyVariables, color_key) ? GlobalData::colorMapStudyVariables[color_key] : Qt::white;
        colors.push_back(color);
    }

    double num_bars = contains_val(GlobalData::gui_num_var, "bar_number") ? GlobalData::gui_num_var["bar_number"] : 20;
    histogramplot->add(series, categories, &colors, num_bars);
    chart->add(histogramplot);

    double accuracy = compute_accuracy(series);
    stat_results.push_back({"accuracy", accuracy});
    chart->setTitle(QString("accuracy = %1").arg(QString::number(accuracy, 'g', 3)));
}
























void Statistics::updateROCCurve(){

    chart->clear();
    chart->setTitle("");
    chart->setVisible(true);

    series_titles.clear();
    series.clear();
    flat_data.clear();
    stat_results.clear();

    string target_variable = GlobalData::gui_string_var["study_var"];
    bool do_continue = (lipid_space != 0) && contains_val(lipid_space->study_variable_values, target_variable) && lipid_space->analysis_finished && (lipid_space->study_variable_values[target_variable].study_variable_type == NominalStudyVariable) && (lipid_space->selected_lipidomes.size() > 1) && (GlobalData::stat_level == LipidomeLevel);

    chart->setVisible(do_continue);
    if (!do_continue){
        return;
    }


    // setup array for target variable values, if nominal then each with incrementing number
    map<string, double> nominal_target_values;
    Indexes target_indexes;
    Array target_values;
    int nom_counter = 0;
    vector<string> nominal_values;

    string secondary_target_variable = GlobalData::gui_string_var["secondary_var"];
    bool has_secondary = contains_val(lipid_space->study_variable_values, secondary_target_variable);

    for (auto lipidome : lipid_space->selected_lipidomes){
        if (lipidome->study_variables[target_variable].missing) continue;
        if (has_secondary && lipidome->study_variables[secondary_target_variable].missing) continue;

        string nominal_value = lipidome->study_variables[target_variable].nominal_value;
        if (uncontains_val(nominal_target_values, nominal_value)){
            nominal_target_values.insert({nominal_value, nom_counter++});
            nominal_values.push_back(nominal_value);
        }
        target_indexes.push_back(nominal_target_values[nominal_value]);
        if (has_secondary) target_values.push_back(lipidome->study_variables[secondary_target_variable].numerical_value);


    }

    if (nom_counter != 2){
        chart->setVisible(false);
        return;
    }



    Matrix statistics_matrix(lipid_space->statistics_matrix);

    // if any lipidome has a missing study variable, discard the lipidome from the statistic
    Indexes lipidomes_to_keep;
    for (int r = 0; r < statistics_matrix.rows; ++r){
        if (!lipid_space->selected_lipidomes[r]->study_variables[target_variable].missing) lipidomes_to_keep.push_back(r);
    }
    Matrix tmp;
    tmp.rewrite(statistics_matrix, lipidomes_to_keep);
    statistics_matrix.rewrite(tmp);

    if (statistics_matrix.cols > 1) statistics_matrix.scale();
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
            for (int c = 0; c < statistics_matrix.cols; c++){
                double val = statistics_matrix(r, c);
                if (!isnan(val) && !isinf(val)) sum += val;
            }
            if (statistics_matrix.cols > 1 || sum > 1e-15){
                series[target_indexes[r]].push_back(sum);
            }
        }
    }

    pair<vector<double>, vector<double>> ROC;
    double dist = 0;
    double pos_max = 0;

    ks_separation_value(series[0], series[1], dist, pos_max, &ROC);

    Lineplot *lineplot = new Lineplot(chart);
    Lineplot *diagonalplot = new Lineplot(chart);


    double auc = 0, fp = 0;

    bool flip = Boxplot::median(series[0], 0, series[0].size()) > Boxplot::median(series[1], 0, series[1].size());
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

    vector< pair< pair<double, double>, pair<double, double> > > lines;
    vector< pair< pair<double, double>, pair<double, double> > > diagonal;
    diagonal.push_back({{0, 0}, {1, 1}});
    diagonalplot->add(diagonal, "");


    for (uint i = 0; i < ROC.first.size() - 1; ++i){
        lines.push_back({{ROC.first[i], ROC.second[i]}, {ROC.first[i + 1], ROC.second[i + 1]}});
        auc += (ROC.second[i] - fp) * ROC.first[i];
        fp = ROC.second[i];
    }
    lineplot->add(lines, "", QColor("#209fdf"));

    chart->add(lineplot);
    chart->add(diagonalplot);

    stat_results.push_back({"AUC", auc});
    chart->setXLabel("False positive Rate");
    chart->setYLabel("Sensitivity");
    chart->setTitle(QString("ROC Curve, AUC = %1 %").arg(QString::number(auc * 100., 'g', 4)));
}

























void Statistics::updateBoxPlot(){

    chart->clear();
    chart->setTitle("");
    chart->setVisible(true);

    series_titles.clear();
    series.clear();
    flat_data.clear();
    stat_results.clear();

    string target_variable = GlobalData::gui_string_var["study_var"];
    if (!lipid_space || uncontains_val(lipid_space->study_variable_values, target_variable) || !lipid_space->analysis_finished) return;

    bool is_nominal = lipid_space->study_variable_values[target_variable].study_variable_type == NominalStudyVariable;

    if (lipid_space->selected_lipidomes.size() <= 1 || GlobalData::stat_level != LipidomeLevel){
        chart->setVisible(false);
        return;
    }


    // setup array for target variable values, if nominal then each with incrementing number
    map<string, double> nominal_target_values;
    Indexes target_indexes;
    Array target_values;
    int nom_counter = 0;
    vector<string> nominal_values;
    vector<Lipidome*> selected_lipidomes_for_boxplot;

    string secondary_target_variable = GlobalData::gui_string_var["secondary_var"];
    SecondaryType secondary_type = NoSecondary;
    if (contains_val(lipid_space->study_variable_values, secondary_target_variable)){
        secondary_type = (lipid_space->study_variable_values[secondary_target_variable].study_variable_type == NominalStudyVariable) ? NominalSecondary : NumericalSecondary;
    }
    Array S;

    if (is_nominal){
        for (auto lipidome : lipid_space->selected_lipidomes){
            if (lipidome->study_variables[target_variable].missing) continue;
            if (secondary_type != NoSecondary && lipidome->study_variables[secondary_target_variable].missing) continue;

            string nominal_value = lipidome->study_variables[target_variable].nominal_value;
            selected_lipidomes_for_boxplot.push_back(lipidome);

            if (uncontains_val(nominal_target_values, nominal_value)){
                nominal_target_values.insert({nominal_value, nom_counter++});
                nominal_values.push_back(nominal_value);
                series_titles.push_back(nominal_value);
            }
            target_indexes.push_back(nominal_target_values[nominal_value]);
            if (secondary_type != NoSecondary) target_values.push_back(lipidome->study_variables[secondary_target_variable].numerical_value);


        }
    }
    else {
        for (auto lipidome : lipid_space->selected_lipidomes){
            if (lipidome->study_variables[target_variable].missing) continue;
            if (secondary_type != NoSecondary && lipidome->study_variables[secondary_target_variable].missing) continue;

            target_values.push_back(lipidome->study_variables[target_variable].numerical_value);
            if (secondary_type == NominalSecondary){
                string nominal_value = lipidome->study_variables[secondary_target_variable].nominal_value;
                if (uncontains_val(nominal_target_values, nominal_value)){
                    nominal_target_values.insert({nominal_value, nom_counter++});
                    nominal_values.push_back(nominal_value);
                }
                target_indexes.push_back(nominal_target_values[nominal_value]);
            }
            else if (secondary_type == NumericalSecondary){
                S.push_back(lipidome->study_variables[secondary_target_variable].numerical_value);
            }
        }
    }

    Matrix statistics_matrix(lipid_space->statistics_matrix);

    // if any lipidome has a missing study variable, discard the lipidome from the statistic
    Indexes lipidomes_to_keep;
    for (int r = 0; r < statistics_matrix.rows; ++r){
        if (!lipid_space->selected_lipidomes[r]->study_variables[target_variable].missing) lipidomes_to_keep.push_back(r);
    }
    Matrix tmp;
    tmp.rewrite(statistics_matrix, lipidomes_to_keep);
    statistics_matrix.rewrite(tmp);

    if (is_nominal){

        flat_data.insert({"Sample", vector<QVariant>()});
        flat_data.insert({"Category", vector<QVariant>()});
        flat_data.insert({"Value", vector<QVariant>()});

        series.resize(nom_counter);
        if (secondary_type != NoSecondary){
            for (uint r = 0; r < target_indexes.size(); ++r){
                series[target_indexes[r]].push_back(target_values[r]);
                flat_data["Sample"].push_back(QString(selected_lipidomes_for_boxplot[r]->cleaned_name.c_str()));
                flat_data["Category"].push_back(QString(nominal_values[target_indexes[r]].c_str()));
                flat_data["Value"].push_back(target_values[r]);
            }
        }
        else {
            if (statistics_matrix.cols > 1) statistics_matrix.scale();
            for (int r = 0; r < statistics_matrix.rows; ++r){
                double sum = 0;
                for (int c = 0; c < statistics_matrix.cols; c++){
                    double val = statistics_matrix(r, c);
                    if (!isnan(val) && !isinf(val)) sum += val;
                }
                if (statistics_matrix.cols > 1 || sum > 1e-15){
                    series[target_indexes[r]].push_back(sum);
                    flat_data["Sample"].push_back(QString(selected_lipidomes_for_boxplot[r]->cleaned_name.c_str()));
                    flat_data["Category"].push_back(QString(nominal_values[target_indexes[r]].c_str()));
                    flat_data["Value"].push_back(sum);
                }
            }
        }

        Boxplot* boxplot = new Boxplot(chart, show_data);
        for (uint i = 0; i < nominal_values.size(); ++i){
            Array &single_series = series[i];
            QString category = QString(nominal_values[i].c_str()) + QString(" (%1)").arg(series[i].size());
            string color_key = target_variable + "_" + nominal_values[i];
            QColor color = contains_val(GlobalData::colorMapStudyVariables, color_key) ? GlobalData::colorMapStudyVariables[color_key] : Qt::white;

            boxplot->add(single_series, category, color);
        }
        chart->add(boxplot);



        if (nom_counter == 2){
            double accuracy = compute_accuracy(series);
            stat_results.push_back({"accuracy", accuracy});
            double p_student = p_value_student(series[0], series[1]);
            double p_welch = p_value_welch(series[0], series[1]);
            double p_ks = p_value_kolmogorov_smirnov(series[0], series[1]);
            stat_results.push_back({"p_value(Student)", p_student});
            stat_results.push_back({"p_value(Welch)", p_welch});
            stat_results.push_back({"p_value(KS)", p_ks});
            chart->setTitle(QString("Statistics: <i>p</i>-value<sub>Student</sub> = %1,   <i>p</i>-value<sub>Welch</sub> = %2,   <i>p</i>-value<sub>KS</sub> = %3").arg(QString::number(p_student, 'g', 3)).arg(QString::number(p_welch, 'g', 3)).arg(QString::number(p_ks, 'g', 3)));
        }
        else if (nom_counter > 2){
            double accuracy = compute_accuracy(series);
            stat_results.push_back({"accuracy", accuracy});
            double p_anova = p_value_anova(series);
            stat_results.push_back({"p_value(ANOVA)", p_anova});
            if (p_anova >= 1e-3){
                chart->setTitle(QString("Statistics: <i>p</i>-value<sub>ANOVA</sub> = %2").arg(p_anova, 0, 'g', 4));
            }
            else {
                chart->setTitle(QString("Statistics: <i>p</i>-value<sub>ANOVA</sub> = %2").arg(QString::number(p_anova, 'e', -1)));
            }
        }

        chart->setYLabel("Arbitrary units [A.U.]");
    }
    else {

        Array coefficiants;
        if (secondary_type != NumericalSecondary){
            Array constants(statistics_matrix.rows, 1);
            statistics_matrix.add_column(constants);
            coefficiants.compute_coefficiants(statistics_matrix, target_values);    // estimating coefficiants
            S.mult(statistics_matrix, coefficiants);
        }

        flat_data.insert({"Sample", vector<QVariant>()});
        flat_data.insert({"Value measured", vector<QVariant>()});
        flat_data.insert({"Value model", vector<QVariant>()});

        Scatterplot *scatterplot = new Scatterplot(chart);
        if (secondary_type == NoSecondary){

            series_titles.push_back(target_variable + " (model)");
            series_titles.push_back(target_variable + " (measured)");
            series.resize(2);

            vector< pair<double, double> > data;
            for (uint i = 0; i < S.size(); ++i){
                data.push_back({S[i], target_values[i]});
                series[0].push_back(S[i]);
                series[1].push_back(target_values[i]);
            }
            scatterplot->add(data, target_variable.c_str());

        }
        else {
            if (secondary_type == NominalSecondary){
                vector< vector< pair<double, double> > > data(nominal_values.size());
                series.resize(S.size() * 2);

                for (uint i = 0; i < S.size(); ++i){
                    data[target_indexes[i]].push_back({S[i], target_values[i]});
                    series[target_indexes[i] * 2].push_back(S[i]);
                    series[target_indexes[i] * 2 + 1].push_back(target_values[i]);
                }

                for (auto nominal_value : nominal_values){
                    series_titles.push_back(secondary_target_variable + " / " + nominal_value + " (model)");
                    series_titles.push_back(secondary_target_variable + " / " + nominal_value + " (measured)");

                    string color_key = secondary_target_variable + "_" + nominal_value;
                    QColor c = contains_val(GlobalData::colorMapStudyVariables, color_key) ? GlobalData::colorMapStudyVariables[color_key] : QColor("#209fdf");
                    scatterplot->add(data[nominal_target_values[nominal_value]], nominal_value.c_str(), c);
                }
            }
            else {
                vector< pair<double, double> > data;
                series.resize(2);
                for (uint i = 0; i < S.size(); ++i){
                    data.push_back({S[i], target_values[i]});
                    series[0].push_back(S[i]);
                    series[1].push_back(target_values[i]);
                }
                series_titles.push_back(secondary_target_variable);
                series_titles.push_back(target_variable);
                scatterplot->add(data, QString("%1 / %2").arg(secondary_target_variable.c_str()).arg(target_variable.c_str()), QColor("#209fdf"));
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
            slope_num += (S[r] - mx) * (target_values[r] - my);
            slope_denom += sq(S[r] - mx);
        }
        double slope = slope_num / slope_denom;
        double intercept = my - slope * mx;

        double SQR = 0, SQT = 0;
        for (uint r = 0; r < S.size(); ++r){
            SQR += sq(target_values[r] - (slope * S[r] + intercept));
            SQT += sq(target_values[r] - my);
        }
        double R2 = 1. - SQR / SQT;
        stat_results.push_back({"R^2", R2});
        // store the weights of the linear regression for export
        if (secondary_type != NumericalSecondary && coefficiants.size() == lipid_space->statistics_lipids.size() + 1){
            for (uint i = 0; i < lipid_space->statistics_lipids.size(); ++i){
                stat_results.push_back({QString("weight %1").arg(lipid_space->statistics_lipids[i].c_str()).toStdString(), coefficiants[i]});
            }
            stat_results.push_back({"weight constant", coefficiants[coefficiants.size() - 1]});
        }

        // draw the regression line as a function
        FunctionPlot *regression_function = new FunctionPlot(chart);
        QString rg_name = QString().asprintf("Regression model (R<sup>2</sup> = %0.3f)", R2);
        regression_function->add([](double x, vector<double> par){ return par[0] * x + par[1];}, {slope, intercept}, rg_name, QColor("#99ca53"));
        chart->add(regression_function);

        QString sign = intercept >= 0 ? "+" : "-";
        chart->setTitle(QString("Linear regression model"));
    }
}
























void Statistics::updatePCA(){

    chart->clear();
    chart->setTitle("");
    chart->setVisible(true);

    series_titles.clear();
    series.clear();
    flat_data.clear();
    stat_results.clear();

    string target_variable = GlobalData::gui_string_var["study_var"];
    if (!lipid_space || uncontains_val(lipid_space->study_variable_values, target_variable) || !lipid_space->analysis_finished) return;

    bool is_nominal = lipid_space->study_variable_values[target_variable].study_variable_type == NominalStudyVariable;

    if (lipid_space->selected_lipidomes.size() <= 1 || lipid_space->statistics_matrix.cols < lipid_space->cols_for_pca || lipid_space->study_variable_values[target_variable].study_variable_type != NominalStudyVariable || GlobalData::stat_level != LipidomeLevel){
        chart->setVisible(false);
        return;
    }


    // setup array for target variable values, if nominal then each with incrementing number
    map<string, double> nominal_target_values;
    Indexes target_indexes;
    int nom_counter = 0;
    vector<string> nominal_values;

    string secondary_target_variable = GlobalData::gui_string_var["secondary_var"];
    SecondaryType secondary_type = NoSecondary;
    if (contains_val(lipid_space->study_variable_values, secondary_target_variable)){
        secondary_type = (lipid_space->study_variable_values[secondary_target_variable].study_variable_type == NominalStudyVariable) ? NominalSecondary : NumericalSecondary;
    }

    if (is_nominal){
        for (auto lipidome : lipid_space->selected_lipidomes){
            if (lipidome->study_variables[target_variable].missing) continue;
            if (secondary_type != NoSecondary && lipidome->study_variables[secondary_target_variable].missing) continue;

            string nominal_value = lipidome->study_variables[target_variable].nominal_value;

            if (uncontains_val(nominal_target_values, nominal_value)){
                nominal_target_values.insert({nominal_value, nom_counter++});
                nominal_values.push_back(nominal_value);
            }
            target_indexes.push_back(nominal_target_values[nominal_value]);


        }
    }
    else {
        for (auto lipidome : lipid_space->selected_lipidomes){
            if (lipidome->study_variables[target_variable].missing) continue;
            if (secondary_type != NoSecondary && lipidome->study_variables[secondary_target_variable].missing) continue;

            if (secondary_type == NominalSecondary){
                string nominal_value = lipidome->study_variables[secondary_target_variable].nominal_value;
                if (uncontains_val(nominal_target_values, nominal_value)){
                    nominal_target_values.insert({nominal_value, nom_counter++});
                    nominal_values.push_back(nominal_value);
                }
                target_indexes.push_back(nominal_target_values[nominal_value]);
            }
        }
    }

    Matrix statistics_matrix(lipid_space->statistics_matrix);

    // if any lipidome has a missing study variable, discard the lipidome from the statistic
    Indexes lipidomes_to_keep;
    for (int r = 0; r < statistics_matrix.rows; ++r){
        if (!lipid_space->selected_lipidomes[r]->study_variables[target_variable].missing) lipidomes_to_keep.push_back(r);
    }
    Matrix tmp;
    tmp.rewrite(statistics_matrix, lipidomes_to_keep);
    statistics_matrix.rewrite(tmp);
    statistics_matrix.scale();


    Matrix pca;
    lipid_space->statistics_matrix.PCA(pca, lipid_space->cols_for_pca);


    Scatterplot *scatterplot = new Scatterplot(chart);
    if (is_nominal && pca.rows == (int)target_indexes.size()){
        vector< vector< pair<double, double> > > all_data(nominal_values.size());
        vector< vector<QString> > data_labels(nominal_values.size());

        for (int i = 0; i < pca.rows; ++i){
            all_data[target_indexes[i]].push_back({pca(i, GlobalData::PC1), pca(i, GlobalData::PC2)});
            data_labels[target_indexes[i]].push_back(lipid_space->selected_lipidomes[i]->cleaned_name.c_str());
        }

        vector< pair<string, int> > sorted_nominal_values;
        for (uint i = 0; i < nominal_values.size(); ++i) sorted_nominal_values.push_back({nominal_values[i], i});
        sort(sorted_nominal_values.begin(), sorted_nominal_values.end(), [](const pair<string, int> &a, const pair<string, int> &b) -> bool {
            return a < b;
        });


        series.resize(sorted_nominal_values.size() * 2);
        for (uint i = 0; i < sorted_nominal_values.size(); ++i){
            series_titles.push_back(sorted_nominal_values[i].first + " - PC" + std::to_string(GlobalData::PC1 + 1));
            series_titles.push_back(sorted_nominal_values[i].first + " - PC" + std::to_string(GlobalData::PC1 + 2));


            string color_key = target_variable + "_" + sorted_nominal_values[i].first;
            QColor color = contains_val(GlobalData::colorMapStudyVariables, color_key) ? GlobalData::colorMapStudyVariables[color_key] : QColor("#209fdf");
            QString group_label = QStringLiteral("%1 (%2)").arg(sorted_nominal_values[i].first.c_str()).arg(all_data[sorted_nominal_values[i].second].size());
            scatterplot->add(all_data[sorted_nominal_values[i].second], group_label, color, &data_labels[sorted_nominal_values[i].second]);

            for (auto &p : all_data[sorted_nominal_values[i].second]){
                series[i * 2].push_back(p.first);
                series[i * 2 + 1].push_back(p.second);
            }
        }
    }
    else {
        vector< pair<double, double> > data;
        vector<QString> data_labels;
        for (int i = 0; i < pca.rows; ++i){
            data.push_back({pca(i, 0), pca(i, 1)});
            data_labels.push_back(lipid_space->selected_lipidomes[i]->cleaned_name.c_str());
        }
        QString group_label = QStringLiteral("Global (%1)").arg(data.size());
        scatterplot->add(data, group_label, QColor("#209fdf"), &data_labels);
    }
    chart->add(scatterplot);

    Array vars;
    LipidSpace::compute_PCA_variances(pca, vars);
    chart->setXLabel(QStringLiteral("Principal component %1: %2%").arg(GlobalData::PC1 + 1).arg(vars[GlobalData::PC1] * 100., 0, 'G', 3));
    chart->setYLabel(QStringLiteral("Principal component %1: %2%").arg(GlobalData::PC2 + 1).arg(vars[GlobalData::PC2] * 100., 0, 'G', 3));
    chart->setTitle("Principal Component Analysis");


}





















void Statistics::updatePVal(){


    chart->clear();
    chart->setTitle("");
    chart->setVisible(true);

    series_titles.clear();
    series.clear();
    flat_data.clear();
    stat_results.clear();
    volcano_data.clear();
    volcano_data.insert({"down", vector<string>()});
    volcano_data.insert({"non", vector<string>()});
    volcano_data.insert({"up", vector<string>()});

    string target_variable = GlobalData::gui_string_var["study_var"];
    if (!lipid_space || uncontains_val(lipid_space->study_variable_values, target_variable) || !lipid_space->analysis_finished) return;

    if (lipid_space->study_variable_values[target_variable].study_variable_type != NominalStudyVariable) return;




    // setup array for target variable values, if nominal then each with incrementing number
    set<string> nominal_values_list;
    map<string, int> nominal_value_positions;
    map<string, string> lipidome_to_nominal_value;

    for (auto lipidome : lipid_space->selected_lipidomes){
        if (lipidome->study_variables[target_variable].missing) continue;

        string nominal_value = lipidome->study_variables[target_variable].nominal_value;

        if (GlobalData::condition_mode_enrichment == StandardMode){
            if (uncontains_val(nominal_value_positions, nominal_value)) nominal_value_positions.insert({nominal_value, nominal_value_positions.size()});
            lipidome_to_nominal_value.insert({lipidome->cleaned_name, nominal_value});
        }
        else {
            lipidome_to_nominal_value.insert({lipidome->cleaned_name, nominal_value});
        }
    }

    if (GlobalData::condition_mode_enrichment == SelectionMode){
        if (GlobalData::first_enrichment_classes.size() == 0 || GlobalData::second_enrichment_classes.size() == 0) return;

        for (auto condition : GlobalData::first_enrichment_classes) nominal_values_list.insert(condition);
        for (auto condition : GlobalData::second_enrichment_classes) nominal_values_list.insert(condition);
        if (nominal_values_list.size() < 2) return;

    }
    else if (nominal_value_positions.size() < 2) return;


    Matrix stat_matrix;
    series.push_back(Array());
    map<LipidAdduct*, int> lipid_map;
    map<string, int> lipid_name_map;
    map<string, int> lipidome_name_map;
    map<string, string> &translations = lipid_space->lipid_name_translations[(int)GlobalData::gui_num_var["translate"]];
    // setting up lipid to column in matrix map
    for (uint i = 0; i < lipid_space->global_lipidome->lipids.size(); ++i){
        LipidAdduct* lipid = lipid_space->global_lipidome->lipids[i];
        if (uncontains_val(lipid_map, lipid)){
            lipid_map.insert({lipid, lipid_map.size()});
            string lipid_name = lipid_space->global_lipidome->species[i];
            if (contains_val(translations, lipid_name)) lipid_name = translations[lipid_name];
            lipid_name_map.insert({lipid_name, lipid_name_map.size()});
        }
    }

    stat_matrix.reset(lipid_space->selected_lipidomes.size(), lipid_map.size());
    stat_matrix += NAN;
    for (uint r = 0; r < lipid_space->selected_lipidomes.size(); ++r){
        Lipidome* lipidome = lipid_space->selected_lipidomes[r];
        lipidome_name_map.insert({lipidome->cleaned_name, r});
        for (uint i = 0; i < lipidome->lipids.size(); ++i){
            if (contains_val(lipid_map, lipidome->lipids[i])){
                stat_matrix(r, lipid_map[lipidome->lipids[i]]) = lipidome->normalized_intensities[i];
            }
        }
    }

    flat_data.insert({"Lipid species", vector<QVariant>()});
    flat_data.insert({"Lipidome", vector<QVariant>()});
    flat_data.insert({"Condition", vector<QVariant>()});
    flat_data.insert({"Quantity", vector<QVariant>()});
    for (auto &kv : lipid_name_map){
        string lipid_name = kv.first;
        int lipid_col = kv.second;
        double p_value = 0;
        vector<Array> arrays(nominal_value_positions.size(), Array());
        Array first_array;
        Array second_array;

        for (auto &kv_val : lipidome_to_nominal_value){
            string lipidome_name = kv_val.first;
            string nom_value = kv_val.second;
            if (GlobalData::condition_mode_enrichment == SelectionMode && uncontains_val(nominal_value_positions, nom_value)) continue;

            int nominal_position = (GlobalData::condition_mode_enrichment == StandardMode) ? nominal_value_positions[nom_value] : -1;

            if (GlobalData::condition_mode_enrichment == SelectionMode && uncontains_val(GlobalData::first_enrichment_classes, nom_value) && uncontains_val(GlobalData::second_enrichment_classes, nom_value)) continue;

            Array &fill_array = GlobalData::condition_mode_enrichment == SelectionMode ? (contains_val(GlobalData::first_enrichment_classes, nom_value) ? first_array : second_array) : arrays[nominal_position];
            if (uncontains_val(lipidome_name_map, lipidome_name)) continue;
            int lipidome_row = lipidome_name_map[lipidome_name];

            double stat_value = stat_matrix(lipidome_row, lipid_col);
            if (!isnan(stat_value)){
                flat_data["Lipid species"].push_back(lipid_name.c_str());
                flat_data["Lipidome"].push_back(lipidome_name.c_str());
                flat_data["Condition"].push_back(nom_value.c_str());
                flat_data["Quantity"].push_back(stat_value);
                fill_array.push_back(stat_value);
            }
        }

        if (GlobalData::condition_mode_enrichment == SelectionMode){
            if (first_array.size() < 2 || second_array.size() < 2) continue;

            if (GlobalData::enrichment_test == "student") p_value = p_value_student(first_array, second_array);
            else if (GlobalData::enrichment_test == "welch") p_value = p_value_welch(first_array, second_array);
            else if (GlobalData::enrichment_test == "ks") p_value = p_value_kolmogorov_smirnov(first_array, second_array);
            else continue;

        }
        else {
            for (int i = arrays.size() - 1; i >= 0; --i){
                if (arrays[i].size() < 2) arrays.erase(arrays.begin() + i);
            }
            if (nominal_value_positions.size() == 2){
                if (GlobalData::enrichment_test == "student") p_value = p_value_student(arrays[0], arrays[1]);
                else if (GlobalData::enrichment_test == "welch") p_value = p_value_welch(arrays[0], arrays[1]);
                else if (GlobalData::enrichment_test == "ks") p_value = p_value_kolmogorov_smirnov(arrays[0], arrays[1]);
            }
            else if (nominal_value_positions.size() > 2) {
                p_value = p_value_anova(arrays);
            }
            else continue;
        }

        series.back().push_back(p_value);
    }
    if (series.back().empty()) return;

    series_titles.push_back("p-values");

    Histogramplot* histogramplot = new Histogramplot(chart);
    vector<QString> categories{"p-values"};
    vector<QColor> colors{"#85b3ce"};

    double num_bars = contains_val(GlobalData::gui_num_var, "bar_number") ? GlobalData::gui_num_var["bar_number"] : 20;
    histogramplot->add(series, categories, &colors, num_bars, 0, 1);
    histogramplot->borders.setX(0);
    histogramplot->borders.setY(1);
    chart->add(histogramplot);
    chart->yrange.setX(0);
    chart->float_x_precision = 2;
    chart->update_chart();
    chart->setTitle("p-value distribution");
}






















void Statistics::updateVolcano(){

    chart->clear();
    chart->setTitle("");
    chart->setVisible(true);

    series_titles.clear();
    series.clear();
    flat_data.clear();
    stat_results.clear();
    volcano_data.clear();

    string target_variable = GlobalData::gui_string_var["study_var"];
    if (!lipid_space || uncontains_val(lipid_space->study_variable_values, target_variable) || !lipid_space->analysis_finished) return;

    if (lipid_space->study_variable_values[target_variable].study_variable_type != NominalStudyVariable) return;

    // setup array for target variable values, if nominal then each with incrementing number
    map<string, string> lipidome_to_nominal_value;
    set<string> nominal_values_list;
    map<string, int> nominal_value_positions;
    vector<string> nominal_values;
    string first_conditions = "";
    string second_conditions = "";

    for (auto lipidome : lipid_space->selected_lipidomes){
        if (lipidome->study_variables[target_variable].missing) continue;

        string nominal_value = lipidome->study_variables[target_variable].nominal_value;

        if (GlobalData::condition_mode_enrichment == StandardMode){
            if (uncontains_val(nominal_value_positions, nominal_value)){
                nominal_value_positions.insert({nominal_value, nominal_value_positions.size()});
                nominal_values.push_back(nominal_value);
            }
            lipidome_to_nominal_value.insert({lipidome->cleaned_name, nominal_value});
        }
        else {
            lipidome_to_nominal_value.insert({lipidome->cleaned_name, nominal_value});
        }
    }


    if (GlobalData::condition_mode_enrichment == SelectionMode){
        for (auto condition : GlobalData::first_enrichment_classes){
            nominal_values_list.insert(condition);
            if (first_conditions.length()) first_conditions += ", ";
            first_conditions += condition;
        }
        for (auto condition : GlobalData::second_enrichment_classes){
            nominal_values_list.insert(condition);
            if (second_conditions.length()) second_conditions += ", ";
            second_conditions += condition;
        }

        if (nominal_values_list.size() < 2) return;

        volcano_data.insert({"down", vector<string>()});
        volcano_data.insert({"non", vector<string>()});
        volcano_data.insert({"up", vector<string>()});

    }
    else {
        if (nominal_value_positions.size() < 2) return;
        if (nominal_value_positions.size() == 2){
            volcano_data.insert({"down", vector<string>()});
            volcano_data.insert({"non", vector<string>()});
            volcano_data.insert({"up", vector<string>()});
        }
        else {
            volcano_data.insert({"non", vector<string>()});
            volcano_data.insert({"reg", vector<string>()});
        }
    }




    Matrix stat_matrix;
    map<LipidAdduct*, int> lipid_map;
    map<string, int> lipid_name_map;
    map<string, int> lipidome_name_map;
    map<string, string> &translations = lipid_space->lipid_name_translations[(int)GlobalData::gui_num_var["translate"]];
    // setting up lipid to column in matrix map
    for (uint i = 0; i < lipid_space->global_lipidome->lipids.size(); ++i){
        LipidAdduct* lipid = lipid_space->global_lipidome->lipids[i];
        if (uncontains_val(lipid_map, lipid)){
            lipid_map.insert({lipid, lipid_map.size()});
            string lipid_name = lipid_space->global_lipidome->species[i];
            if (contains_val(translations, lipid_name)) lipid_name = translations[lipid_name];
            lipid_name_map.insert({lipid_name, lipid_name_map.size()});
        }
    }

    stat_matrix.reset(lipid_space->selected_lipidomes.size(), lipid_map.size());
    stat_matrix += NAN;
    for (uint r = 0; r < lipid_space->selected_lipidomes.size(); ++r){
        Lipidome* lipidome = lipid_space->selected_lipidomes[r];
        lipidome_name_map.insert({lipidome->cleaned_name, r});
        for (uint i = 0; i < lipidome->lipids.size(); ++i){
            if (contains_val(lipid_map, lipidome->lipids[i])){
                stat_matrix(r, lipid_map[lipidome->lipids[i]]) = lipidome->normalized_intensities[i];
            }
        }
    }

    double max_x = 0;
    double min_x = 0;
    Array p_values;
    Array fold_changes;
    vector<QString> p_val_lipids;
    flat_data.insert({"Lipid species", vector<QVariant>()});
    flat_data.insert({"Lipidome", vector<QVariant>()});
    flat_data.insert({"Condition", vector<QVariant>()});
    flat_data.insert({"Quantity", vector<QVariant>()});
    for (auto &kv : lipid_name_map){
        string lipid_name = kv.first;
        int lipid_col = kv.second;
        vector<Array> arrays(nominal_value_positions.size(), Array());
        Array first_array;
        Array second_array;


        for (auto &kv_val : lipidome_to_nominal_value){
            string lipidome_name = kv_val.first;
            string nom_value = kv_val.second;
            if (GlobalData::condition_mode_enrichment == SelectionMode && uncontains_val(nominal_value_positions, nom_value)) continue;

            int nominal_position = (GlobalData::condition_mode_enrichment == StandardMode) ? nominal_value_positions[nom_value] : -1;

            if (GlobalData::condition_mode_enrichment == SelectionMode && uncontains_val(GlobalData::first_enrichment_classes, nom_value) && uncontains_val(GlobalData::second_enrichment_classes, nom_value)) continue;

            Array &fill_array = GlobalData::condition_mode_enrichment == SelectionMode ? (contains_val(GlobalData::first_enrichment_classes, nom_value) ? first_array : second_array) : arrays[nominal_position];
            if (uncontains_val(lipidome_name_map, lipidome_name)) continue;
            int lipidome_row = lipidome_name_map[lipidome_name];

            double stat_value = stat_matrix(lipidome_row, lipid_col);
            if (!isnan(stat_value)){
                flat_data["Lipid species"].push_back(lipid_name.c_str());
                flat_data["Lipidome"].push_back(lipidome_name.c_str());
                flat_data["Condition"].push_back(nom_value.c_str());
                flat_data["Quantity"].push_back(stat_value);
                fill_array.push_back(stat_value);
            }
        }

        double fc = -1;
        double p_value = -1;
        if (GlobalData::condition_mode_enrichment == SelectionMode){
            if (first_array.size() < 2 || second_array.size() < 2) continue;

            if (GlobalData::enrichment_test == "student") p_value = p_value_student(first_array, second_array);
            else if (GlobalData::enrichment_test == "welch") p_value = p_value_welch(first_array, second_array);
            else if (GlobalData::enrichment_test == "ks") p_value = p_value_kolmogorov_smirnov(first_array, second_array);
            else continue;

            fc = second_array.mean() / first_array.mean();

        }
        else {

            for (int i = arrays.size() - 1; i >= 0; --i){
                if (arrays[i].size() < 2) arrays.erase(arrays.begin() + i);
            }
            if (nominal_value_positions.size() == 2){
                if (GlobalData::enrichment_test == "student") p_value = p_value_student(arrays[0], arrays[1]);
                else if (GlobalData::enrichment_test == "welch") p_value = p_value_welch(arrays[0], arrays[1]);
                else if (GlobalData::enrichment_test == "ks") p_value = p_value_kolmogorov_smirnov(arrays[0], arrays[1]);
            }
            else if (nominal_value_positions.size() > 2) {
                p_value = p_value_anova(arrays);
            }
            else continue;

            if (nominal_value_positions.size() == 2){
                fc = arrays[1].mean() / arrays[0].mean();
            }
            else {
                Array cv_array;
                for (auto &array : arrays){
                    for (auto val : array) cv_array.push_back(val);
                }
                fc = cv_array.stdev() / cv_array.mean();
            }
        }

        if (fc <= 0 || p_value <= 1e-50) continue;

        p_values.push_back(p_value);
        fold_changes.push_back(fc);
        p_val_lipids.push_back(lipid_name.c_str());
    }

    if (GlobalData::enrichment_correction == "bh") multiple_correction_bh(p_values);
    else if (GlobalData::enrichment_correction == "bonferoni") multiple_correction_bonferoni(p_values);


    vector<pair<double, double>> pval_fc_down;
    vector<QString> fc_down_lipids;
    vector<pair<double, double>> pval_fc_non;
    vector<QString> fc_non_lipids;
    vector<pair<double, double>> pval_fc_up;
    vector<QString> fc_up_lipids;
    vector<pair<double, double>> pval_fc_reg;
    vector<QString> fc_reg_lipids;

    double alpha_level = GlobalData::enrichment_sig;
    double log_fc_level = GlobalData::enrichment_log_fc;


    if (nominal_value_positions.size() == 2){
        for (int i = 0; i < 6; ++i) series.push_back(Array());
        series_titles.push_back("Down regulated - p-value");
        series_titles.push_back("Down regulated - fc");
        series_titles.push_back("Not regulated - p-value");
        series_titles.push_back("Not regulated - fc");
        series_titles.push_back("Up regulated - p-value");
        series_titles.push_back("Up regulated - fc");


        for (int i = 0; i < (int)p_values.size(); ++i){
            double p_value = p_values[i];
            double fc = fold_changes[i];

            double log_p_value = -log10(p_value);
            double log_fc = log2(fc);

            if (alpha_level < p_value){
                pval_fc_non.push_back({log_fc, log_p_value});
                fc_non_lipids.push_back(p_val_lipids[i]);

                series[2].push_back(p_value);
                series[3].push_back(fc);
            }
            else {
                if (log_fc <= -log_fc_level){
                    pval_fc_down.push_back({log_fc, log_p_value});
                    fc_down_lipids.push_back(p_val_lipids[i]);

                    series[0].push_back(p_value);
                    series[1].push_back(fc);
                }

                else if (log_fc_level <= log_fc){
                    pval_fc_up.push_back({log_fc, log_p_value});
                    fc_up_lipids.push_back(p_val_lipids[i]);

                    series[4].push_back(p_value);
                    series[5].push_back(fc);
                }

                else {
                    pval_fc_non.push_back({log_fc, log_p_value});
                    fc_non_lipids.push_back(p_val_lipids[i]);

                    series[2].push_back(p_value);
                    series[3].push_back(fc);
                }
            }
            max_x = max(max_x, fabs(log_fc));
        }
        min_x = -max_x;
    }
    else {
        for (int i = 0; i < 4; ++i) series.push_back(Array());
        series_titles.push_back("Not regulated - p-value");
        series_titles.push_back("Not regulated - fc");
        series_titles.push_back("Regulated - p-value");
        series_titles.push_back("Regulated - fc");

        log_fc_level = (pow(2, log_fc_level) - 1.) / (pow(2, log_fc_level) + 1);

        for (int i = 0; i < (int)p_values.size(); ++i){
            double p_value = p_values[i];
            double fc = fold_changes[i];

            double log_p_value = -log10(p_value);
            double log_fc = fc;

            if (alpha_level < p_value){
                pval_fc_non.push_back({log_fc, log_p_value});
                fc_non_lipids.push_back(p_val_lipids[i]);

                series[0].push_back(p_value);
                series[1].push_back(fc);
            }
            else {
                if (log_fc_level <= log_fc){
                    pval_fc_reg.push_back({log_fc, log_p_value});
                    fc_reg_lipids.push_back(p_val_lipids[i]);

                    series[2].push_back(p_value);
                    series[3].push_back(fc);
                }

                else {
                    pval_fc_non.push_back({log_fc, log_p_value});
                    fc_non_lipids.push_back(p_val_lipids[i]);

                    series[0].push_back(p_value);
                    series[1].push_back(fc);
                }
            }
            max_x = max(max_x, fabs(log_fc));
        }
    }

    Scatterplot* scatterplot = 0;
    Lineplot *line_plot = 0;
    vector< pair< pair<double, double>, pair<double, double> > > boundaries;
    if (GlobalData::condition_mode_enrichment == SelectionMode){

        string color_key = target_variable + "_" + *GlobalData::first_enrichment_classes.begin();
        QColor color_down = contains_val(GlobalData::colorMapStudyVariables, color_key) ? GlobalData::colorMapStudyVariables[color_key] : QColor("#209fdf");
        color_key = target_variable + "_" + *GlobalData::second_enrichment_classes.begin();
        QColor color_up = contains_val(GlobalData::colorMapStudyVariables, color_key) ? GlobalData::colorMapStudyVariables[color_key] : QColor("#209fdf");

        scatterplot = new Scatterplot(chart, true);
        scatterplot->add(pval_fc_non, QString("Not regulated (%1)").arg(pval_fc_non.size()), "#dddddd", &fc_non_lipids);
        scatterplot->add(pval_fc_down, QString("Sig. up regulated %1 (%2)").arg(first_conditions.c_str()).arg(pval_fc_down.size()), color_down, &fc_down_lipids);
        scatterplot->add(pval_fc_up, QString("Sig. up regulated %1 (%2)").arg(second_conditions.c_str()).arg(pval_fc_up.size()), color_up, &fc_up_lipids);
        chart->add(scatterplot);

        for (auto lipid_name : fc_down_lipids) volcano_data["down"].push_back(lipid_name.toStdString());
        for (auto lipid_name : fc_non_lipids) volcano_data["non"].push_back(lipid_name.toStdString());
        for (auto lipid_name : fc_up_lipids) volcano_data["up"].push_back(lipid_name.toStdString());

        line_plot = new Lineplot(chart, true);
        boundaries.push_back({{-1e5, -log10(alpha_level)}, {1e5, -log10(alpha_level)}});
        boundaries.push_back({{-log_fc_level, -1e5}, {-log_fc_level, 1e5}});
        boundaries.push_back({{log_fc_level, -1e5}, {log_fc_level, 1e5}});
    }

    else {

        if (nominal_value_positions.size() == 2){
            string color_key = target_variable + "_" + nominal_values[0];
            QColor color_down = contains_val(GlobalData::colorMapStudyVariables, color_key) ? GlobalData::colorMapStudyVariables[color_key] : QColor("#209fdf");
            color_key = target_variable + "_" + nominal_values[1];
            QColor color_up = contains_val(GlobalData::colorMapStudyVariables, color_key) ? GlobalData::colorMapStudyVariables[color_key] : QColor("#209fdf");

            scatterplot = new Scatterplot(chart, true);
            scatterplot->add(pval_fc_non, QString("Not regulated (%1)").arg(pval_fc_non.size()), "#dddddd", &fc_non_lipids);
            scatterplot->add(pval_fc_down, QString("Sig. up regulated %1 (%2)").arg(nominal_values[0].c_str()).arg(pval_fc_down.size()), color_down, &fc_down_lipids);
            scatterplot->add(pval_fc_up, QString("Sig. up regulated %1 (%2)").arg(nominal_values[0].c_str()).arg(pval_fc_up.size()), color_up, &fc_up_lipids);
            chart->add(scatterplot);

            for (auto lipid_name : fc_down_lipids) volcano_data["down"].push_back(lipid_name.toStdString());
            for (auto lipid_name : fc_non_lipids) volcano_data["non"].push_back(lipid_name.toStdString());
            for (auto lipid_name : fc_up_lipids) volcano_data["up"].push_back(lipid_name.toStdString());

            line_plot = new Lineplot(chart, true);
            boundaries.push_back({{-1e5, -log10(alpha_level)}, {1e5, -log10(alpha_level)}});
            boundaries.push_back({{-log_fc_level, -1e5}, {-log_fc_level, 1e5}});
            boundaries.push_back({{log_fc_level, -1e5}, {log_fc_level, 1e5}});
        }
        else {
            string color_key = target_variable + "_" + nominal_values[0];
            QColor color_reg = contains_val(GlobalData::colorMapStudyVariables, color_key) ? GlobalData::colorMapStudyVariables[color_key] : QColor("#209fdf");

            scatterplot = new Scatterplot(chart, true);
            scatterplot->add(pval_fc_non, QString("Not regulated (%1)").arg(pval_fc_non.size()), "#dddddd", &fc_non_lipids);
            scatterplot->add(pval_fc_reg, QString("Sig. regulated (%1)").arg(pval_fc_reg.size()), color_reg, &fc_reg_lipids);
            chart->add(scatterplot);

            for (auto lipid_name : fc_non_lipids) volcano_data["non"].push_back(lipid_name.toStdString());
            for (auto lipid_name : fc_reg_lipids) volcano_data["reg"].push_back(lipid_name.toStdString());

            line_plot = new Lineplot(chart, true);
            boundaries.push_back({{-1e5, -log10(alpha_level)}, {1e5, -log10(alpha_level)}});
            boundaries.push_back({{log_fc_level, -1e5}, {log_fc_level, 1e5}});
        }

    }



    line_plot->add(boundaries, "", "#ff0000");
    chart->add(line_plot);

    chart->xrange.setX(min_x * 1.05);
    chart->xrange.setY(max_x * 1.05);
    chart->yrange.setX(-0.1);
    chart->setTitle("Volcano plot");


    connect(scatterplot, &Scatterplot::enterLipid, this, &Statistics::lipidEntered);
    connect(scatterplot, &Scatterplot::exitLipid, this, &Statistics::lipidExited);
    connect(scatterplot, &Scatterplot::markLipid, this, &Statistics::lipidMarked);
    connect(scatterplot, &Scatterplot::markLipids, this, &Statistics::lipidsMarked);
}

