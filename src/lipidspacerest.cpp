#include "httplib.h"
#include <chrono>
#include <thread>
#include <QtGlobal>
#include <QJsonDocument>
#include <QByteArray>
#include <QtCore>
#include <vector>
#include <fstream>
#include "cppgoslin/cppgoslin.h"
#include "lipidspace/AssistanceFunctions.h"
#include "lipidspace/globaldata.h"
#include "lipidspace/lipidspace.h"
#include "lipidspace/logging.h"

using namespace std;
using namespace httplib;

vector<string> dict_keys{"TableType", "TableColumnTypes", "Table"};

// https://datatracker.ietf.org/doc/html/rfc7807 - Problem Details for HTTP APIs
class ProblemDetails
{
public:
    std::optional<QString> type;
    QString title;
    int status;
    QString detail;
    std::optional<QString> instance;

    ProblemDetails(std::optional<QString> type, QString title, int status, QString detail, std::optional<QString> instance)
    {
        this->type = type;
        this->title = title;
        this->status = status;
        this->detail = detail;
        this->instance = instance;
    }

    ProblemDetails(std::string title, int status, std::string detail)
    {
        this->type = std::nullopt;
        this->title = QString::fromStdString(title);
        this->status = status;
        this->detail = QString::fromStdString(detail);
        this->instance = std::nullopt;
    }

    std::string to_json()
    {
        // create a json string
        // type is a URI reference and should only be included if it is not null and not empty
        // title is a short, human-readable summary of the problem type and always required to be provided
        // status is the HTTP status code and always required to be provided
        // detail is a human-readable explanation specific to this occurrence of the problem and always required to be provided
        // instance is a URI reference that identifies the specific occurrence of the problem and should only be included if it is not null and not empty
        std::string type_str = this->type.has_value() ? "\"type\": \"" + this->type.value().toStdString() + "\", " : "";
        std::string title_str = "\"title\": \"" + this->title.toStdString() + "\", ";
        std::string status_str = "\"status\": " + QString::number(this->status).toStdString() + ", ";
        std::string detail_str = "\"detail\": \"" + this->detail.toStdString() + "\", ";
        std::string instance_str = this->instance.has_value() ? "\"instance\": \"" + this->instance.value().toStdString() + "\", " : "";
        return "{" + type_str + title_str + status_str + detail_str + instance_str + "}";
    }
};

class LipidSpaceRest
{
public:
    Server svr;
    thread t;

    inline int start(string host, int port, string temp_folder, bool debug)
    {
        qInfo("Using %d threads with openBLAS!", openblas_get_num_threads());
        GlobalData::rest_temp_folder = temp_folder;
        GlobalData::debug = debug;
        // stop(SIGINT);
        qInfo("Starting LipidSpaceRest server version='%s' on host='%s' port='%d' tmp_folder='%s' debug='%s'", GlobalData::LipidSpace_version.c_str(), host.c_str(), port, temp_folder.c_str(), debug ? "true" : "false");
        // register handlers
        // Health Endpoint
        svr.Get("/lipidspace/v1/health", [](const Request &req, Response &res) {
            if (req.get_header_value("Accept") == "application/json"){
                res.status = 200;
                res.set_content("{\"status\": \"UP\"}", "application/json");
                return;
            }
            else
            {

                ProblemDetails pd = ProblemDetails("Unsupported accept media type", 415, "Unsupported accept media type header: '" + req.get_header_value("Accept") + "'! Please use 'Content-Type=application/json'!");
                res.status = pd.status;
                res.body = pd.to_json();
                qWarning() << pd.detail;
                return;
            }
        });
        // PCA Endpoint
        svr.Post("/lipidspace/v1/pca", [](const Request &req, Response &res){
            if (req.get_header_value("Content-Type") == "application/json"){
                if (GlobalData::debug) {
                    qInfo() << "Received request: '" << req.body.c_str() << "'";
                }

                QString callId = QUuid::createUuid().toString(QUuid::StringFormat::WithoutBraces);
                // ensure we have a unique directory for each REST call to allow for parallel processing
                QString dirPath = QString::fromStdString(GlobalData::rest_temp_folder) + "/" + callId + "/";
                if (!QDir().mkpath(dirPath))
                {
                    res.status = 500;
                    res.body = ProblemDetails("Failed to create temporary directory", 500, "Failed to create temporary directory for call id='" + callId.toStdString() + "'").to_json();
                    return;
                }
                QByteArray qbytes = QByteArray(req.body.c_str());
                if (GlobalData::debug) {
                    QString requestFile = dirPath + "/request.json";
                    ofstream request_file(requestFile.toStdString().c_str());
                    request_file << req.body;
                    request_file.flush();
                }
                QJsonDocument pcaRequest = QJsonDocument::fromJson(qbytes);
                if (pcaRequest.isNull())
                {
                    qWarning() << "Malformed JSON: '" << req.body.c_str() << "'";
                    res.status = 400;
                    res.body = ProblemDetails("Malformed JSON", res.status, "Malformed JSON received in request body!").to_json();
                    return;
                }
                else {
                    // check if request is an object
                    if (!pcaRequest.isObject())
                    {
                        ProblemDetails pd = ProblemDetails("Malformed JSON", 400, "Malformed JSON received in request body: not a dictionary!");
                        res.status = pd.status;
                        res.body = pd.to_json();
                        qWarning() << pd.detail;
                        return;
                    }

                    // check if request contains valid keys
                    for (auto key : dict_keys)
                    {
                        if (!pcaRequest.object().contains(key.c_str()))
                        {
                        ProblemDetails pd = ProblemDetails("Malformed JSON", 400, "Malformed JSON received in request body: key '" + key + "' is not a dictionary!");
                        res.status = pd.status;
                        res.body = pd.to_json();
                        qWarning() << pd.detail;
                        return;
                        }
                    }

                    // check if request contains valid table type
                    if (!pcaRequest["TableType"].isString())
                    {
                        ProblemDetails pd = ProblemDetails("Malformed JSON", 400, "Malformed JSON received in request body: 'TableType' value is not a string!");
                        res.status = pd.status;
                        res.body = pd.to_json();
                        qWarning() << pd.detail;
                        return;
                    }

                    // check if request contains valid table type
                    if (uncontains_val(TableTypeMap, pcaRequest["TableType"].toString().toStdString()))
                    {
                        ProblemDetails pd = ProblemDetails("Malformed JSON", 400, "Malformed JSON received in request body: '" + pcaRequest["TableType"].toString().toStdString() + "'is not a valid table type!");
                        res.status = pd.status;
                        res.body = pd.to_json();
                        qWarning() << pd.detail;
                        return;
                    }

                    // check if table column types are valid
                    if (!pcaRequest["TableColumnTypes"].isArray())
                    {
                        ProblemDetails pd = ProblemDetails("Malformed JSON", 400, "Malformed JSON, 'TableColumnTypes' value is not an array!");
                        res.status = pd.status;
                        res.body = pd.to_json();
                        qWarning() << pd.detail;
                        return;
                    }
                    for (auto value : pcaRequest["TableColumnTypes"].toArray())
                    {
                        if (!value.isString())
                        {
                        ProblemDetails pd = ProblemDetails("Malformed JSON", 400, "Malformed JSON, 'TableColumnTypes' array contains a non string!");
                        res.status = pd.status;
                        res.body = pd.to_json();
                        qWarning() << pd.detail;
                        return;
                        }

                        if (uncontains_val(TableColumnTypeMap, value.toString().toStdString()))
                        {
                        ProblemDetails pd = ProblemDetails("Malformed JSON", 400, "Malformed JSON, '" + value.toString().toStdString() + "' is not a valid table column type!");
                        res.status = pd.status;
                        res.body = pd.to_json();
                        qWarning() << pd.detail;
                        return;
                        }
                    }

                    // check if table is of string type
                    if (!pcaRequest["Table"].isString())
                    {
                        ProblemDetails pd = ProblemDetails("Malformed JSON", 400, "Malformed JSON, 'Table' value is not a string!");
                        res.status = pd.status;
                        res.body = pd.to_json();
                        qWarning() << pd.detail;
                        return;
                    }

                    // check which spaces are request, if none, default is all
                    set<string> requested_spaces;
                    if (pcaRequest.object().contains("RequestedSpaces")){
                        for (auto value : pcaRequest["RequestedSpaces"].toArray()) {
                            string requested_space = value.toString().toStdString();
                            requested_spaces.insert(requested_space);
                        }
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
                    LipidSpace lipid_space(false);
                    lipid_space.ignore_unknown_lipids = true;
                    lipid_space.ignore_doublette_lipids = true;


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
                        ProblemDetails pd = ProblemDetails("LipidSpace import error: ImportData", 400, string("An error occurred during LipidSpace ImportData, '") + e.what() + string("'!"));
                        res.status = pd.status;
                        res.body = pd.to_json();
                        qWarning() << pd.detail;
                        return;
                    }
                    catch (std::exception &e)
                    {
                        ProblemDetails pd = ProblemDetails("LipidSpace server error", 500, string("A server error occurred during LipidSpace ImportData, '") + e.what() + string("'!"));
                        res.status = pd.status;
                        res.body = pd.to_json();
                        qWarning() << pd.detail;
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
                        ProblemDetails pd = ProblemDetails("LipidSpace import error: load_row_table", 400, string("An error occurred during LipidSpace load_row_table, '") + e.what() + string("'!"));
                        res.status = pd.status;
                        res.body = pd.to_json();
                        qWarning() << pd.detail;
                        return;
                    }
                    catch (std::exception &e)
                    {
                        ProblemDetails pd = ProblemDetails("LipidSpace server error: load_row_table", 500, string("A server error occurred during LipidSpace load_row_table, '") + e.what() + string("'!"));
                        res.status = pd.status;
                        res.body = pd.to_json();
                        qWarning() << pd.detail;
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
                        ProblemDetails pd = ProblemDetails("LipidSpace analysis error", 400, string("An error occurred during LipidSpace analysis, '") + e.what() + string("'!"));
                        res.status = pd.status;
                        res.body = pd.to_json();
                        qWarning() << pd.detail;
                        return;
                    }
                    catch (std::exception &e)
                    {
                        ProblemDetails pd = ProblemDetails("LipidSpace server error", 500, string("A server error occurred during LipidSpace analysis, '") + e.what() + string("'!"));
                        res.status = pd.status;
                        res.body = pd.to_json();
                        qWarning() << pd.detail;
                        return;
                    }

                    // check if enough lipids and lipidomes are provided for the analysis
                    if (lipid_space.lipidomes.size() < 1)
                    {
                        ProblemDetails pd = ProblemDetails("LipidSpace analysis error", 400, "An error occurred during LipidSpace analysis, no lipidome was provided!");
                        res.status = pd.status;
                        res.body = pd.to_json();
                        qWarning() << pd.detail;
                        return;
                    }
                    if (lipid_space.global_lipidome->lipids.size() < 3)
                    {
                        ProblemDetails pd = ProblemDetails("LipidSpace analysis error", 400, "An error occurred during LipidSpace analysis, less than 3 lipids in total were provided!");
                        res.status = pd.status;
                        res.body = pd.to_json();
                        qWarning() << pd.detail;
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
                        sstream << lipid_space.global_lipidome->to_json(&imported_lipid_names);
                    }

                    // add group lipidomes if requested
                    for (auto &kv : lipid_space.group_lipidomes){
                        if (!requested_spaces.empty() && uncontains_val(requested_spaces, kv.first)) continue;
                        for (auto lipidome : kv.second){
                            sstream << (cnt++ > 0 ? ", " : "") << lipidome->to_json(&imported_lipid_names);
                        }
                    }

                    // add sample lipidomes if requested
                    for (auto lipidome : lipid_space.lipidomes)
                    {
                        if (requested_spaces.empty() || contains_val(requested_spaces, lipidome->lipidome_name)){
                            sstream << (cnt++ > 0 ? ", " : "") << lipidome->to_json(&imported_lipid_names);
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
                ProblemDetails pd = ProblemDetails("Unsupported content type", 415, "Unsupported content type: '" + req.get_header_value("Content-Type") + "'! Please use 'Content-Type=application/json'!");
                res.status = pd.status;
                res.body = pd.to_json();
                qWarning() << pd.detail;
                return;
            }
        });

        /*
        svr.Post("/hi", [](const Request &req, Response &res){
            res.set_content("Hello World!", "text/plain");
        });
        */

        t = thread([&]()
                   { svr.listen(host.c_str(), port); });
        qInfo() << "Started server! SIGINT (CTRL + c) will stop the server.";
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

    if (parser.isSet(tmpOption))
    {
        tmp_folder = parser.value(tmpOption);
        qInfo() << "Tmp folder set to: '" << host.toStdString().c_str() << "'";
    }

    if (parser.isSet(debugOption))
    {
        debug = true;
    }

    lsr.start(host.toStdString(), port, tmp_folder.toStdString(), debug);
    while (!(lsr.svr.is_running()))
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    lsr.t.join();
}
