#include "lipidspace/statistics.h"




Statistics::Statistics(QWidget *parent) : QChartView(parent) {
    lipid_space = 0;
    chart = new QChart();
    chart->setAnimationOptions(QChart::SeriesAnimations);
    chart->createDefaultAxes();
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);
    setChart(chart);
    chart->legend()->setFont(QFont("Helvetica", GlobalData::gui_num_var["legend_size"]));
}


void Statistics::set_lipid_space(LipidSpace *_lipid_space){
    lipid_space = _lipid_space;
}



void Statistics::setLegendSize(int font_size){
    resetMatrix();
    GlobalData::gui_num_var["legend_size"] = font_size;
    chart->legend()->setFont(QFont("Helvetica", font_size));
}



void Statistics::setTickSize(int font_size){
    resetMatrix();
    GlobalData::gui_num_var["tick_size"] = font_size;
    updateChart();
}




void Statistics::exportAsPdf(){
    if (chart->series().size() == 0) return;
    QString file_name = QFileDialog::getSaveFileName(this, "Export as pdf", GlobalData::last_folder, "*.pdf (*.pdf)");
    if (!file_name.length()) return;
    
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


double Statistics::hyperg_2F1(double a, double b, double c, double d){
    double e = 1.;
    if (d > 1. || d < 0){
        a = c - a;
        e = pow(1. - d, b);
        d = 1. - 1. / (1. - d);
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
    return p / e;
}


double Statistics::t_distribution_cdf(double t_stat, double free_deg){
    if (t_stat > 0) t_stat = -t_stat;
    return (0.5 + t_stat * tgamma((free_deg + 1.) / 2.) * hyperg_2F1(0.5, (free_deg + 1.) / 2., 1.5, - sq(t_stat) / free_deg) / (sqrt(M_PI * free_deg) * tgamma(free_deg / 2.))) * 2.;
}


double Statistics::p_value_welch(Array &a, Array &b){
    double m1 = a.mean();
    double s1 = a.sample_stdev();
    double n = a.size();
    double m2 = b.mean();
    double s2 = b.sample_stdev();
    double m = b.size();
    
    double w0 = (m1 - m2);
    double t = w0 / sqrt(sq(s1 / sqrt(n)) + sq(s2 / sqrt(m)));
    cout << "t: " << t << endl;
    double v = sq(sq(s1) / n + sq(s2) / m) / (sq(sq(s1)) / (sq(n) * (n - 1)) + sq(sq(s2)) / (sq(m) * (m - 1)));
    
    return t_distribution_cdf(t, v);
}


double Statistics::p_value_student(Array &a, Array &b){
    double m1 = a.mean();
    double s1 = a.sample_stdev();
    double n = a.size();
    double m2 = b.mean();
    double s2 = b.sample_stdev();
    double m = b.size();
    
    double s = sqrt(((n - 1) * sq(s1) + (m - 1) * sq(s2)) / (n + m - 2));
    double w0 = (m1 - m2);
    double t = sqrt(n * m / (n + m)) * (w0 / s);
    
    return t_distribution_cdf(t, n + m - 2);
}




double Statistics::f_distribution_cdf(double f_stat, double df1, double df2){
    
    double x = df1 * f_stat / (df1 * f_stat + df2);
    double a = df1 / 2.;
    double b = df2 / 2.;
    double B = exp(lgamma(a) + lgamma(b) - lgamma(a + b));
    return 1. - pow(x, a) * hyperg_2F1(a, 1 - b, a + 1, x) / (a * B);
    
}



double Statistics::p_value_anova(vector<Array*> &v){
    vector<double> mean_y;
    vector<double> std_y;
    double total_y = 0;
    for (auto a : v){
        mean_y.push_back(a->mean());
        std_y.push_back(a->sample_stdev());
        total_y += mean_y.back();
    }
    total_y /= (double)mean_y.size();
        
    double MS = 0;
    double SQR = 0;
    double N = 0;
    for (uint i = 0; i < mean_y.size(); ++i) {
        MS += v[i]->size() * sq(mean_y[i] - total_y);
        SQR += (v[i]->size() - 1.) * sq(std_y[i]);
        N += v[i]->size();
    }
    MS /= ((double)mean_y.size() - 1.);
    SQR /= (N - v.size());
    
    
    double F = MS / SQR;
    return f_distribution_cdf(F, v.size() - 1, N - v.size());
}



void Statistics::updateChart(){
    chart->removeAllSeries();
    
    
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
    
        vector<vector<double>> series(nom_counter);
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
            
            vector<double> &single_series = series[i];
            sort(single_series.begin(), single_series.end());
            int count = single_series.size();
            box->setValue(QBoxSet::LowerExtreme, single_series.front());
            box->setValue(QBoxSet::UpperExtreme, single_series.back());
            box->setValue(QBoxSet::Median, median(single_series, 0, count));
            box->setValue(QBoxSet::LowerQuartile, median(single_series, 0, count >> 1));
            box->setValue(QBoxSet::UpperQuartile, median(single_series, (count >> 1) + (count & 1), count));
            
            chart->addSeries(single_plot_series);
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
