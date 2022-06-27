#include <libraries/cpp-httplib/httplib.h>
#include <chrono>
#include <thread>
#include <QtGlobal>
#include <QJsonDocument>
#include <QByteArray>
#include <QtCore>

using namespace std;
using namespace httplib;

class LipidSpaceRest
{
public:
  Server svr;
  thread t;

  inline int start(const char *host, int port)
  {
    // stop(SIGINT);
    qInfo("Starting server on host='%s' port='%d'", host, port);
    // register handlers
    // compute global distance matrix (lipidspace.cpp 2729, PCA: 2749, 2764, 2765, 2773, createDendrogram: 2782, )
    // feature selection: (lipidspace.cpp 2831), "Genes" are bitvectors for feature selection
    // TableType s.h. AssistanceFunctions.h
    // input: one table per lipidome (with id + name), columns: "Species" (SampleColumn), optional: Study Variables + concatenierungs spalte aller study variables + 1 column per lipid name with quantities (LipidColumn) + column type info (SampleColumn, QuantColumn, LipidColumn, FeatureColumnNumerical, FeatureColumnNominal, IgnoreColumn)
    // create Lipidome objects (AssistanceFunction.h)
    svr.Post("/lipidspace/v1/pca", [](const Request &req, Response &res)
             {
      if(req.get_header_value("Content-Type")=="application/json") {
        qInfo("Received req body: '%s'", req.body.c_str());
        QByteArray qbytes = QByteArray(req.body.c_str());
        QJsonDocument pcaRequest = QJsonDocument::fromJson(qbytes);
        if(pcaRequest.isNull()) {
          qWarning("Malformed JSON: '%s'", req.body.c_str());
          res.status = 400;
          res.reason = "Malformed JSON";
        }
        std::string response = pcaRequest.toJson().toStdString();
        res.set_content(response, "application/json");
        // response is distance matrix, columns / rows are lipidomes, order is the same as in input
        qInfo("Setting response '%s'", response.c_str());
      } else {
        qWarning("Unsupported content type: '%s'", req.get_header_value("Content-Type").c_str());
        res.status = 415;
        res.reason = "Unsupported content type. Please use 'Content-Type=application/json'";
      } });
    svr.Get("/hi", [](const Request &req, Response &res)
            { res.set_content("Hello World!", "text/plain"); });

    t = thread([&]()
               { svr.listen("0.0.0.0", port); });
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

  signal(SIGINT, handleSigInt);

  QCoreApplication app(argc, argv);
  QCoreApplication::setApplicationName("LipidSpaceREST");
  QCoreApplication::setApplicationVersion("1.0");

  QCommandLineParser parser;
  parser.setApplicationDescription("REST Api for LipidSpace");
  parser.addHelpOption();
  parser.addVersionOption();
  QCommandLineOption hostOption({"b","bind"}, QCoreApplication::translate("main", "Host address to <bind> to."), "bind", "0.0.0.0");
  parser.addOption(hostOption);
  QCommandLineOption portOption({"p","port"}, QCoreApplication::translate("main", "Host <port> to listen on."), "port", "8888");
  parser.addOption(portOption);

  parser.process(QCoreApplication::arguments());

  QString host = "0.0.0.0";

  if (parser.isSet(hostOption))
  {
    host = parser.value(hostOption);
    qInfo("Host set to: %s", host.toStdString().c_str());
  }

  int port = 8888;
  if (parser.isSet(portOption))
  {
    QString portValue = parser.value(portOption);
    port = atoi(portValue.toStdString().c_str());
    qInfo("Port set to: %s", portValue.toStdString().c_str());
  }

  lsr.start(host.toStdString().c_str(), port);
  while (!(lsr.svr.is_running()))
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  lsr.t.join();
}