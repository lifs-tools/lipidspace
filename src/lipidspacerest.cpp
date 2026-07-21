#include "httplib.h"
#include <atomic>
#include <chrono>
#include <thread>
#include <QLoggingCategory>
#include <qlogging.h>
#include <QtGlobal>
#include <QJsonDocument>
#include <QByteArray>
#include <QFile>
#include <QtCore>
#include <QDirIterator>
#include <QStorageInfo>
#include <vector>
#include <fstream>
#include "cblas.h"
#include "cppgoslin/cppgoslin.h"
#include "lipidspace/AssistanceFunctions.h"
#include "lipidspace/globaldata.h"
#include "lipidspace/lipidspace.h"
#include "lipidspace/atlas.h"
#include "lipidspace/logging.h"

using namespace std;
using namespace httplib;

vector<string> dict_keys{"TableType", "TableColumnTypes", "Table"};


// Load a LipidSpace from a REST table spec (shared by /atlas/build and /atlas/fit).
// Writes the table to dirPath, imports and parses it, and optionally runs the full
// analysis (needed to freeze a frame; fit only needs the parsed query lipids).
// Returns true on success; on failure sets res and returns false.
static bool atlas_load_table(LipidSpace &ls, json &body, const QString &dirPath,
                             bool do_run_analysis, Response &res) {
    for (const char *key : {"TableType", "TableColumnTypes", "Table"}) {
        if (!body.contains(key)) {
            res.status = 400;
            res.reason = string("Malformed JSON, missing key '") + key + "'";
            return false;
        }
    }
    if (!body["TableType"].is_string() || !body["TableColumnTypes"].is_array() || !body["Table"].is_string()) {
        res.status = 400;
        res.reason = "Malformed JSON, invalid table field types";
        return false;
    }
    string tt = body["TableType"].get<string>();
    if (TableTypeMap.find(tt) == TableTypeMap.end()) {
        res.status = 400;
        res.reason = "Malformed JSON, invalid TableType '" + tt + "'";
        return false;
    }
    TableType table_type = TableTypeMap.at(tt);

    vector<TableColumnType> *column_types = new vector<TableColumnType>();
    for (auto &v : body["TableColumnTypes"]) {
        if (!v.is_string() || TableColumnTypeMap.find(v.get<string>()) == TableColumnTypeMap.end()) {
            delete column_types;
            res.status = 400;
            res.reason = "Malformed JSON, invalid TableColumnTypes entry";
            return false;
        }
        column_types->push_back(TableColumnTypeMap.at(v.get<string>()));
    }

    QString table_file_name = dirPath + "/table_file.csv";
    {
        ofstream table_file(table_file_name.toStdString().c_str());
        table_file << body["Table"].get<string>();
        table_file.flush();
    }

    ls.ignore_unknown_lipids = true;
    ls.ignore_doublette_lipids = true;

    ImportData *import_data = 0;
    try {
        import_data = new ImportData(table_file_name.toStdString(), "", table_type, column_types);
    } catch (LipidSpaceException &e) {
        res.status = 400; res.reason = string("ImportData error: ") + e.what(); return false;
    } catch (std::exception &e) {
        res.status = 500; res.reason = string("ImportData error: ") + e.what(); return false;
    }
    try {
        ls.load_table(import_data);
    } catch (LipidSpaceException &e) {
        delete import_data; res.status = 400; res.reason = string("load_table error: ") + e.what(); return false;
    } catch (std::exception &e) {
        delete import_data; res.status = 500; res.reason = string("load_table error: ") + e.what(); return false;
    }
    delete import_data;

    if (do_run_analysis) {
        try {
            ls.run_analysis();
        } catch (LipidSpaceException &e) {
            res.status = 400; res.reason = string("analysis error: ") + e.what(); return false;
        } catch (std::exception &e) {
            res.status = 500; res.reason = string("analysis error: ") + e.what(); return false;
        }
    }
    return true;
}

static qint64 dirSize(const QString &path)
{
    qint64 total = 0;
    QDirIterator it(path, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        it.next();
        if (it.fileInfo().isFile())
            total += it.fileInfo().size();
    }
    return total;
}

class LipidSpaceRest
{
public:
    Server svr;
    thread t;
    thread vacuum_t;
    atomic<bool> vacuum_running{false};

    void vacuum_loop()
    {
        while (vacuum_running.load()) {
            for (int i = 0; i < GlobalData::rest_vacuum_interval_secs * 10 && vacuum_running.load(); ++i)
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            if (!vacuum_running.load()) break;

            QString base = QString::fromStdString(GlobalData::rest_temp_folder);
            QFileInfoList entries = QDir(base).entryInfoList(
                QDir::Dirs | QDir::NoDotAndDotDot,
                QDir::Time | QDir::Reversed); // oldest first

            QDateTime now = QDateTime::currentDateTimeUtc();

            for (const QFileInfo &entry : entries) {
                if (!vacuum_running.load()) break;

                qint64 age = entry.lastModified().toUTC().secsTo(now);
                bool expired = age >= GlobalData::rest_vacuum_max_age_secs;
                bool active  = age <  GlobalData::rest_vacuum_min_active_secs;

                QStorageInfo storage(base);
                bool diskLow = storage.isValid()
                            && storage.bytesFree() < GlobalData::rest_disk_threshold_bytes;

                if (expired || (diskLow && !active)) {
                    const char *reason = expired ? "max-age" : "disk-pressure";
                    if (GlobalData::debug) {
                        QDateTime created  = entry.birthTime().toUTC();
                        QDateTime modified = entry.lastModified().toUTC();
                        qint64 size = dirSize(entry.absoluteFilePath());
                        qInfo("Vacuum: removing dir name='%s' created='%s' modified='%s' size=%lld bytes reason=%s",
                              qPrintable(entry.fileName()),
                              qPrintable(created.isValid() ? created.toString(Qt::ISODate) : QString("N/A")),
                              qPrintable(modified.toString(Qt::ISODate)),
                              static_cast<long long>(size),
                              reason);
                    } else {
                        qInfo("Vacuum: removing dir '%s' reason=%s", qPrintable(entry.fileName()), reason);
                    }
                    QDir(entry.absoluteFilePath()).removeRecursively();
                }
            }
        }
    }

    inline int start(string host, int port, string temp_folder, bool debug)
    {
#ifdef OPENBLAS_OS_LINUX 
        qInfo("Using %d threads with openBLAS!", openblas_get_num_threads());
#endif
        GlobalData::rest_temp_folder = temp_folder;
        GlobalData::debug = debug;
        // stop(SIGINT);
        qInfo("Starting LipidSpaceRest server version='%s' on host='%s' port='%d' tmp_folder='%s' debug='%s'", GlobalData::LipidSpace_version.c_str(), host.c_str(), port, temp_folder.c_str(), debug ? "true" : "false");
        // register handlers

        svr.Post("/lipidspace/v1/pca", [](const Request &req, Response &res){
            if (req.get_header_value("Content-Type") == "application/json"){

                QString callId = QUuid::createUuid().toString(QUuid::StringFormat::WithoutBraces);
                // ensure we have a unique directory for each REST call to allow for parallel processing
                QString dirPath = QString::fromStdString(GlobalData::rest_temp_folder) + "/" + callId + "/";
                if (!QDir().mkpath(dirPath))
                {
                    res.status = 500;
                    res.reason = ("Failed to create temporary directory for call id " + callId).toStdString();
                    qWarning("Failed to create temporary directory: '%s'", qPrintable(dirPath));
                    return;
                }
                QByteArray qbytes = QByteArray(req.body.c_str());
                if (GlobalData::debug) {
                    QString requestFile = dirPath + "/request.json";
                    ofstream request_file(requestFile.toStdString().c_str());
                    request_file << req.body;
                    request_file.flush();
                    qInfo() << "Saved request to: '" << requestFile.toStdString().c_str() << "'";
                }
                QJsonDocument pcaRequest = QJsonDocument::fromJson(qbytes);
                if (pcaRequest.isNull())
                {
                    qWarning() << "Malformed JSON: '" << req.body.c_str() << "'";
                    res.status = 400;
                    res.reason = "Malformed JSON";
                }
                else {
                    // check if request is an object
                    if (!pcaRequest.isObject())
                    {
                        res.status = 400;
                        res.reason = "Malformed JSON, not a dictionary";
                        qWarning() << res.reason.c_str();
                        return;
                    }

                    // check if request contains valid keys
                    for (auto key : dict_keys)
                    {
                        if (!pcaRequest.object().contains(key.c_str()))
                        {
                        res.status = 400;
                        res.reason = "Malformed JSON, key '" + key + "' not a dictionary";
                        qWarning() << res.reason.c_str();
                        return;
                        }
                    }

                    // check if request contains valid table type
                    if (!pcaRequest["TableType"].isString())
                    {
                        res.status = 400;
                        res.reason = "Malformed JSON, 'TableType' value is not a string";
                        qWarning() << res.reason.c_str();
                        return;
                    }

                    // check if request contains valid table type
                    if (uncontains_val(TableTypeMap, pcaRequest["TableType"].toString().toStdString()))
                    {
                        res.status = 400;
                        res.reason = "Malformed JSON, '" + pcaRequest["TableType"].toString().toStdString() + "'is not a valid table type";
                        qWarning() << res.reason.c_str();
                        return;
                    }

                    // check if table column types are valid
                    if (!pcaRequest["TableColumnTypes"].isArray())
                    {
                        res.status = 400;
                        res.reason = "Malformed JSON, 'TableColumnTypes' value is not an array";
                        qWarning() << res.reason.c_str();
                        return;
                    }
                    for (auto value : pcaRequest["TableColumnTypes"].toArray())
                    {
                        if (!value.isString())
                        {
                        res.status = 400;
                        res.reason = "Malformed JSON, 'TableColumnTypes' array contains a non string";
                        qWarning() << res.reason.c_str();
                        return;
                        }

                        if (uncontains_val(TableColumnTypeMap, value.toString().toStdString()))
                        {
                        res.status = 400;
                        res.reason = "Malformed JSON, '" + value.toString().toStdString() + "' is not a valid table column type";
                        qWarning() << res.reason.c_str();
                        return;
                        }
                    }

                    // check if table is of string type
                    if (!pcaRequest["Table"].isString())
                    {
                        res.status = 400;
                        res.reason = "Malformed JSON, 'Table' value is not a string";
                        qWarning() << res.reason.c_str();
                        return;
                    }

                    // check which spaces are request, if none, default is all
                    set<string> requested_spaces;
                    if (pcaRequest.object().contains("RequestedSpaces")){
                        if (GlobalData::debug) {
                            qInfo() << "RequestedSpaces: '" << pcaRequest["RequestedSpaces"].toArray() << "'";
                        }
                        for (auto value : pcaRequest["RequestedSpaces"].toArray()) {
                            string requested_space = value.toString().toStdString();
                            requested_spaces.insert(requested_space);
                        }
                    }








                    // number of PCA dimensions to return; default 2 keeps existing
                    // REST clients and the Qt application byte-compatible
                    int pca_dimensions = 2;
                    if (pcaRequest.object().contains("PcaDimensions")){
                        if (!pcaRequest["PcaDimensions"].isDouble()){
                            res.status = 400;
                            res.reason = "Malformed JSON, 'PcaDimensions' is not an integer";
                            qWarning() << res.reason.c_str();
                            return;
                        }
                        pca_dimensions = pcaRequest["PcaDimensions"].toInt(2);
                        if (pca_dimensions < 2) pca_dimensions = 2;
                    }

                    // store table on the disk
                    QString table_file_name = dirPath + "/table_file.csv";
                    ofstream table_file(table_file_name.toStdString().c_str());
                    table_file << pcaRequest["Table"].toString().toStdString();
                    table_file.flush();

                    if (GlobalData::debug) {
                        qInfo() << "Preparing LipidSpace analysis with table file '" << table_file_name << "'";
                    }
                    // retrieve data from the request
                    TableType table_type = TableTypeMap.at(pcaRequest["TableType"].toString().toStdString());
                    vector<TableColumnType> *column_types = new vector<TableColumnType>();
                    for (auto value : pcaRequest["TableColumnTypes"].toArray())
                    {
                        column_types->push_back(TableColumnTypeMap.at(value.toString().toStdString()));
                    }


                    // setup LipidSpace
                    LipidSpace lipid_space;
                    lipid_space.ignore_unknown_lipids = true;
                    lipid_space.ignore_doublette_lipids = true;

                    // lipidome distance: "hausdorff" (default) or "hellinger" (Atlas fingerprint)
                    if (pcaRequest.object().contains("DistanceMetric")){
                        if (!pcaRequest["DistanceMetric"].isString()){
                            res.status = 400;
                            res.reason = "Malformed JSON, 'DistanceMetric' is not a string";
                            qWarning() << res.reason.c_str();
                            return;
                        }
                        string dm = pcaRequest["DistanceMetric"].toString().toStdString();
                        if (dm == "hellinger") lipid_space.distance_metric = HellingerMetric;
                        else if (dm == "hausdorff") lipid_space.distance_metric = HausdorffMetric;
                        else {
                            res.status = 400;
                            res.reason = "Invalid 'DistanceMetric' (use 'hausdorff' or 'hellinger')";
                            qWarning() << res.reason.c_str();
                            return;
                        }
                    }


                    stringstream sstream;

                    // load the provided table (which is stored on disk)
                    ImportData *import_data = 0;

                    try
                    {
                        if (GlobalData::debug) {
                        qInfo() << "Preparing ImportData";
                        }
                        import_data = new ImportData(table_file_name.toStdString(), "", table_type, column_types);
                    }
                    catch (LipidSpaceException &e)
                    {
                        res.status = 400;
                        res.reason = string("An error occurred during LipidSpace ImportData, '") + e.what() + string("'");
                        qWarning() << res.reason.c_str();
                        return;
                    }
                    catch (std::exception &e)
                    {
                        res.status = 500;
                        res.reason = string("A server error occurred during LipidSpace ImportData, '") + e.what() + string("'");
                        qWarning() << res.reason.c_str();
                        return;
                    }


                    try
                    {
                        if (GlobalData::debug) {
                            qInfo() << "Loading table";
                        }
                        lipid_space.load_table(import_data);
                    }
                    catch (LipidSpaceException &e)
                    {
                        res.status = 400;
                        res.reason = string("An error occurred during LipidSpace load_row_table, '") + e.what() + string("'");
                        qWarning() << res.reason.c_str();
                        return;
                    }
                    catch (std::exception &e)
                    {
                        res.status = 500;
                        res.reason = string("A server error occurred during LipidSpace load_row_table, '") + e.what() + string("'");
                        qWarning() << res.reason.c_str();
                        return;
                    }
                    if (import_data) delete import_data;


                    try
                    {
                        if (GlobalData::debug) {
                        qInfo() << "Calling run_analysis";
                        }
                        lipid_space.run_analysis();
                    }
                    catch (LipidSpaceException &e)
                    {
                        res.status = 400;
                        res.reason = string("An error occurred during LipidSpace analysis, '") + e.what() + string("'");
                        qWarning() << res.reason.c_str();
                        return;
                    }
                    catch (std::exception &e)
                    {
                        res.status = 500;
                        res.reason = string("A server error occurred during LipidSpace analysis, '") + e.what() + string("'");
                        qWarning() << res.reason.c_str();
                        return;
                    }

                    // check if enough lipids and lipidomes are provided for the analysis
                    if (lipid_space.lipidomes.size() < 1)
                    {
                        res.status = 400;
                        res.reason = string("An error occurred during LipidSpace analysis, no lipidome was provided");
                        qWarning() << res.reason.c_str();
                        return;
                    }
                    if (lipid_space.global_lipidome->lipids.size() < 3)
                    {
                        res.status = 400;
                        res.reason = string("An error occurred during LipidSpace analysis, less than 3 lipids in total were provided");
                        qWarning() << res.reason.c_str();
                        return;
                    }

                    // write output stream in json format
                    // add a list of lipid spaces
                    sstream << "{\"LipidSpaces\": [";
                    int cnt = 0;

                    map<string, string> &imported_lipid_names = lipid_space.lipid_name_translations[IMPORT_NAME];

                    // add global lipidome if requested
                    if (requested_spaces.empty() || contains_val(requested_spaces, "global")){
                        cnt++;
                        sstream << lipid_space.global_lipidome->to_json(&imported_lipid_names, pca_dimensions);
                    }

                    // add group lipidomes if requested
                    for (auto &kv : lipid_space.group_lipidomes){
                        // log available group_lipidomes
                        if (GlobalData::debug) {
                            qInfo() << "Available group_lipidomes: '" << kv.first.c_str() << "'";
                        }
                        if (!requested_spaces.empty() && uncontains_val(requested_spaces, kv.first)) continue;
                        for (auto lipidome : kv.second){
                            if (GlobalData::debug) {
                                qInfo() << "Lipidome: '" << lipidome->lipidome_name.c_str() << "'";
                            }
                            sstream << (cnt++ > 0 ? ", " : "") << lipidome->to_json(&imported_lipid_names, pca_dimensions);
                        }
                    }

                    // add sample lipidomes if requested
                    for (auto lipidome : lipid_space.lipidomes)
                    {
                        if (requested_spaces.empty() || contains_val(requested_spaces, lipidome->lipidome_name)){
                            sstream << (cnt++ > 0 ? ", " : "") << lipidome->to_json(&imported_lipid_names, pca_dimensions);
                        }
                    }

                    // add the lipidome distance matrix
                    sstream << "]";




                    if (requested_spaces.empty() || contains_val(requested_spaces, "LipidomeDistanceMatrix")){

                        sstream << ", \"LipidomeDistanceMatrix\": [";

                        Matrix &m = lipid_space.hausdorff_distances;
                        for (int r = 0; r < m.rows; ++r)
                        {
                            if (r)
                            sstream << ", ";
                            sstream << "[";
                            for (int c = 0; c < m.cols; c++)
                            {
                            if (c)
                                sstream << ", ";
                            sstream << m(r, c);
                            }
                            sstream << "]";
                        }
                        sstream << "], \"LinkageMatrix\": [";

                        int index = lipid_space.lipidomes.size();
                        vector<vector<double>> linkages;
                        lipid_space.dendrogram_root->get_linkages(linkages, index);

                        int comma = 0;
                        for (auto &v : linkages){
                            sstream << (comma++ > 0 ? ", " : "") << "[" << v[0] << ", " << v[1] << ", " << v[2] << ", " << v[3] << "]";
                        }

                        sstream << "]";
                    }

                    sstream << "}";

                    // send it back
                    res.status = 200;
                    res.set_content(sstream.str(), "application/json");
                    if (GlobalData::debug) {
                        QString responseFile = dirPath + "/response.json";
                        ofstream response_file(responseFile.toStdString().c_str());
                        response_file << res.body;
                        response_file.flush();
                    }
                }
            }
            else
            {
                qWarning() << "Unsupported content type: '" << req.get_header_value("Content-Type").c_str() << "'";
                res.status = 415;
                res.reason = "Unsupported content type. Please use 'Content-Type=application/json'";
            }
        });

        // Build an Atlas: freeze a structural frame + modules, fingerprint every dataset,
        // calibrate confidence, and return the portable atlas artifact as JSON.
        svr.Post("/lipidspace/v1/atlas/build", [](const Request &req, Response &res){
            if (req.get_header_value("Content-Type") != "application/json") {
                res.status = 415;
                res.reason = "Unsupported content type. Please use 'Content-Type=application/json'";
                return;
            }
            QString callId = QUuid::createUuid().toString(QUuid::StringFormat::WithoutBraces);
            QString dirPath = QString::fromStdString(GlobalData::rest_temp_folder) + "/" + callId + "/";
            if (!QDir().mkpath(dirPath)) {
                res.status = 500;
                res.reason = "Failed to create temporary directory";
                return;
            }
            json body;
            try {
                body = json::parse(req.body);
            } catch (std::exception &e) {
                res.status = 400; res.reason = "Malformed JSON"; return;
            }

            LipidSpace lipid_space;
            if (!atlas_load_table(lipid_space, body, dirPath, true, res)) return;

            if (lipid_space.selected_lipidomes.size() < 1) {
                res.status = 400; res.reason = "No lipidomes were provided for the atlas"; return;
            }
            if (lipid_space.global_lipidome->lipids.size() < 3) {
                res.status = 400; res.reason = "Fewer than 3 lipids in total; cannot build a frame"; return;
            }

            int K = body.value("Modules", 20);
            string label_variable = body.value("LabelVariable", string());
            set<string> frame_only;
            if (body.contains("FrameOnlySamples") && body["FrameOnlySamples"].is_array()) {
                for (auto &v : body["FrameOnlySamples"]) {
                    if (v.is_string()) frame_only.insert(v.get<string>());
                }
            }

            Atlas atlas;
            try {
                atlas.build(lipid_space, K, label_variable, true, frame_only);
            } catch (std::exception &e) {
                res.status = 500; res.reason = string("Atlas build error: ") + e.what(); return;
            }

            json out;
            atlas.to_json(out);
            res.status = 200;
            res.set_content(out.dump(), "application/json");
        });

        // Fit query lipidome(s) against a prebuilt Atlas: return nearest datasets, an
        // optional predicted label, a calibrated confidence, an OOD flag, and frame coverage.
        svr.Post("/lipidspace/v1/atlas/fit", [](const Request &req, Response &res){
            if (req.get_header_value("Content-Type") != "application/json") {
                res.status = 415;
                res.reason = "Unsupported content type. Please use 'Content-Type=application/json'";
                return;
            }
            QString callId = QUuid::createUuid().toString(QUuid::StringFormat::WithoutBraces);
            QString dirPath = QString::fromStdString(GlobalData::rest_temp_folder) + "/" + callId + "/";
            if (!QDir().mkpath(dirPath)) {
                res.status = 500;
                res.reason = "Failed to create temporary directory";
                return;
            }
            json body;
            try {
                body = json::parse(req.body);
            } catch (std::exception &e) {
                res.status = 400; res.reason = "Malformed JSON"; return;
            }

            if (!body.contains("Atlas") || !body["Atlas"].is_object()) {
                res.status = 400; res.reason = "Malformed JSON, missing 'Atlas' object"; return;
            }
            Atlas atlas;
            try {
                atlas.from_json(body["Atlas"]);
            } catch (std::exception &e) {
                res.status = 400; res.reason = string("Invalid Atlas: ") + e.what(); return;
            }

            int k = body.value("NumNeighbors", 5);
            set<string> label_vars;   // which nominal study variables to predict; empty = all
            if (body.contains("LabelVariables") && body["LabelVariables"].is_array()) {
                for (auto &v : body["LabelVariables"]) {
                    if (v.is_string()) label_vars.insert(v.get<string>());
                }
            }

            LipidSpace lipid_space;
            if (!atlas_load_table(lipid_space, body, dirPath, false, res)) return;

            if (lipid_space.lipidomes.size() < 1) {
                res.status = 400; res.reason = "No query lipidomes were provided"; return;
            }

            // Parse the atlas reference lipids once, for Nystrom projection of any query
            // lipids not already present in the frozen frame. A dedicated LipidSpace owns
            // the parsed references (its all_lipids stays empty), so they free cleanly.
            bool project = !atlas.ref_names.empty() && atlas.eigenvectors.rows > 0;
            LipidSpace ref_ls;
            vector<LipidAdduct*> ref_lipids;
            map<string, LipidAdduct*> ref_map;
            if (project) {
                ref_ls.ignore_unknown_lipids = true;
                ref_ls.ignore_doublette_lipids = true;
                for (auto &name : atlas.ref_names) {
                    LipidAdduct *rl = nullptr;
                    try { rl = ref_ls.load_lipid(name, ref_map); }
                    catch (std::exception &) { rl = nullptr; }
                    ref_lipids.push_back(rl);
                }
            }

            json results = json::array();
            for (auto lipidome : lipid_space.lipidomes) {
                json r = project
                    ? atlas.fit_projected(lipid_space, lipidome, ref_lipids, k, label_vars)
                    : atlas.fit(lipidome->species, lipidome->original_intensities, k, label_vars);
                r["query"] = lipidome->lipidome_name;   // raw sample name (no " - <file>" suffix)
                results.push_back(r);
            }
            for (auto &kv : ref_map) if (kv.second) delete kv.second;

            json out;
            out["results"] = results;
            res.status = 200;
            res.set_content(out.dump(), "application/json");
        });

        svr.Get("/lipidspace/v1/openapi.yaml", [](const Request &, Response &res){
            QFile file(":/rest/openapi.yaml");
            if (file.open(QIODevice::ReadOnly)){
                res.set_content(file.readAll().toStdString(), "application/yaml");
            } else {
                res.status = 500;
                res.reason = "Failed to load embedded OpenAPI spec";
            }
        });

        svr.Get("/lipidspace/v1/docs", [](const Request &, Response &res){
            QFile file(":/rest/swagger-ui.html");
            if (file.open(QIODevice::ReadOnly)){
                res.set_content(file.readAll().toStdString(), "text/html; charset=utf-8");
            } else {
                res.status = 500;
                res.reason = "Failed to load embedded Swagger UI";
            }
        });

        svr.Get("/actuator/health", [](const Request &, Response &res){
            QString storagePath = GlobalData::rest_temp_folder.empty()
                ? "."
                : QString::fromStdString(GlobalData::rest_temp_folder);
            QStorageInfo storage(storagePath);

            const qint64 threshold = GlobalData::rest_disk_threshold_bytes;
            bool diskOk = storage.isValid() && storage.bytesFree() >= threshold;
            QString diskStatus = diskOk ? "UP" : "DOWN";

            QJsonObject diskDetails;
            diskDetails["total"] = storage.bytesTotal();
            diskDetails["free"]  = storage.bytesFree();
            diskDetails["threshold"] = threshold;
            diskDetails["path"] = storagePath;

            QJsonObject diskComponent;
            diskComponent["status"]  = diskStatus;
            diskComponent["details"] = diskDetails;

            QJsonObject components;
            components["diskSpace"] = diskComponent;

            QJsonObject healthObj;
            healthObj["status"]     = diskOk ? QString("UP") : QString("DOWN");
            healthObj["components"] = components;

            QJsonDocument healthDoc(healthObj);
            res.status = diskOk ? 200 : 503;
            res.set_content(healthDoc.toJson(QJsonDocument::Compact).toStdString(), "application/json");
        });

        t = thread([&]()
                   { svr.listen(host.c_str(), port); });

        vacuum_running.store(true);
        vacuum_t = thread([this]{ vacuum_loop(); });
        qInfo("Started server with vacuum (interval=%ds max_age=%ds min_active=%ds)! SIGINT (CTRL + c) will stop the server.",
              GlobalData::rest_vacuum_interval_secs,
              GlobalData::rest_vacuum_max_age_secs,
              GlobalData::rest_vacuum_min_active_secs);
        return 0;
    }

    inline int stop(int signal)
    {
        qInfo() << "Server received signal: " << signal;
        if (signal == SIGINT || signal == SIGHUP || signal == SIGKILL || signal == SIGABRT)
        { // only handle SIG INTERRUPT for now
            if (svr.is_running())
            {
                qInfo() << "Stopping server...";
                vacuum_running.store(false);
                if (vacuum_t.joinable()) vacuum_t.join();
                svr.stop();
                t.join();
                qInfo() << "Stopped server!";
                return 0; // server and thread have terminated
            }
            qInfo() << "Server is not running!";
            return 1; // server is not running
        }
        qInfo() << "Server does not handle signal.";
        return 0;
    }
};

static LipidSpaceRest lsr;
extern "C" void signal_handler(int signum) {
   int status = lsr.stop(signum);
   exit(status);
}

void handleSigInt(int x)
{
    int status = lsr.stop(x);
    exit(status);
}

int main(int argc, char *argv[])
{

    signal(SIGINT, signal_handler);
    signal(SIGHUP, signal_handler);
    signal(SIGKILL, signal_handler);
    signal(SIGABRT, signal_handler);

    using namespace std;
    using namespace httplib;

    QCommandLineParser parser;
    parser.setApplicationDescription("REST API for LipidSpace");
    parser.addHelpOption();
    parser.addVersionOption();
    QCommandLineOption hostOption({"b", "bind"}, QCoreApplication::translate("main", "Host address to <bind> to."), "bind", "0.0.0.0");
    parser.addOption(hostOption);
    QCommandLineOption portOption({"p", "port"}, QCoreApplication::translate("main", "Host <port> to listen on."), "port", "8888");
    parser.addOption(portOption);
    QCommandLineOption tmpOption({"t", "tmp_folder"}, QCoreApplication::translate("main", "Temp folder <tmp_folder> path."), "tmp_folder", ".");
    parser.addOption(tmpOption);
    QCommandLineOption debugOption({"d", "debug"}, QCoreApplication::translate("main", "Set the server to run in debug mode. Saves incoming and outgoing JSON requests."));
    parser.addOption(debugOption);
    QCommandLineOption diskThresholdOption({"k", "disk-threshold-mb"}, QCoreApplication::translate("main", "Minimum free disk space in MB on the tmp folder before the health endpoint reports DOWN."), "disk_threshold_mb", "10");
    parser.addOption(diskThresholdOption);
    QCommandLineOption vacuumIntervalOption({"i", "vacuum-interval-secs"}, QCoreApplication::translate("main", "Interval in <seconds> between vacuum runs."), "vacuum_interval_secs", "60");
    parser.addOption(vacuumIntervalOption);
    QCommandLineOption vacuumMaxAgeOption({"a", "vacuum-max-age-secs"}, QCoreApplication::translate("main", "Maximum age in <seconds> of a tmp directory before it is unconditionally removed."), "vacuum_max_age_secs", "3600");
    parser.addOption(vacuumMaxAgeOption);
    QCommandLineOption vacuumMinActiveOption({"m", "vacuum-min-active-secs"}, QCoreApplication::translate("main", "Minimum age in <seconds> a tmp directory must have before it may be removed due to disk pressure."), "vacuum_min_active_secs", "300");
    parser.addOption(vacuumMinActiveOption);

    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("LipidSpaceREST");
    QCoreApplication::setApplicationVersion(GlobalData::LipidSpace_version.c_str());

    QString host = "0.0.0.0";
    QString tmp_folder = ".";
    int port = 8888;
    bool debug = false;

    parser.process(QCoreApplication::arguments());

    if (parser.isSet(hostOption))
    {
        host = parser.value(hostOption);
        qInfo() << "Host set to: " << host.toStdString().c_str();
    }

    if (parser.isSet(portOption))
    {
        bool ok = false;
        int p = parser.value(portOption).toInt(&ok);
        if (ok && p > 0 && p <= 65535)
        {
            port = p;
            qInfo() << "Port set to:" << port;
        }
        else
        {
            qWarning() << "Invalid port value '" << parser.value(portOption) << "', using default 8888";
        }
    }

    if (parser.isSet(tmpOption))
    {
        tmp_folder = parser.value(tmpOption);
        qInfo() << "Tmp folder set to: '" << tmp_folder.toStdString().c_str() << "'";
    }

    if (parser.isSet(debugOption))
    {
        debug = true;
    }

    if (parser.isSet(diskThresholdOption))
    {
        bool ok = false;
        int mb = parser.value(diskThresholdOption).toInt(&ok);
        if (ok && mb > 0)
        {
            GlobalData::rest_disk_threshold_bytes = static_cast<qint64>(mb) * 1024 * 1024;
            qInfo() << "Disk threshold set to:" << mb << "MB";
        }
        else
        {
            qWarning() << "Invalid disk-threshold-mb value '" << parser.value(diskThresholdOption) << "', using default 10 MB";
        }
    }

    if (parser.isSet(vacuumIntervalOption))
    {
        bool ok = false;
        int secs = parser.value(vacuumIntervalOption).toInt(&ok);
        if (ok && secs > 0)
        {
            GlobalData::rest_vacuum_interval_secs = secs;
            qInfo() << "Vacuum interval set to:" << secs << "seconds";
        }
        else
        {
            qWarning() << "Invalid vacuum-interval-secs value '" << parser.value(vacuumIntervalOption) << "', using default 60s";
        }
    }

    if (parser.isSet(vacuumMaxAgeOption))
    {
        bool ok = false;
        int secs = parser.value(vacuumMaxAgeOption).toInt(&ok);
        if (ok && secs > 0)
        {
            GlobalData::rest_vacuum_max_age_secs = secs;
            qInfo() << "Vacuum max age set to:" << secs << "seconds";
        }
        else
        {
            qWarning() << "Invalid vacuum-max-age-secs value '" << parser.value(vacuumMaxAgeOption) << "', using default 3600s";
        }
    }

    if (parser.isSet(vacuumMinActiveOption))
    {
        bool ok = false;
        int secs = parser.value(vacuumMinActiveOption).toInt(&ok);
        if (ok && secs >= 0)
        {
            GlobalData::rest_vacuum_min_active_secs = secs;
            qInfo() << "Vacuum min active set to:" << secs << "seconds";
        }
        else
        {
            qWarning() << "Invalid vacuum-min-active-secs value '" << parser.value(vacuumMinActiveOption) << "', using default 300s";
        }
    }

    lsr.start(host.toStdString(), port, tmp_folder.toStdString(), debug);
    while (!(lsr.svr.is_running()))
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    lsr.t.join();
}
