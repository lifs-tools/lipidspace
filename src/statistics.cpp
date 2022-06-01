#include "lipidspace/statistics.h"




Statistics::Statistics(QWidget *parent) : QChartView(parent) {
    lipid_space = 0;
    chart = new QChart();
    chart->setAnimationOptions(QChart::SeriesAnimations);
	setRenderHint(QPainter::Antialiasing);
    chart->createDefaultAxes();
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);
    setChart(chart);
    chart->legend()->setFont(QFont("Helvetica", GlobalData::gui_num_var["legend_size"]));
    chart->setTitleFont(QFont("Helvetica", GlobalData::gui_num_var["legend_size"]));
    
    chart->layout()->setContentsMargins(0,0,0,0);
    chart->setBackgroundRoundness(0);
}


void Statistics::set_lipid_space(LipidSpace *_lipid_space){
    lipid_space = _lipid_space;
}



void Statistics::setLegendSizeBoxPlot(int font_size){
    GlobalData::gui_num_var["legend_size"] = font_size;
    chart->legend()->setFont(QFont("Helvetica", font_size));
    chart->setTitleFont(QFont("Helvetica", font_size));
    updateBoxPlot();
}



void Statistics::setLegendSizeBarPlot(int font_size){
    GlobalData::gui_num_var["legend_size"] = font_size;
    chart->legend()->setFont(QFont("Helvetica", font_size));
    chart->setTitleFont(QFont("Helvetica", font_size));
    updateBarPlot();
}



void Statistics::setLegendSizeHistogram(int font_size){
    GlobalData::gui_num_var["legend_size"] = font_size;
    chart->legend()->setFont(QFont("Helvetica", font_size));
    chart->setTitleFont(QFont("Helvetica", font_size));
    updateHistogram();
}



void Statistics::setTickSizeBoxPlot(int font_size){
    GlobalData::gui_num_var["tick_size"] = font_size;
    updateBoxPlot();
}



void Statistics::setTickSizeBarPlot(int font_size){
    GlobalData::gui_num_var["tick_size"] = font_size;
    updateBarPlot();
}



void Statistics::setTickSizeHistogram(int font_size){
    GlobalData::gui_num_var["tick_size"] = font_size;
    updateHistogram();
}



void Statistics::setBarNumber(int bar_number){
    GlobalData::gui_num_var["bar_number"] = bar_number;
    updateHistogram();
}






void Statistics::exportData(){
    if (chart->series().size() == 0) return;
    QString file_name = QFileDialog::getSaveFileName(this, "Export as csv", GlobalData::last_folder, "Worksheet *.xlsx (*.xlsx);;Data Table *.csv (*.csv);;Data Table *.tsv (*.tsv)");
    if (!file_name.length()) return;

    try {
        if (QFile::exists(file_name)){
            cout << "deleting" << endl;
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
            QMessageBox::information(this, "Export error", "Unknown export format for file '" + file_name + "'.");
            return;
        }
        QMessageBox::information(this, "Export completed", "The export is completed into the file '" + file_name + "'.");
    }
    catch(exception &){
        QMessageBox::critical(this, "Error during export", "An error occurred during the export into the file '" + file_name + "'. Is your hard disk drive full by chance or do you have enough permissions to write files into this folder?");
    }
}



void Statistics::exportAsPdf(){
    if (chart->series().size() == 0) return;
    QString file_name = QFileDialog::getSaveFileName(this, "Export as pdf", GlobalData::last_folder, "*.pdf (*.pdf)");
    if (!file_name.length()) return;
    
    try {
        QFileInfo fi(file_name);
        GlobalData::last_folder = fi.absoluteDir().absolutePath();
        
        QPrinter printer(QPrinter::ScreenResolution);
        printer.setOutputFormat(QPrinter::PdfFormat);
        printer.setFontEmbeddingEnabled(true);
        QPageSize pageSize(QSizeF(viewport()->width(), viewport()->height()) , QPageSize::Point);
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
        render(&p);
        p.end();
        
        QMessageBox::information(this, "Export completed", "The export is completed into the file '" + file_name + "'.");
    }
    catch(exception &){
        QMessageBox::critical(this, "Error during export", "An error occurred during the export into the file '" + file_name + "'. Is your hard disk drive full by chance or do you have enough permissions to write files into this folder?");
    }
}


double Statistics::median(vector<double> &lst, int begin, int end){
    int count = end - begin;
    if (count & 1) {
        return lst[(count >> 1) + begin];
    } else {
        double right = lst[(count >> 1) + begin];
        double left = lst[(count >> 1) - 1 + begin];
        return (right + left) / 2.0;
    }
}



/*
Sources:
https://www.cplusplus.com/forum/general/255896/
https://scicomp.stackexchange.com/questions/40536/hypergeometric-function-2f-1z-with-z-1-in-gsl
*/
double Statistics::hyperg_2F1(double a, double b, double c, double d){
    double e = 1.;
    if (d > 1. || d < 0){
        a = c - a;
        e = 1. / pow(1. - d, b);
        d = 1. - 1. / (1. - d);
    }
    
    if (b < 0){
        e *= pow(1 - d, c - a - b);
        a = c - a;
        b = c - b;
    }
    
    double TOLERANCE = 1.0e-10;
    double term = a * b * d / c;
    double p = 1.0 + term;
    double n = 1.;

    while (abs( term ) > TOLERANCE){
        a += 1.;
        b += 1.;
        c += 1.;
        n += 1.;
        term *= a * b * d / c / n;
        if (isinf(term) or isnan(term)) break;
        p += term;
    }
    return p * e;
}



/*
Source: https://en.wikipedia.org/wiki/Student%27s_t-distribution
*/
double Statistics::t_distribution_cdf(double t_stat, double free_deg){
    if (t_stat > 0) t_stat = -t_stat;
    return (0.5 + t_stat * exp(lgamma((free_deg + 1.) / 2.) - lgamma(free_deg / 2.)) * hyperg_2F1(0.5, (free_deg + 1.) / 2., 1.5, - sq(t_stat) / free_deg) / sqrt(M_PI * free_deg)) * 2.;
}


/*
Source: https://en.wikipedia.org/wiki/Welch%27s_t-test
*/
double Statistics::p_value_welch(Array &a, Array &b){
    double m1 = a.mean();
    double s1 = a.sample_stdev();
    double n = a.size();
    double m2 = b.mean();
    double s2 = b.sample_stdev();
    double m = b.size();
    
    double w0 = (m1 - m2);
    double t = w0 / sqrt(sq(s1 / sqrt(n)) + sq(s2 / sqrt(m)));
    double v = sq(sq(s1) / n + sq(s2) / m) / (sq(sq(s1)) / (sq(n) * (n - 1)) + sq(sq(s2)) / (sq(m) * (m - 1)));
    
    double pval = t_distribution_cdf(t, v);
    return max(0., min(1., pval));
}



/*
Source: https://en.wikipedia.org/wiki/Student%27s_t-test
*/
double Statistics::p_value_student(Array &sample1, Array &sample2){
    double m1 = sample1.mean();
    double s1 = sample1.sample_stdev();
    double n = sample1.size();
    double m2 = sample2.mean();
    double s2 = sample2.sample_stdev();
    double m = sample2.size();
    
    double s = sqrt(((n - 1) * sq(s1) + (m - 1) * sq(s2)) / (n + m - 2));
    double t = sqrt(n * m / (n + m)) * ((m1 - m2) / s);
    return t_distribution_cdf(t, n + m - 2);
}



/*
Sources:
https://en.wikipedia.org/wiki/F-distribution
https://docs.scipy.org/doc/scipy/reference/generated/scipy.special.betainc.html
*/
double Statistics::f_distribution_cdf(double f_stat, double df1, double df2){
    
    double x = df1 * f_stat / (df1 * f_stat + df2);
    double a = df1 / 2.;
    double b = df2 / 2.;
    double pval = 1. - pow(x, a) / a * hyperg_2F1(a, 1 - b, a + 1, x) / beta(a, b);
    return max(0., min(1., pval));
    
}



/*
Algorithm adapted from: arXiv:2102.08037
Thomas Viehmann
Numerically more stable computation of the p-values for the two-sample Kolmogorov-Smirnov test
*/
double Statistics::p_value_kolmogorov_smirnov(Array &sample1, Array &sample2){
    double d = 0, cdf1 = 0, cdf2 = 0;
    int ptr1 = 0, ptr2 = 0, m = sample1.size(), n = sample2.size();
    sort (sample1.begin(), sample1.end());
    sort (sample2.begin(), sample2.end());
    double inv_m = 1. / (double)m;
    double inv_n = 1. / (double)n;
    
    while (ptr1 < m && ptr2 < n){
        if (sample1[ptr1] < sample2[ptr2]){
            cdf1 += inv_m;
            ptr1 += 1;
        }
        else {
            cdf2 += inv_n;
            ptr2 += 1;
        }
        d = max(d, fabs(cdf1 - cdf2));
    }
    
    int size = 2 * m * d + 2;
    double *lastrow = new double[size];
    double *row = new double[size];
    for (int i = 0; i < size; ++i){
        lastrow[i] = 0;
        row[i] = 0;
    }
    int last_start_j = 0;
    int start_j = 0;
    for (int i = 0; i < n + 1; ++i){
        start_j = max((int)(m * ((double)i / n + d )) + 1 - size, 0);
        swap(lastrow, row);
        double val = 0;
        for (int jj = 0; jj < size; ++jj){
            int j = jj + start_j;
            double dist = (double)i / (double)n - (double)j / (double)m;
            if (dist > d || dist < -d) val = 1;
            else if (i == 0 || j == 0) val = 0;
            else if (jj + start_j - last_start_j >= size) val = (double)(i + val * j) / (double)(i + j);
            else  val = (lastrow[jj + start_j - last_start_j] * i + val * j) / (double)(i + j);
            row[jj] = val;
        }
        last_start_j = start_j;
    }
    double pval = row[m - start_j];
    
    
    delete []lastrow;
    delete []row;
    return max(0., min(1., pval));
}


/* 
Source: https://en.wikipedia.org/wiki/One-way_analysis_of_variance
*/
double Statistics::p_value_anova(vector<Array> &arrays){
    vector<double> mean_y;
    vector<double> std_y;
    double total_y = 0;
    for (auto a : arrays){
        mean_y.push_back(a.mean());
        std_y.push_back(a.sample_stdev());
        total_y += mean_y.back();
    }
    total_y /= (double)mean_y.size();
        
    double MSB = 0;
    double MSW = 0;
    double N = 0;
    for (uint i = 0; i < mean_y.size(); ++i) {
        MSB += arrays[i].size() * sq(mean_y[i] - total_y);
        MSW += (arrays[i].size() - 1.) * sq(std_y[i]);
        N += arrays[i].size();
    }
    double v1 = mean_y.size() - 1.;
    double v2 = N - arrays.size();
    MSB /= v1;
    MSW /= v2;
    
    
    double F = MSB / MSW;
    return f_distribution_cdf(F, v1, v2);
}



void Statistics::updateBarPlot(){
    
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
    setVisible(is_nominal);
    
    if (!is_nominal) return;
    
   
    
    
    // setup array for target variable values, if nominal then each with incrementing number
    map<string, double> nominal_target_values;
    Array target_values;
    vector<string> target_titles;
    int nom_counter = 0;
    vector<QBoxPlotSeries*> plot_series;
    
    for (auto lipidome : lipid_space->selected_lipidomes){
        string nominal_value = lipidome->features[target_variable].nominal_value;
        if (uncontains_val(nominal_target_values, nominal_value)){
            nominal_target_values.insert({nominal_value, nom_counter++});
            plot_series.push_back(new QBoxPlotSeries());
            plot_series.back()->setName(nominal_value.c_str());
            target_titles.push_back(nominal_value);
            string color_key = target_variable + "_" + nominal_value;
            if (contains_val(GlobalData::colorMapFeatures, color_key)){
                QBrush brush(GlobalData::colorMapFeatures[color_key]);
                plot_series.back()->setBrush(brush);
            }
        }
        target_values.push_back(nominal_target_values[nominal_value]);
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
    
    vector<string> lipid_names(lipid_map.size(), "");
    for (auto kv : lipid_name_map) lipid_names[kv.second] = kv.first;
    
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
    
    for (int t = 0; t < nom_counter; ++t){
        for (int c = 0; c < global_matrix.cols; c++){
            series_titles.push_back(lipid_names[c] + " / " + target_titles[t]);
            plot_series[t]->append(new QBoxSet(lipid_names[c].c_str()));
        }
    }
    
    for (int r = 0; r < global_matrix.rows; ++r){ // for all values of a study variable
        for (int c = 0; c < global_matrix.cols; c++){
            if (global_matrix(r, c) > 1e-15){
                plot_series[target_values[r]]->boxSets()[c]->append(global_matrix(r, c));
            }
        }
    }
    
    series.resize(series_titles.size());
    int series_iter = 0;
    for (auto single_plot_series : plot_series){
        
        for (auto box : single_plot_series->boxSets()){
            Array vals;
            for (int i = 0; i < box->count(); ++i){
                vals.push_back(box->at(i));
                series[series_iter].push_back(box->at(i));
            }
            ++series_iter;
            
            double mean = vals.mean();
            if (isnan(mean) || isinf(mean)) mean = 0;
            double std = vals.sample_stdev();
            if (isnan(std) || isinf(std)) std = 0;
            
            box->setValue(QBoxSet::LowerExtreme, 0);
            box->setValue(QBoxSet::UpperExtreme, mean + std);
            box->setValue(QBoxSet::Median, 0);
            box->setValue(QBoxSet::LowerQuartile, 0);
            box->setValue(QBoxSet::UpperQuartile, mean);
        }
        single_plot_series->setBoxWidth(1.5);
        chart->addSeries(single_plot_series);
    }
    
    chart->createDefaultAxes();
    
    for (auto axis : chart->axes()){
        axis->setLabelsFont(QFont("Helvetica", GlobalData::gui_num_var["tick_size"]));
    }
}







void Statistics::updateHistogram(){
    
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
    setVisible(is_nominal);
    
    if (!is_nominal) return;
    
    // setup array for target variable values, if nominal then each with incrementing number
    map<string, double> nominal_target_values;
    Array target_values;
    int nom_counter = 0;
    vector<QBarSet*> plot_series;
    
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
        target_values.push_back(nominal_target_values[nominal_value]);
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

    series.resize(nom_counter);
    double all_min = 1e9;
    double all_max = -1e9;
    for (int r = 0; r < global_matrix.rows; ++r){
        double sum = 0;
        for (int c = 0; c < global_matrix.cols; c++) sum += global_matrix(r, c);
        if (global_matrix.cols > 1 || sum > 1e-15){
            series[target_values[r]].push_back(sum);
            all_min = min(all_min, sum);
            all_max = max(all_max, sum);
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
}










void Statistics::updateBoxPlot(){
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
    
    
    // setup array for target variable values, if nominal then each with incrementing number
    map<string, double> nominal_target_values;
    Array target_values;
    int nom_counter = 0;
    vector<QBoxPlotSeries*> plot_series;
    if (is_nominal){
        for (auto lipidome : lipid_space->selected_lipidomes){
            string nominal_value = lipidome->features[target_variable].nominal_value;
            if (uncontains_val(nominal_target_values, nominal_value)){
                nominal_target_values.insert({nominal_value, nom_counter++});
                plot_series.push_back(new QBoxPlotSeries());
                plot_series.back()->setName(nominal_value.c_str());
                series_titles.push_back(nominal_value);
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
    if (is_nominal){
        if (global_matrix.cols > 1) global_matrix.scale();
    
        series.resize(nom_counter);
        for (int r = 0; r < global_matrix.rows; ++r){
            double sum = 0;
            for (int c = 0; c < global_matrix.cols; c++) sum += global_matrix(r, c);
            if (global_matrix.cols > 1 || sum > 1e-15){
                plot_series[target_values[r]]->boxSets()[0]->append(sum);
                series[target_values[r]].push_back(sum);
            }
        }
        for (uint i = 0; i < plot_series.size(); ++i){
            auto single_plot_series = plot_series[i];
            QBoxSet *box = single_plot_series->boxSets()[0];
            
            Array &single_series = series[i];
            sort(single_series.begin(), single_series.end());
            int count = single_series.size();
            box->setValue(QBoxSet::LowerExtreme, single_series.front());
            box->setValue(QBoxSet::UpperExtreme, single_series.back());
            box->setValue(QBoxSet::Median, median(single_series, 0, count));
            box->setValue(QBoxSet::LowerQuartile, median(single_series, 0, count >> 1));
            box->setValue(QBoxSet::UpperQuartile, median(single_series, (count >> 1) + (count & 1), count));
            
            chart->addSeries(single_plot_series);
        }
        if (nom_counter == 2){
            double p_student = p_value_student(series[0], series[1]);
            double p_welch = p_value_welch(series[0], series[1]);
            double p_ks = p_value_kolmogorov_smirnov(series[0], series[1]);
            stat_results.insert({"p_value(Student)", p_student});
            stat_results.insert({"p_value(Welch)", p_welch});
            stat_results.insert({"p_value(KS)", p_ks});
            chart->setTitle(QString("Statistics: <i>p</i>-value<sub>Student</sub> = %1,   <i>p</i>-value<sub>Welch</sub> = %2,   <i>p</i>-value<sub>KS</sub> = %3").arg(QString::number(p_student, 'g', 3)).arg(QString::number(p_welch, 'g', 3)).arg(QString::number(p_ks, 'g', 3)));
        }
        else if (nom_counter > 2){
            double p_anova = p_value_anova(series);
            stat_results.insert({"p_value(ANOVA)", p_anova});
            chart->setTitle(QString("Statistics: <i>p</i>-value<sub>ANOVA</sub> = %1").arg(QString::number(p_anova, 'g', 3)));
        }
    }
    else {
        Array constants;
        constants.resize(global_matrix.rows, 1);
        global_matrix.add_column(constants);
        
        double min_x = 1e100;
        double max_x = 0;
        Array coefficiants;
        coefficiants.compute_coefficiants(global_matrix, target_values);    // estimating coefficiants
        Array S;
        S.mult(global_matrix, coefficiants);
    
        QScatterSeries* series_model = new QScatterSeries();
        series_model->setName((target_variable + " / Linear lipid model").c_str());
        for (uint i = 0; i < S.size(); ++i){
            series_model->append(S[i], target_values[i]);
            min_x = min(min_x, S[i]);
            max_x = max(max_x, S[i]);
        }
        chart->addSeries(series_model);
        
        
        
        
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
        
        QLineSeries* series_regression = new QLineSeries();
        series_regression->setName(QString().asprintf("Model regression (R<sup>2</sup> = %0.3f)", R2));
        series_regression->append(min_x, slope * min_x + intercept);
        series_regression->append(max_x, slope * max_x + intercept);
        chart->addSeries(series_regression);
    }
    chart->createDefaultAxes();
    
    for (auto axis : chart->axes()){
        if (is_nominal && axis->orientation() == Qt::Horizontal){
            chart->removeAxis(axis);
        }
        else {
            axis->setLabelsFont(QFont("Helvetica", GlobalData::gui_num_var["tick_size"]));
        }
    }
}
