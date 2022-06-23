#include <libraries/cpp-httplib/httplib.h>
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

class LipidSpaceRest
{
public:
    Server svr;
    thread t;

    inline int start(string host, int port, string temp_folder, bool debug)
    {
        GlobalData::rest_temp_folder = temp_folder;
        GlobalData::debug = debug;
        // stop(SIGINT);
        qInfo("Starting LipidSpaceRest server version='%s' on host='%s' port='%d' tmp_folder='%s' debug='%s'", GlobalData::LipidSpace_version.c_str(), host.c_str(), port, temp_folder.c_str(), debug ? "true" : "false");
        // register handlers

        svr.Post("/lipidspace/v1/pca", [](const Request &req, Response &res)
                 {
               if (req.get_header_value("Content-Type") == "application/json")
               {
                 QString callId = QUuid::createUuid().toString(QUuid::StringFormat::WithoutBraces);
                 // ensure we have a unique directory for each REST call to allow for parallel processing
                 QString dirPath = QString::fromStdString(GlobalData::rest_temp_folder) + "/" + callId + "/";
                 if (!QDir().mkpath(dirPath))
                 {
                   res.status = 500;
                   res.reason = ("Failed to create temporary directory for call id " + callId).toStdString();
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
                   qWarning("Malformed JSON: '%s'", req.body.c_str());
                   res.status = 400;
                   res.reason = "Malformed JSON";
                 }
                 else
                 {
                   // check if request is an object
                   if (!pcaRequest.isObject())
                   {
                     res.status = 400;
                     res.reason = "Malformed JSON, not a dictionary";
                     return;
                   }

                   // check if request contains valid keys
                   for (auto key : dict_keys)
                   {
                     if (!pcaRequest.object().contains(key.c_str()))
                     {
                       res.status = 400;
                       res.reason = "Malformed JSON, key '" + key + "' not a dictionary";
                       return;
                     }
                   }

                   // check if request contains valid table type
                   if (!pcaRequest["TableType"].isString())
                   {
                     res.status = 400;
                     res.reason = "Malformed JSON, 'TableType' value is not a string";
                     return;
                   }

                   // check if request contains valid table type
                   if (uncontains_val(TableTypeMap, pcaRequest["TableType"].toString().toStdString()))
                   {
                     res.status = 400;
                     res.reason = "Malformed JSON, '" + pcaRequest["TableType"].toString().toStdString() + "'is not a valid table type";
                     return;
                   }

                   // check if table column types are valid
                   if (!pcaRequest["TableColumnTypes"].isArray())
                   {
                     res.status = 400;
                     res.reason = "Malformed JSON, 'TableColumnTypes' value is not an array";
                     return;
                   }
                   for (auto value : pcaRequest["TableColumnTypes"].toArray())
                   {
                     if (!value.isString())
                     {
                       res.status = 400;
                       res.reason = "Malformed JSON, 'TableColumnTypes' array contains a non string";
                       return;
                     }

                     if (uncontains_val(TableColumnTypeMap, value.toString().toStdString()))
                     {
                       res.status = 400;
                       res.reason = "Malformed JSON, '" + value.toString().toStdString() + "' is not a valid table column type";
                       return;
                     }
                   }

                   // check if table is of string type
                   if (!pcaRequest["Table"].isString())
                   {
                     res.status = 400;
                     res.reason = "Malformed JSON, 'Table' value is not a string";
                     return;
                   }

                   // store table on the disk
                   QString table_file_name = dirPath + "/table_file.csv";
                   ofstream table_file(table_file_name.toStdString().c_str());
                   table_file << pcaRequest["Table"].toString().toStdString();
                   table_file.flush();

                   // retrieve data from the request
                   TableType table_type = TableTypeMap.at(pcaRequest["TableType"].toString().toStdString());
                   vector<TableColumnType> *column_types = new vector<TableColumnType>();
                   for (auto value : pcaRequest["TableColumnTypes"].toArray())
                   {
                     column_types->push_back(TableColumnTypeMap.at(value.toString().toStdString()));
                   }

                   // setup LipidSpace
                   LipidSpace lipid_space;
                   stringstream sstream;

                   // load the provided table (which is stored on disk)
                   try
                   {
                     switch (table_type)
                     {
                     case ROW_PIVOT_TABLE:
                       lipid_space.load_row_table(table_file_name.toStdString(), column_types, "");
                       break;

                     case COLUMN_PIVOT_TABLE:
                       lipid_space.load_column_table(table_file_name.toStdString(), column_types, "");
                       break;

                     case FLAT_TABLE:
                       lipid_space.load_flat_table(table_file_name.toStdString(), column_types, "");
                       break;
                     }
                     lipid_space.run_analysis();
                   }
                   catch (LipidSpaceException &e)
                   {
                     res.status = 400;
                     res.reason = string("An error occurred during LipidSpace analysis, '") + e.what() + string("'");
                     return;
                   }

                   // check if enough lipids and lipidomes are provided for the analysis
                   if (lipid_space.lipidomes.size() < 1)
                   {
                     res.status = 400;
                     res.reason = string("An error occurred during LipidSpace analysis, no lipidome was provided");
                     return;
                   }
                   if (lipid_space.global_lipidome->lipids.size() < 3)
                   {
                     res.status = 400;
                     res.reason = string("An error occurred during LipidSpace analysis, less than 3 lipids in total were provided");
                     return;
                   }

                   // write output stream in json format
                   // add a list of lipid spaces
                   sstream << "{\"LipidSpaces\": [";
                   sstream << lipid_space.global_lipidome->to_json();
                   for (auto lipidome : lipid_space.lipidomes)
                   {
                     sstream << ", " << lipidome->to_json();
                   }

                   // add the lipidome distance matrix
                   sstream << "], \"LipidomeDistanceMatrix\": [";

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
                   sstream << "]}";

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
                 qWarning("Unsupported content type: '%s'", req.get_header_value("Content-Type").c_str());
                 res.status = 415;
                 res.reason = "Unsupported content type. Please use 'Content-Type=application/json'";
               } });

        /*
        svr.Post("/hi", [](const Request &req, Response &res){
            res.set_content("Hello World!", "text/plain");
        });
        */

        t = thread([&]()
                   { svr.listen(host.c_str(), port); });
        qInfo("Started server! SIGINT (CTRL+c) will stop the server.");
        return 0;
    }

    inline int stop(int signal)
    {
        qInfo("Server received signal: %d", signal);
        if (signal == SIGINT)
        { // only handle SIG INTERRUPT for now
            if (svr.is_running())
            {
                qInfo("Stopping server...");
                svr.stop();
                t.join();
                qInfo("Stopped server!");
                return 0; // server and thread have terminated
            }
            qInfo("Server is not running!");
            return 1; // server is not running
        }
        qInfo("Server does not handle signal.");
        return 0;
    }
};

static LipidSpaceRest lsr;
extern "C" void signal_handler(int signum) { lsr.stop(signum); }

void handleSigInt(int x)
{
    int status = lsr.stop(x);
    exit(status);
}

int main(int argc, char *argv[])
{
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
    QCommandLineOption debugOption({"d", "debug"}, QCoreApplication::translate("main", "Set the server to run in mode. Saves incoming and outgoing JSON requests."));
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
        qInfo("Host set to: %s", host.toStdString().c_str());
    }

    if (parser.isSet(tmpOption))
    {
        tmp_folder = parser.value(tmpOption);
        qInfo("Tmp folder set to: '%s'", host.toStdString().c_str());
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
