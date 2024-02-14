#include "lipidspace/AssistanceFunctions.h"
#include "lipidspace/globaldata.h"


LipidSpaceException::LipidSpaceException(string _message, LipidSpaceExceptionType _type){
    message = _message;
    type = _type;
    Logging::write_log(message);
}


namespace {
    const int scSearchBarIndex = 0;
}



OntologyTerm::OntologyTerm(string _term_id, string _name, vector<string> &_relations, string _domain){
    term_id = _term_id;
    name = _name;
    for (auto rel : _relations) relations.push_back(rel);
    domain = _domain;
}


void fast_split(char* text, int len, char delimiter, vector<string_view> &list){
    if (len == 0) return;

    int last_pos = 0;
    for (int i = 0; i < len; ++i){
        if (text[i] == delimiter){
            list.push_back(string_view(text + last_pos, i - last_pos));
            last_pos = i + 1;
        }
    }
    list.push_back(string_view(text + last_pos, len - last_pos));
}



OntologyEnrichment::OntologyEnrichment(LipidParser *l){
    // inflate ontology file
    ifstream ifs(QCoreApplication::applicationDirPath().toStdString() + "/data/ontology_mmu.gz", ios::in | ios::binary | ios::ate);

    if (!ifs.good()){
        Logging::write_log("Error: file '/data/ontology.gz' not found.");
        throw LipidException("Error: file '/data/ontology.gz' not found. Please check the log message.");
    }

    // inflate file
    unsigned fileSize = ifs.tellg();
    ifs.seekg(0, ios::beg);
    char* bytes = new char[fileSize];
    ifs.read(bytes, fileSize);
    unsigned half_length = fileSize / 2;

    unsigned uncompLength = fileSize;
    char* uncomp = new char[uncompLength];

    z_stream strm;
    strm.next_in = (Bytef *)bytes;
    strm.avail_in = fileSize;
    strm.total_out = 0;
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;

    bool done = false;

    if (inflateInit2(&strm, (16 + MAX_WBITS)) != Z_OK) {
        delete []uncomp;
        delete []bytes;
        Logging::write_log("Error: file '/data/ontology.gz' is corrupted (code inflateInit2).");
        throw LipidException("Error: file '/data/ontology.gz' not found. Please check the log message.");
    }

    while (!done) {
        if (strm.total_out >= uncompLength ) {
            char* uncomp2 = new char[uncompLength + half_length];
            memcpy( uncomp2, uncomp, uncompLength );
            uncompLength += half_length ;
            delete []uncomp;
            uncomp = uncomp2;
        }

        strm.next_out = (Bytef *) (uncomp + strm.total_out);
        strm.avail_out = uncompLength - strm.total_out;

        // Inflate another chunk.
        int err = inflate (&strm, Z_SYNC_FLUSH);
        if (err == Z_STREAM_END) done = true;
        else if (err != Z_OK) break;
    }

    if (inflateEnd (&strm) != Z_OK) {
        delete []uncomp;
        Logging::write_log("Error: file '/data/ontology.gz' is corrupted (code inflateEnd).");
        throw LipidException("Error: file '/data/ontology.gz' not found. Please check the log message.");
    }
    delete []bytes;


    lipid_parser = l;
    string term_id = "";
    string name = "";
    bool is_lipid_species = false;
    bool is_lipid_class = false;
    bool is_carbon_chain = false;
    string domain = "";
    vector<string> relations;
    vector<string> synonyms;

    vector<string_view> lines;
    fast_split(uncomp, strm.total_out, '\n', lines);

    for (auto line : lines){
        if (!line.length()) continue;

        if (line == "[Term]"){
            if (term_id != "" && name != "" && name.find("inding") == string::npos){

                if (contains_val(ontology_terms, term_id)){
                    throw LipidSpaceException("Term id '" + term_id + "' in lipid ontology already defined.");
                }
                OntologyTerm *term = new OntologyTerm(term_id, name, relations, domain);

                if (is_lipid_species){
                    if (uncontains_val(lipids, name)) lipids.insert({name, term});
                }
                if (is_lipid_class){
                    for (auto synonym : synonyms){
                        if (uncontains_val(lipid_classes, synonym)) lipid_classes.insert({synonym, {term}});
                        else lipid_classes.at(synonym).push_back(term);
                    }
                }
                if (is_carbon_chain){
                    for (auto synonym : synonyms){
                        if (uncontains_val(carbon_chains, synonym)) carbon_chains.insert({synonym, term});
                    }
                }

                if (domain.length() > 0){
                    search_terms.insert({term_id, set<string>()});
                    domains.insert(domain);
                }

                ontology_terms.insert({term_id, term});
            }
            term_id = "";
            name = "";
            domain = "";
            is_lipid_species = false;
            is_lipid_class = false;
            is_carbon_chain = false;
            synonyms.clear();
            relations.clear();
        }

        else if (line.substr(0, 4) == "id: "){
            term_id = line.substr(4);
        }

        else if (line.substr(0, 6) == "name: "){
            name = line.substr(6);
        }
        else if (line.substr(0, 6) == "is_a: "){
            string sub_line = string(line.substr(6));
            uint pos = sub_line.find(" ! ");
            string relation = sub_line.substr(0, pos);
            relations.push_back(relation);

            if (relation == "LS:0000001") is_lipid_class = true;
            else if (relation == "LS:0000002") is_lipid_species = true;
            else if (relation == "LS:0000003") is_carbon_chain = true;
        }

        else if (line.substr(0, 14) == "relationship: "){
            line = line.substr(14);
            uint pos = line.find(" ");
            if (pos != string::npos){
                line = line.substr(pos + 1);
                pos = line.find(" ! ");
                line = line.substr(0, pos);
                relations.push_back(string(line));
            }
        }

        else if (line.substr(0, 11) == "namespace: "){
            domain = line.substr(11);
        }
        else if (line == "is_obsolete: true"){
            term_id = "";
            name = "";
        }
        else if (line.substr(0, 8) == "synonym:"){
            vector<string_view> tokens;
            fast_split((char*)line.data(), line.length(), '"', tokens);
            if (tokens.size() >= 2) synonyms.push_back(string(tokens.at(1)));
        }
    }


    if (term_id != "" && name != "" && name.find("inding") == string::npos){
        if (contains_val(ontology_terms, term_id)){
            throw LipidSpaceException("Term id '" + term_id + "' in lipid ontology already defined.");
        }
        OntologyTerm *term = new OntologyTerm(term_id, name, relations, domain);

        if (is_lipid_species){
            if (uncontains_val(lipids, name)) lipids.insert({name, term});
        }
        if (is_lipid_class){
            for (auto synonym : synonyms){
                if (uncontains_val(lipid_classes, synonym)) lipid_classes.insert({synonym, vector<OntologyTerm*>()});
                lipid_classes.at(synonym).push_back(term);
            }
        }
        if (is_carbon_chain){
            for (auto synonym : synonyms){
                if (uncontains_val(carbon_chains, synonym)) carbon_chains.insert({synonym, term});
            }
        }

        if (domain.length() > 0){
            search_terms.insert({term_id, set<string>()});
            domains.insert(domain);
        }

        ontology_terms.insert({term_id, term});
    }


    if (contains_val(domains, "external")) domains.erase("external");
    delete []uncomp;
}


OntologyEnrichment::~OntologyEnrichment(){
    for (auto &kv : ontology_terms) delete kv.second;
}


void OntologyEnrichment::set_background_lipids(vector<string> &lipid_list){
    for (auto &kv : search_terms){
        kv.second.clear();
        ontology_terms.at(kv.first)->lipid_paths.clear();
    }
    compute_event_occurrance(lipid_list, search_terms);

    for (auto &kv : search_terms){
        for (auto &paths : ontology_terms.at(kv.first)->lipid_paths){
            kv.second.insert(paths.first);
        }
    }

    num_background = (int)lipid_list.size();
}



void OntologyEnrichment::recursive_event_adding(string lipid_input_name, string term_id, set<string> &visited_terms, map<string, set<string>> &occ_list, int recursion, vector<string> *path){
    if (recursion > 0) return;

    bool delete_path = false;
    if (path == nullptr){
        delete_path = true;
        path = new vector<string>();
    }


    /*
    if (contains_val(occ_list, term_id)){
        occ_list.at(term_id).insert(lipid_input_name);
    }
    */

    if (contains_val(ontology_terms, term_id)){
        OntologyTerm *term = ontology_terms.at(term_id);
        path->push_back(term_id);

        if (uncontains_val(term->lipid_paths, lipid_input_name)){
            term->lipid_paths.insert({lipid_input_name, map<string, vector<string> >()});
            map<string, vector<string> > &lipid_paths = term->lipid_paths.at(lipid_input_name);
            if (uncontains_val(lipid_paths, path->front())) lipid_paths.insert({path->front(), vector<string>()});
            vector<string> &lipid_path = lipid_paths.at(path->front());
            for (string path_id : *path) lipid_path.push_back(path_id);
        }



        int next_recursion = recursion + contains_val(domains, term->domain);
        for (string parent_term_id : term->relations){
            if (uncontains_val(visited_terms, parent_term_id)){
                visited_terms.insert(parent_term_id);

                recursive_event_adding(lipid_input_name, parent_term_id, visited_terms, occ_list, next_recursion, path);
            }
        }
        path->pop_back();
    }


    if (delete_path) delete path;
}



void OntologyEnrichment::compute_event_occurrance(vector<string> &lipid_list, map<string, set<string>> &occ_list){
    for (string lipid_input_name : lipid_list){
        try {
            LipidAdduct *lipid = lipid_parser->parse(lipid_input_name);
            if (lipid->lipid->info->level > MOLECULAR_SPECIES) lipid->lipid->info->level = MOLECULAR_SPECIES;
            lipid->sort_fatty_acyl_chains();
            string lipid_name = lipid->get_lipid_string();
            string lipid_name_species = lipid->get_lipid_string(SPECIES);
            string lipid_name_class = lipid->get_extended_class();

            set<string> visited_terms;
            if (contains_val(lipids, lipid_name)){
                OntologyTerm *term = lipids.at(lipid_name);
                if (uncontains_val(visited_terms, term->term_id)){
                    visited_terms.insert(term->term_id);
                    recursive_event_adding(lipid_input_name, term->term_id, visited_terms, occ_list);
                }
            }
            if (contains_val(lipids, lipid_name_species)){
                OntologyTerm *term = lipids.at(lipid_name_species);
                if (uncontains_val(visited_terms, term->term_id)){
                    visited_terms.insert(term->term_id);
                    recursive_event_adding(lipid_input_name, term->term_id, visited_terms, occ_list);
                }
            }
            if (contains_val(lipid_classes, lipid_name_class)){
                for (auto term : lipid_classes.at(lipid_name_class)){
                    if (uncontains_val(visited_terms, term->term_id)){
                        visited_terms.insert(term->term_id);
                        recursive_event_adding(lipid_input_name, term->term_id, visited_terms, occ_list);
                    }
                }
            }

            if (lipid != nullptr && lipid->lipid != nullptr){
                for (auto fa : lipid->lipid->fa_list){
                    if (fa->num_carbon == 0) continue;

                    string fa_string = ((fa->lipid_FA_bond_type == LCB_REGULAR || fa->lipid_FA_bond_type == LCB_EXCEPTION) ? "L" : "C") + fa->to_string(MOLECULAR_SPECIES);
                    if (contains_val(carbon_chains, fa_string)){
                        visited_terms.insert(carbon_chains.at(fa_string)->term_id);
                        recursive_event_adding(lipid_input_name, carbon_chains.at(fa_string)->term_id, visited_terms, occ_list);
                    }
                }
            }
            delete lipid;
        }
        catch (exception &){

        }
    }
}


int intersection_count(set<string> &s1, set<string> &s2){
    int intersections = 0;
    auto p1 = s1.begin();
    auto p2 = s2.begin();


    while (p1 != s1.end() && p2 != s2.end()){
        int comp = (*p1).compare((*p2));
        if (comp == 0){
            intersections++;
            p1++;
            p2++;
        }
        else if (comp < 0) p1++;
        else p2++;
    }
    return intersections;
}



void OntologyEnrichment::enrichment_analysis(vector<string> &target_list, vector<OntologyResult> &result_list){
    if (num_background == 0) return;

    set<string> target_set;
    for (auto target : target_list) target_set.insert(target);

    for (auto &kv : search_terms){
        string term_id = kv.first;

        if (kv.second.size() == 0 || uncontains_val(ontology_terms, term_id) || uncontains_val(GlobalData::enrichment_domains, ontology_terms.at(term_id)->domain)) continue;

        int target_number = intersection_count(kv.second, target_set);

        double p_hyp = exact_fischer(num_background, kv.second.size(), (int)target_list.size(), target_number);
        result_list.push_back(OntologyResult{ontology_terms.at(term_id), num_background, (int)kv.second.size(), (int)target_list.size(), set<string>(), p_hyp});
        //for (auto lipid_name : search_target_terms.at(kv.first)) result_list.back().target_events.insert(lipid_name);
    }


    /*
    map<string, set<string>> search_target_terms;
    for (auto &kv : search_terms) search_target_terms.insert({kv.first, set<string>()});




    compute_event_occurrance(target_list, search_target_terms);


    for (auto &kv : search_terms){
        string term_id = kv.first;

        if (kv.second.size() == 0 || uncontains_val(ontology_terms, term_id) || uncontains_val(GlobalData::enrichment_domains, ontology_terms.at(term_id)->domain)) continue;

        double p_hyp = exact_fischer(num_background, kv.second.size(), (int)target_list.size(), search_target_terms.at(kv.first).size());
        cout << p_hyp << endl;
        result_list.push_back(OntologyResult{ontology_terms.at(term_id), num_background, (int)kv.second.size(), (int)target_list.size(), set<string>(), p_hyp});
        for (auto lipid_name : search_target_terms.at(kv.first)) result_list.back().target_events.insert(lipid_name);
    }
    */
}






MultiSelectComboBox::MultiSelectComboBox(QWidget* aParent) :
    QComboBox(aParent),
    mListWidget(new QListWidget(this)),
    mLineEdit(new QLineEdit(this)),
    mSearchBar(new QLineEdit(this))
{
    QListWidgetItem* curItem = new QListWidgetItem(mListWidget);
    mSearchBar->setPlaceholderText("Search ...");
    mSearchBar->setClearButtonEnabled(true);
    mListWidget->addItem(curItem);
    mListWidget->setItemWidget(curItem, mSearchBar);

    mLineEdit->setReadOnly(true);
    mLineEdit->installEventFilter(this);

    setModel(mListWidget->model());
    setView(mListWidget);
    setLineEdit(mLineEdit);

    connect(mSearchBar, &QLineEdit::textChanged, this, &MultiSelectComboBox::onSearch);
    connect(this, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, &MultiSelectComboBox::itemClicked);
}

void MultiSelectComboBox::hidePopup()
{
    int width = this->width();
    int height = mListWidget->height();
    int x = QCursor::pos().x() - mapToGlobal(geometry().topLeft()).x() + geometry().x();
    int y = QCursor::pos().y() - mapToGlobal(geometry().topLeft()).y() + geometry().y();
    if (x >= 0 && x <= width && y >= this->height() && y <= height + this->height())
    {
        // Item was clicked, do not hide popup
    }
    else
    {
        QComboBox::hidePopup();
    }
}

void MultiSelectComboBox::stateChanged(int aState)
{
    Q_UNUSED(aState);
    stringList.clear();
    QString selectedData("");
    int count = mListWidget->count();

    for (int i = 1; i < count; ++i)
    {
        QWidget *widget = mListWidget->itemWidget(mListWidget->item(i));
        QCheckBox *checkBox = static_cast<QCheckBox *>(widget);

        if (checkBox->isChecked())
        {
            if (selectedData.length() > 0) selectedData.append(", ");
            selectedData.append(checkBox->text());
            stringList.append(checkBox->text());
        }
    }
    if (!selectedData.isEmpty())
    {
        mLineEdit->setText(selectedData);
    }
    else
    {
        mLineEdit->clear();
    }

    mLineEdit->setToolTip(selectedData);
    emit selectionChanged();
}

void MultiSelectComboBox::addItem(const QString& aText, const QVariant& aUserData, bool checked)
{
    Q_UNUSED(aUserData);
    QListWidgetItem* listWidgetItem = new QListWidgetItem(mListWidget);
    QCheckBox* checkBox = new QCheckBox(this);
    checkBox->setText(aText);
    if (checked) checkBox->setCheckState(Qt::Checked);
    mListWidget->addItem(listWidgetItem);
    mListWidget->setItemWidget(listWidgetItem, checkBox);
    connect(checkBox, &QCheckBox::stateChanged, this, &MultiSelectComboBox::stateChanged);
    if (checked) stateChanged(0);
}

QStringList MultiSelectComboBox::currentText()
{
    return stringList;
}

void MultiSelectComboBox::addItems(const QStringList& aTexts)
{
    for(const auto& string : aTexts)
    {
        addItem(string);
    }
}

int MultiSelectComboBox::count() const {
    return max(0, mListWidget->count() - 1); // Do not count the search bar
}

void MultiSelectComboBox::onSearch(const QString& aSearchString)
{
    for(int i = 1; i < mListWidget->count(); i++)
    {
        QCheckBox* checkBox = static_cast<QCheckBox*>(mListWidget->itemWidget(mListWidget->item(i)));
        if(checkBox->text().contains(aSearchString, Qt::CaseInsensitive))
        {
            mListWidget->item(i)->setHidden(false);
        }
        else
        {
            mListWidget->item(i)->setHidden(true);
        }
    }
}


void MultiSelectComboBox::itemClicked(int aIndex)
{
    if(aIndex != scSearchBarIndex)// 0 means the search bar
    {
        QWidget* widget = mListWidget->itemWidget(mListWidget->item(aIndex));
        QCheckBox *checkBox = static_cast<QCheckBox *>(widget);
        checkBox->setChecked(!checkBox->isChecked());
    }
}

void MultiSelectComboBox::SetSearchBarPlaceHolderText(const QString& aPlaceHolderText)
{
    mSearchBar->setPlaceholderText(aPlaceHolderText);
}

void MultiSelectComboBox::SetPlaceHolderText(const QString& aPlaceHolderText)
{
    mLineEdit->setPlaceholderText(aPlaceHolderText);
}

void MultiSelectComboBox::clear()
{
    mListWidget->clear();
    QListWidgetItem* curItem = new QListWidgetItem(mListWidget);
    mSearchBar = new QLineEdit(this);
    mSearchBar->setPlaceholderText("Search ...");
    mSearchBar->setClearButtonEnabled(true);
    mListWidget->addItem(curItem);
    mListWidget->setItemWidget(curItem, mSearchBar);

    connect(mSearchBar, &QLineEdit::textChanged, this, &MultiSelectComboBox::onSearch);
}

void MultiSelectComboBox::wheelEvent(QWheelEvent* aWheelEvent)
{
    // Do not handle the wheel event
    Q_UNUSED(aWheelEvent);
}

bool MultiSelectComboBox::eventFilter(QObject* aObject, QEvent* aEvent)
{
    if(aObject == mLineEdit && aEvent->type() == QEvent::MouseButtonRelease) {
        showPopup();
        return false;
    }
    return false;
}

void MultiSelectComboBox::keyPressEvent(QKeyEvent* aEvent)
{
    // Do not handle key event
    Q_UNUSED(aEvent);
}

void MultiSelectComboBox::setCurrentText(const QString& aText)
{
    Q_UNUSED(aText);
}

void MultiSelectComboBox::setCurrentText(const QStringList& aText)
{
    int count = mListWidget->count();

    for (int i = 1; i < count; ++i)
    {
        QWidget* widget = mListWidget->itemWidget(mListWidget->item(i));
        QCheckBox* checkBox = static_cast<QCheckBox*>(widget);
        QString checkBoxString = checkBox->text();
        if(aText.contains(checkBoxString))
        {
            checkBox->setChecked(true);
        }
    }
}

void MultiSelectComboBox::ResetSelection()
{
    int count = mListWidget->count();

    for (int i = 1; i < count; ++i)
    {
        QWidget *widget = mListWidget->itemWidget(mListWidget->item(i));
        QCheckBox *checkBox = static_cast<QCheckBox *>(widget);
        checkBox->setChecked(false);
    }
}







SingleListWidget::SingleListWidget(QWidget *parent) : QListWidget(parent) {
    field_name = "";
    num = 1;
}

void SingleListWidget::addFieldName(string _field_name){
    field_name = _field_name;
}

void SingleListWidget::setNum(int _num){
    num = _num;
}

void SingleListWidget::dropEvent(QDropEvent *event){
    if (count() + ((QListWidget*)event->source())->selectedItems().size() > num){
        oneItemViolation(field_name, num);
    }
    else {
        QListWidget::dropEvent(event);
    }
}




FADTreeWidget::FADTreeWidget(QWidget *parent) : QTreeWidget(parent){
    setMouseTracking(true);
}


void FADTreeWidget::enterEvent(QEvent *event){
    emit hoverEnter(event);
}


void FADTreeWidget::leaveEvent(QEvent *event){
    emit hoverLeave(event);
}





CBTableView::CBTableView(QWidget *parent) : QTableView(parent) {
    QAbstractButton *btn = findChild<QAbstractButton *>();
    transpose_label = 0;
    setAcceptDrops(true);

    if (btn != NULL) {
        QVBoxLayout* lay = new QVBoxLayout(btn);
        lay->setContentsMargins(0, 0, 0, 0);
        transpose_label = new QLabel("Transpose");
        transpose_label->setAlignment(Qt::AlignCenter);
        transpose_label->setContentsMargins(2, 2, 2, 2);
        lay->addWidget(transpose_label);
        btn->disconnect(SIGNAL(clicked()));
        connect(btn, SIGNAL(clicked()), this, SLOT(cornerButtonClicked()));
    }
}


void CBTableView::cornerButtonClicked(){
    cornerButtonClick();
}


void CBTableView::wheelEvent(QWheelEvent* event){
    if (GlobalData::ctrl_pressed){
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        bool zoom_in = event->angleDelta().y() > 0;
#else
        bool zoom_in = event->delta() > 0;
#endif
        int zoom_value = max(1, (int)GlobalData::gui_num_var.at("table_zoom") + (2 * zoom_in - 1));
        GlobalData::gui_num_var.at("table_zoom") = zoom_value;
        QFont item_font("Helvetica", zoom_value);
        transpose_label->setFont(item_font);
        emit zooming();
    }
    else {
        QTableView::wheelEvent(event);
    }
}


void CBTableView::dragEnterEvent(QDragEnterEvent* event){
   event->acceptProposedAction();
}


void CBTableView::dragMoveEvent(QDragMoveEvent* event){
   event->acceptProposedAction();
}


void CBTableView::dragLeaveEvent(QDragLeaveEvent* event){
   event->accept();
}


void CBTableView::dropEvent(QDropEvent *event){
    if (event->mimeData()->hasUrls()){
        emit openFiles(event->mimeData()->urls());
    }
}




Mapping::Mapping(){
    name = "";
    parent = "";
    action = NoAction;
    study_variable_type = NominalStudyVariable;
    mapping = "";
}

Mapping::Mapping(string _name, StudyVariableType v_type){
    name = _name;
    parent = "";
    study_variable_type = v_type;
    action = NoAction;
    mapping = "";
}



void Analytics::analytics(string action) {
    ifstream infile(QCoreApplication::applicationDirPath().toStdString() + "/data/analytics.txt");
    int result = 0;
    if (!infile.good()){
        return;
    }
    else if (infile >> result) {
        if (result == 1){
            string url = "https://lifs-tools.org/matomo/matomo.php?idsite=14&rec=1&e_c=LipidSpace-" + GlobalData::LipidSpace_version + "&e_a=" + action;
            reply.reset(qnam.get(QNetworkRequest(QUrl(url.c_str()))));
        }
    }

    infile.close();
}


ImportData::ImportData(string _table_file, string _sheet, TableType _table_type, vector<TableColumnType> *_column_types){
    table_file = _table_file;
    sheet = _sheet;
    table_type = _table_type;
    column_types = _column_types;
    mapping_data = 0;

    ifstream infile(table_file);
    if (!infile.good()){
        throw LipidSpaceException("Error: file '" + table_file + "' could not be found.", FileUnreadable);
    }
    try {
        file_table_handler = new FileTableHandler(table_file, sheet);
    }
    catch (OpenXLSX::XLInputError &e){
        throw LipidSpaceException("Error: file '" + table_file + "' could not be read.", FileUnreadable);
    }
}



ImportData::ImportData(string _table_file, string _sheet, TableType _table_type, vector<TableColumnType> *_column_types, FileTableHandler *_file_table_handler){
    table_file = _table_file;
    sheet = _sheet;
    table_type = _table_type;
    column_types = _column_types;
    file_table_handler = _file_table_handler;
    mapping_data = 0;
}



ImportData::ImportData(string _table_file, string _sheet, TableType _table_type, vector<TableColumnType> *_column_types, FileTableHandler *_file_table_handler, MappingData *_mapping_data){
    table_file = _table_file;
    sheet = _sheet;
    table_type = _table_type;
    column_types = _column_types;
    file_table_handler = _file_table_handler;
    mapping_data = _mapping_data;
}


ImportData::~ImportData(){
    delete column_types;
    delete mapping_data;
    delete file_table_handler;
}





KeyTableWidget::KeyTableWidget(QWidget *parent) : QTableWidget(parent){
}


void KeyTableWidget::keyPressEvent(QKeyEvent *event) {
    QTableWidget::keyPressEvent(event);
    emit keyPressed(event);
}




StudyVariableSet::StudyVariableSet(string _name, StudyVariableType v_type){
    name = _name;
    study_variable_type = v_type;
    numerical_filter = {NoFilter, vector<double>()};
}


StudyVariableSet::StudyVariableSet(){
    name = "";
    study_variable_type = NominalStudyVariable;
    numerical_filter = {NoFilter, vector<double>()};
}




StudyVariableSet::StudyVariableSet(StudyVariableSet *study_variable_set){
    name = study_variable_set->name;
    study_variable_type = study_variable_set->study_variable_type;
    for (auto kv : study_variable_set->nominal_values) nominal_values.insert({kv.first, kv.second});
    for (auto value : study_variable_set->numerical_values) numerical_values.insert(value);
    numerical_filter = {study_variable_set->numerical_filter.first, vector<double>()};
    for (auto value : study_variable_set->numerical_filter.second) numerical_filter.second.push_back(value);
}

StudyVariableSet::StudyVariableSet(json &container){
    for (string field : {"name", "study_variable_type"}){
        if (!contains_val(container, field)){
            throw LipidException("Error: field '" + field + "' not found in class 'StudyVariableSet'.");
        }
    }
    numerical_filter = {NoFilter, vector<double>()};

    name = (string)container["name"];
    study_variable_type = container["study_variable_type"];
    if (study_variable_type == NominalStudyVariable){

        if (!contains_val(container, "nominal_values")){
            throw LipidException("Error: field 'nominal_values' not found in class 'StudyVariableSet'.");
        }

        for (auto &kv : container["nominal_values"].items()){
            nominal_values.insert({(string)kv.key(), (bool)kv.value()});
        }
        numerical_filter = {NoFilter, vector<double>()};
    }
    else {
        if (contains_val(container, "numerical_values")){
            for (double val : container["numerical_values"]) numerical_values.insert(val);
        }


        if (contains_val(container, "numerical_filter")){
            if (contains_val(container["numerical_filter"], "key")) numerical_filter.first = container["numerical_filter"]["key"];

            if (contains_val(container["numerical_filter"], "value")){
                for (auto val : container["numerical_filter"]["value"]) numerical_filter.second.push_back(val);
            }
        }
    }
}



void StudyVariableSet::save(json &container){
    container["name"] = name;
    container["study_variable_type"] = study_variable_type;
    if (study_variable_type == NominalStudyVariable){
        for (auto &kv : nominal_values){
            container["nominal_values"][kv.first] = kv.second;
        }
    }
    else {
        for (double val : numerical_values) container["numerical_values"] += val;
        container["numerical_filter"]["key"] = numerical_filter.first;
        for (double val: numerical_filter.second) container["numerical_filter"]["value"] += val;
    }
}



StudyVariable::StudyVariable(){
    name = "";
    study_variable_type = NominalStudyVariable;
    numerical_value = 0;
    nominal_value = "";
    missing = false;
}


StudyVariable::StudyVariable (string _name, string nom_val, bool _missing){
    name = _name;
    study_variable_type = NominalStudyVariable;
    nominal_value = nom_val;
    numerical_value = 0;
    missing = _missing;
}


StudyVariable::StudyVariable (string _name, double num_val, bool _missing){
    name = _name;
    study_variable_type = NumericalStudyVariable;
    numerical_value = num_val;
    nominal_value = "";
    missing = _missing;
}

StudyVariable::StudyVariable (StudyVariable *f){
    name = f->name;
    study_variable_type = f->study_variable_type;
    numerical_value = f->numerical_value;
    nominal_value = f->nominal_value;
    missing = f->missing;
}

StudyVariable::StudyVariable (json &container){
    for (string field : {"name", "study_variable_type", "numerical_value", "nominal_value", "name"}){
        if (!contains_val(container, field)){
            throw LipidException("Error: field '" + field + "' not found in class 'StudyVariable'.");
        }
    }

    name = container["name"];
    study_variable_type = container["study_variable_type"];
    numerical_value = (double)container["numerical_value"];
    nominal_value = (string)container["nominal_value"];
    missing = (bool)container["missing"];
}



void StudyVariable::save(json &container){
    container["name"] = name;
    container["study_variable_type"] = study_variable_type;
    container["numerical_value"] = numerical_value;
    container["nominal_value"] = nominal_value;
    container["missing"] = missing;
}




ClickableLabel::ClickableLabel(QWidget* parent, Qt::WindowFlags) : QLabel(parent){

}


ClickableLabel::~ClickableLabel(){

}


void ClickableLabel::mouseDoubleClickEvent(QMouseEvent*) {
    emit doubleClicked();
}


void ClickableLabel::mousePressEvent(QMouseEvent*) {
    emit clicked();
}





SignalCombobox::SignalCombobox(QWidget *parent, int _row) : QComboBox(parent) {
    tool_tip = "";
    row = _row;
    connect(this, (void (SignalCombobox::*)(int))&SignalCombobox::currentIndexChanged, this, &SignalCombobox::changedIndex);
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void SignalCombobox::enterEvent(QEnterEvent *event) {
#else
void SignalCombobox::enterEvent(QEvent *event) {
#endif

    QComboBox::enterEvent(event);
    QToolTip::showText(QCursor::pos(), tool_tip);
}


void SignalCombobox::leaveEvent(QEvent *event){
    QComboBox::leaveEvent(event);
    QToolTip::hideText();
}

void SignalCombobox::changedIndex(int ){
    emit comboChanged(this);
}





SignalLineEdit::SignalLineEdit(int _row, StudyVariableType ft, QWidget *parent) : QLineEdit(parent){
    row = _row;
    study_variable_type = ft;
    connect(this, (void (SignalLineEdit::*)(const QString &))&SignalLineEdit::textChanged, this, &SignalLineEdit::changedText);
}


SignalLineEdit::SignalLineEdit(QWidget *parent) : QLineEdit(parent){
    row = 0;
    connect(this, (void (SignalLineEdit::*)(const QString &))&SignalLineEdit::textChanged, this, &SignalLineEdit::changedText);
}


void SignalLineEdit::changedText(const QString &txt){
    emit lineEditChanged(this, txt);
}





FileTableHandler::FileTableHandler(string file_name, string sheet_name){
    // csv import
    if (sheet_name.length() == 0){
        ifstream infile(file_name);
        if (!infile.good()){
            throw LipidSpaceException("Error: file '" + file_name + "' could not be found.", FileUnreadable);
        }
        int line_num = 0;
        string line;
        while (getline(infile, line)){
            if (line.length() == 0) continue;

            vector<string>* tokens = split_string(line, ',', '"', true);
            if (line_num++ == 0){
                for (int i = 0; i < (int)tokens->size(); ++i) headers.push_back(goslin::strip(tokens->at(i), '"'));
            }
            else {
                if (tokens->size() != headers.size()){
                    delete tokens;
                    throw LipidSpaceException("Error: file '" + file_name + "' has a different number of cells in line " + std::to_string(line_num) + " than in the header line.", ColumnNumMismatch);
                }
                rows.push_back(vector<string>());
                vector<string> &row = rows.back();
                for (int i = 0; i < (int)tokens->size(); ++i) row.push_back(goslin::strip(tokens->at(i), '"'));
            }

            delete tokens;
        }
    }
    else {
        XLDocument doc(file_name.c_str());

        auto wks = doc.workbook().worksheet(sheet_name.c_str());

        int line_num = 0;
        for (auto& wks_row : wks.rows()) {
            if (line_num++ == 0){
                for (auto cell : wks_row.cells()){
                    string value = "";
                    switch(cell.value().type()){
                        case OpenXLSX::XLValueType::Boolean:
                            value = (cell.value().get<bool>()) ? "true" : "false";
                            break;
                        case OpenXLSX::XLValueType::Integer:
                        case OpenXLSX::XLValueType::Float:
                            value = std::to_string(cell.value().get<double>());
                            break;
                        case OpenXLSX::XLValueType::String:
                            value = cell.value().get<string>();
                            break;
                        default: break;
                    }
                    headers.push_back(value);
                }
            }
            else {
                rows.push_back(vector<string>());
                vector<string> &row = rows.back();
                int empty_cells = 0;
                for (auto cell : wks_row.cells()){
                    string value = "";
                    switch(cell.value().type()){
                        case OpenXLSX::XLValueType::Boolean:
                            value = (cell.value().get<bool>()) ? "true" : "false";
                            break;
                        case OpenXLSX::XLValueType::Integer:
                            value = std::to_string(cell.value().get<int>());
                            break;
                        case OpenXLSX::XLValueType::Float:
                            value = std::to_string(cell.value().get<double>());
                            break;
                        case OpenXLSX::XLValueType::String:
                            value = cell.value().get<string>();
                            break;
                        default: break;
                    }
                    row.push_back(value);
                    empty_cells += (value == "");
                }
                if (row.size() == 0 || empty_cells == wks_row.cells().size()){
                    rows.pop_back();
                    break;
                }

                if (row.size() > headers.size()){
                    throw LipidSpaceException("Error: file '" + file_name + "' has a different number of cells (" + std::to_string(row.size()) + ") in line " + std::to_string(line_num) + " than in the header line (" + std::to_string(headers.size()) + ").", ColumnNumMismatch);
                }
                else if (row.size() < headers.size()){
                    row.resize(headers.size(), "");
                }
            }
        }
    }
}


Lipidome::Lipidome(string _lipidome_name, string lipidome_file, string sheet_name, bool is_file_name) : file_name(lipidome_file) {
    QFileInfo qFileInfo(file_name.c_str());
    string cleaned_file_name = qFileInfo.baseName().toStdString();
    lipidome_name = _lipidome_name;

    if (is_file_name){
        lipidome_name = cleaned_file_name;
        suffix = (sheet_name.length() > 0 ? "/" + sheet_name : "");
    }
    else {
        suffix = (cleaned_file_name.length() > 0) ? " - " + cleaned_file_name : "";
        if (suffix.length() > 0 && sheet_name.length() > 0) suffix += "/" + sheet_name;
    }
    cleaned_name = lipidome_name + suffix;
    study_variables.insert({FILE_STUDY_VARIABLE_NAME, StudyVariable(FILE_STUDY_VARIABLE_NAME, cleaned_file_name + (sheet_name.length() > 0 ?  "/" + sheet_name : ""))});
}




Lipidome::Lipidome(Lipidome *lipidome){
    file_name = lipidome->file_name;
    cleaned_name = lipidome->cleaned_name;
    lipidome_name = lipidome->lipidome_name;
    suffix = lipidome->suffix;
    for (auto value : lipidome->species) species.push_back(value);
    for (auto value : lipidome->classes) classes.push_back(value);
    for (auto value : lipidome->categories) categories.push_back(value);
    for (auto value : lipidome->visualization_intensities) visualization_intensities.push_back(value);
    for (auto value : lipidome->selected_lipid_indexes) selected_lipid_indexes.push_back(value);
    for (auto value : lipidome->normalized_intensities) normalized_intensities.push_back(value);
    for (auto value : lipidome->PCA_intensities) PCA_intensities.push_back(value);
    for (auto value : lipidome->original_intensities) original_intensities.push_back(value);
    for (auto kv : lipidome->study_variables) study_variables.insert({kv.first, StudyVariable(&kv.second)});
    m.rewrite(lipidome->m);
}


Lipidome::Lipidome (json &container, map<string, LipidAdduct*> &all_lipids){
    for (string field : {"file_name", "cleaned_name", "lipidome_name", "suffix", "species", "original_intensities", "study_variables"}){
        if (!contains_val(container, field)){
            throw LipidException("Error: field '" + field + "' not found in class 'Lipidome'.");
        }
    }


    file_name = (string)container["file_name"];
    cleaned_name = (string)container["cleaned_name"];
    lipidome_name = (string)container["lipidome_name"];
    suffix = (string)container["suffix"];
    for (string lipid_species : container["species"]){
        if (!contains_val(all_lipids, lipid_species)){
            throw LipidException("Error: lipid '" + lipid_species + "' in lipidome not registered in lipidome '" + cleaned_name + "'.");
        }
        LipidAdduct* l = all_lipids.at(lipid_species);
        lipids.push_back(l);
        species.push_back(lipid_species);
        classes.push_back(l->get_lipid_string(CLASS));
        categories.push_back(l->get_lipid_string(CATEGORY));
    }
    for (double val : container["original_intensities"]){
        original_intensities.push_back(val);
    }
    if (contains_val(container, "visualization_intensities")){
        for (double val : container["visualization_intensities"]){
            visualization_intensities.push_back(val);
        }
    }
    if (contains_val(container, "selected_lipid_indexes")){
        for (double val : container["selected_lipid_indexes"]){
            selected_lipid_indexes.push_back(val);
        }
    }
    if (contains_val(container, "normalized_intensities")){
        for (double val : container["normalized_intensities"]){
            normalized_intensities.push_back(val);
        }
    }
    if (contains_val(container, "PCA_intensities")){
        for (double val : container["PCA_intensities"]){
            PCA_intensities.push_back(val);
        }
    }

    for (auto &kv : container["study_variables"].items()){
        study_variables.insert({(string)kv.key(), StudyVariable(kv.value())});
    }

    m.load(container["m"]);
}


void Lipidome::save(json &container){
    container["file_name"] = file_name;
    container["cleaned_name"] = cleaned_name;
    container["lipidome_name"] = lipidome_name;
    container["suffix"] = suffix;

    for (auto lipid_species : species) container["species"] += lipid_species;
    for (auto &kv : study_variables){
        kv.second.save(container["study_variables"][kv.first]);
    }

    for (int val : selected_lipid_indexes) container["selected_lipid_indexes"] += val;
    for (double val : original_intensities) container["original_intensities"] += val;
    for (double val : visualization_intensities) container["visualization_intensities"] += val;
    for (double val : normalized_intensities) container["normalized_intensities"] += val;
    for (double val : PCA_intensities) container["PCA_intensities"] += val;

    m.save(container["m"]);

}




string Lipidome::to_json(map<string, string> *imported_lipid_names){
    stringstream s;

    s << "{\"LipidomeName\": \"" << replace_all(cleaned_name, "\"", "") << "\", ";

    s << "\"LipidNames\": [";
    for (uint l = 0; l < species.size(); ++l) {
        if (l) s << ", ";
        s << "\"" << replace_all(species.at(l), "\"", "") << "\"";
    }
    s << "], ";


    if (imported_lipid_names){
        s << "\"ImportedLipidNames\": [";
        for (uint l = 0; l < species.size(); ++l) {
            if (l) s << ", ";
            if (contains_val(*imported_lipid_names, species.at(l))){
                s << "\"" << replace_all(imported_lipid_names->at(species.at(l)), "\"", "") << "\"";
            }
            else {
                s << "\"unknown\"";
            }
        }
        s << "], ";
    }

    s << "\"Intensities\": [";
    for (uint l = 0; l < normalized_intensities.size(); ++l) {
        if (l) s << ", ";
        s << normalized_intensities.at(l);
    }
    s << "], ";

    s << "\"X\": [";
    for (int r = 0; r < m.rows; ++r) {
        if (r) s << ", ";
        s << m(r, 0);
    }
    s << "], ";

    s << "\"Y\": [";
    for (int r = 0; r < m.rows; ++r) {
        if (r) s << ", ";
        s << m(r, 1);
    }
    s << "]";

    s << "}";
    return s.str();
}





DendrogramNode::DendrogramNode(int index, map<string, StudyVariableSet> *study_variable_values, Lipidome *lipidome){
    indexes.insert(index);
    order = -1;
    left_child = 0;
    right_child = 0;
    distance = 0;
    x_left = 0;
    x_right = 0;
    y = 0;
    min_distance = {INFINITY, 0};
    depth = 0;


    // initialize empty study variable count table
    for (auto kv : *study_variable_values){
        if (kv.second.study_variable_type == NominalStudyVariable){
            study_variable_count_nominal.insert({kv.first, map<string, int>()});
            for (auto kv_nom : kv.second.nominal_values){
                study_variable_count_nominal.at(kv.first).insert({kv_nom.first, 0});
            }
        }
        else {
            study_variable_numerical.insert({kv.first, vector<double>()});
        }
    }

    for (auto kv : lipidome->study_variables){
        if (kv.second.study_variable_type == NominalStudyVariable){
            study_variable_count_nominal.at(kv.first).at(kv.second.nominal_value) = 1;
        }
        else {
            study_variable_numerical.at(kv.first).push_back(kv.second.numerical_value);
        }
    }
}




DendrogramNode::~DendrogramNode(){
    if (left_child) delete left_child;
    if (right_child) delete right_child;
}



DendrogramNode::DendrogramNode(DendrogramNode* n1, DendrogramNode* n2, double d){
    left_child = n1;
    right_child = n2;
    min_distance = {INFINITY, 0};
    for (auto i : n1->indexes) indexes.insert(i);
    for (auto i : n2->indexes) indexes.insert(i);
    distance = d;
    depth = 1 + max(n1->depth, n2->depth);
}


DendrogramNode::DendrogramNode(json &container){
    for (string field : {"indexes", "order", "left_child", "right_child", "distance", "x_left", "x_right", "y", "depth"}){
        if (!contains_val(container, field)){
            throw LipidException("Error: field '" + field + "' not found in class 'DendrogramNode'.");
        }
    }

    if (contains_val(container, "indexes")){
        for (auto val : container["indexes"]) indexes.insert((int)val);
    }

    order = container["order"];
    distance = container["distance"];
    x_left = container["x_left"];
    x_right = container["x_right"];
    y = container["y"];
    depth = container["depth"];

    left_child = container["left_child"].is_number() ? 0 : new DendrogramNode(container["left_child"]);
    right_child = container["right_child"].is_number() ? 0 : new DendrogramNode(container["right_child"]);

    if (contains_val(container, "study_variable_count_nominal")){
        for (auto &kv : container["study_variable_count_nominal"].items()){
            study_variable_count_nominal.insert({(string)kv.key(), map<string, int>()});
            map<string, int> &m = study_variable_count_nominal.at(kv.key());
            for (auto &kv2 : kv.value().items()) m.insert({(string)kv2.key(), (int)kv2.value()});
        }
    }

    if (contains_val(container, "study_variable_numerical")){
        for (auto &kv : container["study_variable_numerical"].items()){
            study_variable_numerical.insert({(string)kv.key(), vector<double>()});
            vector<double> &v = study_variable_numerical.at(kv.key());
            for (double val : kv.value()) v.push_back(val);
        }
    }

    if (contains_val(container, "study_variable_numerical_thresholds")){
        for (auto &kv : container["study_variable_numerical_thresholds"].items()){
            study_variable_numerical_thresholds.insert({(string)kv.key(), (double)kv.value()});
        }
    }
}


void DendrogramNode::save(json &container){
    for (auto val : indexes) container["indexes"] += val;

    container["order"] = order;
    container["distance"] = distance;
    container["x_left"] = x_left;
    container["x_right"] = x_right;
    container["y"] = y;
    container["depth"] = depth;

    if (left_child) left_child->save(container["left_child"]);
    else container["left_child"] = 0;

    if (right_child) right_child->save(container["right_child"]);
    else container["right_child"] = 0;

    for (auto &kv : study_variable_count_nominal){
        for (auto &kv2 : kv.second){
            container["study_variable_count_nominal"][kv.first][kv2.first] = kv2.second;
        }
    }

    for (auto &kv : study_variable_numerical){
        for (double val : kv.second) container["study_variable_numerical"][kv.first] += val;
    }

    for (auto &kv : study_variable_numerical_thresholds){
        container["study_variable_numerical_thresholds"][kv.first] = kv.second;
    }
}



double* DendrogramNode::execute(int cnt, Array* points, vector<int>* sorted_ticks){
    if (left_child == 0){
        order = cnt;
        sorted_ticks->push_back(*indexes.begin());
        return new double[3]{(double)cnt, 0, (double)cnt + 1};
    }

    double* left_result = left_child->execute(cnt, points, sorted_ticks);
    x_left = left_result[0];
    double yl = left_result[1];
    cnt = left_result[2];
    delete []left_result;

    double* right_result = right_child->execute(cnt, points, sorted_ticks);
    x_right = right_result[0];
    double yr = right_result[1];
    cnt = right_result[2];
    delete []right_result;

    y = distance;

    points->push_back(x_left);
    points->push_back(yl);
    points->push_back(x_left);
    points->push_back(y);

    points->push_back(x_right);
    points->push_back(yr);
    points->push_back(x_right);
    points->push_back(y);

    points->push_back(x_left);
    points->push_back(y);
    points->push_back(x_right);
    points->push_back(y);

    // count study variables
    for (auto kv : left_child->study_variable_count_nominal){
        study_variable_count_nominal.insert({kv.first, map<string, int>()});
        for (auto kv2 : kv.second){
            study_variable_count_nominal.at(kv.first).insert({kv2.first, kv2.second + right_child->study_variable_count_nominal.at(kv.first).at(kv2.first)});
        }
    }

    for (auto kv : left_child->study_variable_numerical){
        // find intersection points for numerical study variables
        vector<double> &set1 = kv.second;
        vector<double> &set2 = right_child->study_variable_numerical.at(kv.first);

        double pos_max = 0, d = 0;
        ks_separation_value(set1, set2, d, pos_max);
        study_variable_numerical_thresholds.insert({kv.first, pos_max});

        study_variable_numerical.insert({kv.first, vector<double>()});
        for(double val : kv.second){
            study_variable_numerical.at(kv.first).push_back(val);
        }
        for(double val : right_child->study_variable_numerical.at(kv.first)){
            study_variable_numerical.at(kv.first).push_back(val);
        }
    }
    return new double[3]{(x_left + x_right) / 2., y, (double)cnt};
}



int DendrogramNode::get_linkages(vector<vector<double>> &linkages, int &index){
    if (indexes.size() == 1){
        return (int)(*indexes.begin());
    }

    int left_index = left_child->get_linkages(linkages, index);
    int right_index = right_child->get_linkages(linkages, index);

    linkages.push_back({(double)left_index, (double)right_index, y, (double)indexes.size()});
    return index++;
}




void DendrogramNode::update_distances(set<DendrogramNode*> &nodes, Matrix &m){
    min_distance = {INFINITY, 0};
    for (auto node : nodes){
        if (node == this) continue;

        double dist = INFINITY;
        if (contains_val(distances, node)){
            dist = distances.at(node);
        }
        else {
            Linkage lnk = GlobalData::linkage;
            dist = (lnk == SingleLinkage) ? INFINITY : 0;
            if (lnk == AverageLinkage) {
                for (auto index1 : indexes){
                    for (auto index2 : node->indexes){
                        dist += m(index1, index2);
                    }
                }
                dist /= (double)(indexes.size() * node->indexes.size());
            }
            else {
                for (auto index1 : indexes){
                    for (auto index2 : node->indexes){
                        double hd = m(index1, index2);
                        if ((lnk == CompleteLinkage && dist < hd) || (lnk == SingleLinkage && (dist == INFINITY || dist > hd))){
                            dist = hd;
                        }
                    }
                }
            }
            distances.insert({node, dist});
            if (uncontains_val(node->distances, this)) node->distances.insert({this, dist});
            else node->distances.at(this) = dist;
        }

        if (min_distance.first == INFINITY || dist < min_distance.first || (dist == min_distance.first && node->depth < min_distance.second->depth)){
            min_distance = {dist, node};
        }
    }
}



double compute_accuracy(vector<Array> &arrays){
    if (arrays.size() < 2) return 1;

    // remove empty arrays
    for (int i = (int)arrays.size() - 1; i >= 0; --i){
        if (arrays.at(i).size() == 0){
            arrays.erase(arrays.begin() + i);
        }
    }
    if (arrays.size() < 2) return 1;


    set<uint> indexes;
    Indexes medians;
    double all = 0;
    for (uint i = 0; i < arrays.size(); ++i){
        auto &array = arrays.at(i);
        sort(array.begin(), array.end());
        medians.push_back(array.median(-1, -1, true));
        indexes.insert(i);
        all += array.size();
    }
    // compute all separation positions for each pair of set
    vector< vector<double> > border_matrix(arrays.size(), vector<double>(arrays.size(), 0));
    double d = 0;
    for (uint i = 0; i < arrays.size() - 1; ++i){
        for (uint j = i + 1; j < arrays.size(); ++j){
            ks_separation_value(arrays.at(i), arrays.at(j), d, border_matrix.at(i).at(j));
        }
    }

    // define a list of all boundaries for each category
    Array borders;
    Indexes array_order;
    borders.push_back(-INFINITY);
    while (indexes.size() > 1){
        uint min_i = 0;
        uint min_j = 0;
        double min_value = INFINITY;

        for (auto i : indexes){
            for (auto j : indexes){
                if (i >= j) continue;
                if (min_value == INFINITY || min_value > border_matrix.at(i).at(j)) {
                    min_i = i;
                    min_j = j;
                    min_value = border_matrix.at(i).at(j);
                }
            }
        }

        borders.push_back(min_value);
        if (medians.at(min_i) < medians.at(min_j)){
            indexes.erase(min_i);
            array_order.push_back(min_i);
        }
        else {
            indexes.erase(min_j);
            array_order.push_back(min_j);
        }
    }
    array_order.push_back(*indexes.begin());
    borders.push_back(INFINITY);

    // count all true hits
    double true_hits = 0;
    for (uint i = 0; i < borders.size() - 1; ++i){
        double border_lower = borders.at(i);
        double border_upper = borders.at(i + 1);
        int left = arrays.at(array_order.at(i)).greatest_less(border_lower) + 1;
        int right = arrays.at(array_order.at(i)).greatest_less(border_upper);
        true_hits += right - left + 1;
    }

    return true_hits / all;
}



void ks_separation_value(vector<double> &a, vector<double> &b, double &d, double &pos_max, pair<vector<double>, vector<double>> *ROC){
    d = 0;
    pos_max = 0;
    int num1 = a.size();
    int num2 = b.size();
    sort(a.begin(), a.end());
    sort(b.begin(), b.end());
    double inv_m = 1. / (double)num1, inv_n = 1. / (double)num2;
    double ptr1 = 0, ptr2 = 0;
    double cdf1 = 0, cdf2 = 0;

    while ((ptr1 < num1) && (ptr2 < num2)){
        if (ROC){
            ROC->first.push_back((num1 - ptr1) * inv_m);
            ROC->second.push_back((num2 - ptr2) * inv_n);
        }
        if (a.at(ptr1) <= b.at(ptr2)){
            cdf1 += inv_m;
            if (d < __abs(cdf1 - cdf2)){
                d = __abs(cdf1 - cdf2);
                pos_max = a.at(ptr1);
            }
            ptr1 += 1;
        }
        else {
            cdf2 += inv_n;
            if (d < __abs(cdf1 - cdf2)){
                d = __abs(cdf1 - cdf2);
                pos_max = b.at(ptr2);
            }
            ptr2 += 1;
        }
    }
}





//////////////////////////////////////////////////////////////////////////
// MIT License
// Copyright (c) 2016 Mengchang Wang
// libAnova
// https://github.com/Amuwa/libAnova
//////////////////////////////////////////////////////////////////////////


double f_distribution_cdf(double F, double m, double n){
    double xx = 0.0, p = 0.0;

    if (m <= 0. || n <= 0.) p = -1.;
    else if (F > 0){
        xx = F / (F + n / m);
        p = betainc(xx, m * 0.5, n * 0.5, true);
    }

    return p;
}



double betainc(double x,double a, double b, bool flip_value) {
    double y, BT, AAA;

    if(x == 0. || x == 1.){
        BT = 0.;
    }
    else {
        AAA = lgamma(a + b) - lgamma(a) - lgamma(b);
        BT = exp(AAA + a * log(x) + b * log(1. - x));
    }
    if(x < (a + 1.) / (a + b + 2.)){
        if (flip_value) y = 1. - BT * beta_cf(a, b, x) / a;
        else y = BT * beta_cf(a, b, x) / a;
    }
    else {
        if (flip_value) y = BT * beta_cf(b, a, 1. - x) / b;
        else y = 1. - BT * beta_cf(b, a, 1. - x) / b;
    }

    return y;
}

double beta_cf(double a,double b,double x) {
    const int count_max = 100;
    const double eps = 1e-7;
    double AM = 1.;
    double BM = 1.;
    double AZ = 1.;
    double EM, TEM, D, AP, BP, AAP, BPP, AOLD;

    double QAB = a + b;
    double QAP = a + 1.;
    double QAM = a - 1.;
    double BZ = 1. - QAB * x / QAP;

    for(int count = 1; count <= count_max; count++){
        EM = count;
        TEM = EM + EM;
        D = EM * (b - count) * x / ((QAM + TEM) * (a + TEM));
        AP = AZ + D * AM;
        BP = BZ + D * BM;
        D = -(a + EM) * (QAB + EM) * x / ((a + TEM) * (QAP + TEM));
        AAP = AP + D * AZ;
        BPP = BP + D * BZ;
        AOLD = AZ;
        AM = AP / BPP;
        BM = BP / BPP;
        AZ = AAP / BPP;
        BZ = 1.;
        if(fabs(AZ - AOLD) < eps * fabs(AZ)) return AZ;
    }
    return AZ;
}






/*
Source: https://en.wikipedia.org/wiki/Student%27s_t-distribution#Cumulative_distribution_function
*/
double t_distribution_cdf(double t_stat, double free_deg){
    double tt = free_deg / (sq(t_stat) + free_deg);
    return max(0., min(1., betainc(tt, free_deg * 0.5, 0.5)));
}


/*
Source: https://en.wikipedia.org/wiki/Welch%27s_t-test
*/
double p_value_welch(Array &a, Array &b){
    double m1 = a.mean();
    double s1 = a.sample_stdev();
    double n = a.size();
    double m2 = b.mean();
    double s2 = b.sample_stdev();
    double m = b.size();

    double w0 = (m1 - m2);
    double t = w0 / sqrt(sq(s1 / sqrt(n)) + sq(s2 / sqrt(m)));
    double v = sq(sq(s1) / n + sq(s2) / m) / (sq(sq(s1)) / (sq(n) * (n - 1)) + sq(sq(s2)) / (sq(m) * (m - 1)));

    return max(0., min(1., t_distribution_cdf(t, v)));
}




/*
Source: https://www.geeksforgeeks.org/python-pearsons-chi-square-test/
*/

double p_value_chi_sq(Array &a, Array &b){
    if (a.size() != b.size()) return -1;
    int n = a.size();

    /*
    Array rt(n, 0);
    Array ct(2, 0);
    double global_sum = 0;
    double free_deg = (n - 1) * (2 - 1);
    for (int r = 0; r < n; ++r){
        ct[0] += a[r];
        ct[1] += b[r];
        rt[r] += a[r] + b[r];
        global_sum += a[r] + b[r];
    }

    Array estimated_a(n, 0);
    Array estimated_b(n, 0);

    for (int r = 0; r < n; ++r){
        estimated_a[r] = ct[0] * rt[r] / global_sum;
        estimated_b[r] = ct[1] * rt[r] / global_sum;
    }

    double x2 = 0;
    for (int r = 0; r < n; ++r){
        x2 += sq(a[r] - estimated_a[r]) / estimated_a[r];
        x2 += sq(b[r] - estimated_b[r]) / estimated_b[r];
    }


    free_deg /= 2.;
    x2 /= 2;

    */

    double x2 = 0;
    for (int r = 0; r < n; ++r){
        x2 += sq(a.at(r) - b.at(r)) / b.at(r);
    }


    double free_deg = ((double)n - 1) / 2.;
    x2 /= 2;
    double pval = 0;
    for (double k = 0; k < 100; ++k){
        pval += exp(k * log(x2) - lgamma(k + free_deg + 1.));
    }
    pval *= pow(x2, free_deg) * exp(-x2);

    return 1. - pval;
}



double cosine_similarity(Array &a, Array &b){
    if (a.size() != b.size()) return -1;

    double cs = 0, tmp_a = 0, tmp_b = 0;
    for (uint i = 0; i < a.size(); ++i){
        cs += a.at(i) * b.at(i);
        tmp_a += sq(a.at(i));
        tmp_b += sq(b.at(i));
    }
    return cs / (sqrt(tmp_a) * sqrt(tmp_b));
}



bool test_benford(Array &a){
    Array benford;
    benford.push_back(0.30102999566398114);
    benford.push_back(0.17609125905568124);
    benford.push_back(0.1249387366082999);
    benford.push_back(0.0969100130080564);
    benford.push_back(0.0791812460476248);
    benford.push_back(0.06694678963061322);
    benford.push_back(0.057991946977686726);
    benford.push_back(0.051152522447381284);
    benford.push_back(0.04575749056067514);

    double total_sum = 0;
    Array leading_digits(10, 0);
    for (auto val : a){
        if (val <= 0) continue;
        leading_digits.at((int)(val / pow(10,(floor(log(val) / log(10))))))++;
        total_sum += 1;
    }
    leading_digits.erase(leading_digits.begin());

    double d = 0;
    for (uint i = 0; i < benford.size(); ++i){
        if (i > 0 && leading_digits.at(i - 1) < leading_digits.at(i)) return false;
        d += sq(benford.at(i) - leading_digits.at(i) / total_sum);
    }

    return sqrt(d) / 21.04527783158751 <= BENFORD_THRESHOLD;
}



bool test_benford(Matrix &m){
    Array benford;
    benford.push_back(0.30102999566398114);
    benford.push_back(0.17609125905568124);
    benford.push_back(0.1249387366082999);
    benford.push_back(0.0969100130080564);
    benford.push_back(0.0791812460476248);
    benford.push_back(0.06694678963061322);
    benford.push_back(0.057991946977686726);
    benford.push_back(0.051152522447381284);
    benford.push_back(0.04575749056067514);

    double total_sum = 0;
    Array leading_digits(10, 0);

    for (int c = 0; c < m.cols; c++){
        for (int r = 0; r < m.rows; ++r){
            double val = m(r, c);
            if (val <= 0) continue;
            leading_digits.at((int)(val / pow(10, floor(log(val) / log(10)))))++;
            total_sum += 1;
        }
    }
    leading_digits.erase(leading_digits.begin());

    double d = 0;
    for (uint i = 0; i < benford.size(); ++i){
        if (i > 0 && leading_digits.at(i - 1) < leading_digits.at(i)) return false;
        d += sq(benford.at(i) - leading_digits.at(i) / total_sum);
    }

    return sqrt(d) / 21.04527783158751 <= BENFORD_THRESHOLD;
}



bool test_benford(vector<Lipidome*> &l){
    Array benford;
    benford.push_back(0.30102999566398114);
    benford.push_back(0.17609125905568124);
    benford.push_back(0.1249387366082999);
    benford.push_back(0.0969100130080564);
    benford.push_back(0.0791812460476248);
    benford.push_back(0.06694678963061322);
    benford.push_back(0.057991946977686726);
    benford.push_back(0.051152522447381284);
    benford.push_back(0.04575749056067514);

    double total_sum = 0;
    Array leading_digits(10, 0);

    for (auto lipidome : l){
        for (auto val : lipidome->original_intensities){
            if (val <= 0) continue;
            leading_digits.at((int)(val / pow(10, floor(log(val) / log(10)))))++;
            total_sum += 1;
        }
    }
    leading_digits.erase(leading_digits.begin());

    double d = 0;
    for (uint i = 0; i < benford.size(); ++i){
        if (i > 0 && leading_digits.at(i - 1) < leading_digits.at(i)) return false;
        d += sq((benford.at(i) - leading_digits.at(i) / total_sum) / benford.at(i));
    }

    return sqrt(d) / 21.04527783158751 <= BENFORD_THRESHOLD;
}




void multiple_correction_bonferoni(Array &a){
    for (auto &v : a) v = min(1., v * a.size());
}




void multiple_correction_bh(Array &arr){
    vector<pair<double, double>> data;
    for (int i = 0; i < (int)arr.size(); ++i) data.push_back({arr.at(i), i});

    sort(data.begin(), data.end(),
    [](const pair<double, double> &a, const pair<double, double> &b) -> bool {
        return a.first < b.first;
    });
    for (int i = data.size() - 2; i >= 0; i--){
        data.at(i).first = min(data.at(i + 1).first, data.at(i).first * data.size() / (i + 1));
    }
    for (auto &kv : data){
        arr.at(kv.second) = kv.first;
    }
}





/*
Source: https://en.wikipedia.org/wiki/Student%27s_t-test
*/
double p_value_student(Array &sample1, Array &sample2){
    double m1 = sample1.mean();
    double s1 = sample1.sample_stdev();
    double n = sample1.size();
    double m2 = sample2.mean();
    double s2 = sample2.sample_stdev();
    double m = sample2.size();

    double s = sqrt(((n - 1) * sq(s1) + (m - 1) * sq(s2)) / (n + m - 2));
    double t = __abs(sqrt(n * m / (n + m)) * ((m1 - m2) / s));
    return max(0., min(1., t_distribution_cdf(t, n + m - 2.)));
}

inline double logbinom(double n, double k) noexcept
{
    return lgamma(n + 1) - lgamma(n - k + 1) - lgamma(k + 1);
}

inline double binom(double n, double k) noexcept
{
    return exp(logbinom(n,k));
}


double hypergeometricProb(int a, int b, int c, int d){
    return exp(logbinom(a + c, a) + logbinom(b + d, b) - logbinom(a + b + c + d, a + b));
}



double exact_fischer(int num_bg, int num_event, int num_target, int num_target_event){
    int a = num_target_event;
    int b = num_event - num_target_event;
    int c = num_target - num_target_event;
    int d = num_bg - num_event - num_target + num_target_event;
    double pCutoff = hypergeometricProb(a, b, c, d);
    double pValue = 0;
    for (int x = 0; x <= min(num_event, num_target); ++x){
        if (a + b - x >= 0 && a + c - x >= 0 && d - a + x >= 0){
            double p = hypergeometricProb(x, a + b - x, a + c - x, d - a + x);
            if (p <= pCutoff) pValue += p;
        }
    }
    return max(0., min(1., pValue));
}




/*
Algorithm adapted from: arXiv:2102.08037
Thomas Viehmann
Numerically more stable computation of the p-values for the two-sample Kolmogorov-Smirnov test
*/
double p_value_kolmogorov_smirnov(Array &sample1, Array &sample2){
    double d = 0, cdf1 = 0, cdf2 = 0;
    int ptr1 = 0, ptr2 = 0, m = sample1.size(), n = sample2.size();
    sort (sample1.begin(), sample1.end());
    sort (sample2.begin(), sample2.end());
    double inv_m = 1. / (double)m;
    double inv_n = 1. / (double)n;

    while (ptr1 < m && ptr2 < n){
        if (sample1.at(ptr1) < sample2.at(ptr2)){
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
double p_value_anova(vector<Array> &arrays){
    Array sums;
    Array means;
    double I = arrays.size();
    double N = 0;
    double total_sum = 0;
    double MSB = 0;
    double MSW = 0;
    for (auto a : arrays){
        sums.push_back(a.sum());
        total_sum += sums.back();

        means.push_back(a.mean());
        N += a.size();

        MSB += sums.back() * means.back();
    }
    for (uint i = 0; i < arrays.size(); ++i){
        auto a = arrays.at(i);
        double m = means.at(i);
        for (auto v : a){
            MSW += sq(v - m);
        }
    }

    MSB -= sq(total_sum) / N;

    N -= I;
    I -= 1;

    double F = MSB * N / (MSW * I);
    return max(0., min(1., f_distribution_cdf(F, I, N)));
}




Progress::Progress(){
    current_progress = 0;
    max_progress = 0;
    stop_progress = false;
    connected = false;
    step_size = 0;
    process_type = NoAnalysis;
}



void Progress::increment(){
    current_progress += 1;
    emit set_current(current_progress);
}


void Progress::setError(QString interrupt_message){
    stop_progress = true;
    emit error(interrupt_message);
}


void Progress::interrupt(){
    stop_progress = true;
}



void Progress::reset(ProcessType _process_type){
    process_type = _process_type;
    stop_progress = false;
    current_progress = 0;
}



void Progress::prepare(int max_val){
    max_progress = max_val;
    current_progress = 0;
    emit set_max(max_progress);
}



double compute_aic(Matrix &data, Array &coefficiants, Array &values){
    Array S;
    S.mult(data, coefficiants);
    double s = 0;
    for (int i = 0; i < (int)S.size(); ++i) s += sq(S.at(i) - values.at(i));
    if (s == 0) return s;

    int k = data.cols;
    int n = data.rows;
    return n * (log(2. * M_PI) + 1. + log(s / n)) + (k * 2.);
}




bool gene_aic(Gene g1, Gene g2){
    return g1.score < g2.score;
}


ListItem::ListItem(string show_name, ListItemType t, QListWidget* parent, string _system_name) : QListWidgetItem(show_name.c_str(), parent) {
    system_name = _system_name.length() > 0 ? _system_name : show_name;
    type = t;
    length = 0;
}




ItemDelegate::ItemDelegate(QObject *parent) : QItemDelegate(parent) {

}



void ItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {

    painter->setPen(QPen(Qt::NoPen));
    int rr = index.row();
    double l = ((ListItem*)((QListWidget*)parent())->item(rr))->length;
    l = max(0., min(1., l));

    if (l > 1e-15) {
        painter->setBrush(QBrush(QColor(212, 240, 255, 80)));
        QRect r = option.rect;
        r.setWidth(r.width() * l - 2);
        painter->drawRect(r);
    }
    QItemDelegate::paint(painter, option, index);
}


TreeItem::TreeItem(int pos, QString name, string var, QTreeWidgetItem* parent) : QTreeWidgetItem(parent){
    setText(pos, name);
    study_variable = var;
}



TreeItem::TreeItem(int pos, QString name, QTreeWidget* parent) : QTreeWidgetItem(parent){
    setText(pos, name);
    study_variable = "";
}


Gene::Gene(int features){
    score = -1;
    gene_code.resize(features, false);
}


Gene::Gene(Gene *gene){
    score = gene->score;
    for (bool feature : gene->gene_code) gene_code.push_back(feature);
}

Gene::Gene(Gene *g1, Gene *g2, double mutation_rate){
    score = -1;
    for (int i = 0; i < (int)g1->gene_code.size(); ++i){
        bool feature = randnum() < 0.5 ? g1->gene_code.at(i) : g2->gene_code.at(i);
        if (randnum() < mutation_rate) feature = !feature;
        gene_code.push_back(feature);
    }
}

void Gene::get_indexes(Indexes &indexes){
    indexes.clear();
    for (int i = 0; i < (int)gene_code.size(); ++i){
        if (gene_code.at(i)) indexes.push_back(i);
    }
}
