#include <libraries/cpp-httplib/httplib.h>
#include <chrono>
#include <thread>
#include <QtGlobal>
#include <QJsonDocument>
#include <QByteArray>

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
    svr.Post("/lipidspace/v1/pca", [](const Request &req, Response &res){
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
        qInfo("Setting response '%s'", response.c_str());
      } else {
        qWarning("Unsupported content type: '%s'", req.get_header_value("Content-Type").c_str());
        res.status = 415;
        res.reason = "Unsupported content type. Please use 'Content-Type=application/json'";
      }
    });
    svr.Get("/hi", [](const Request &req, Response &res)
            { res.set_content("Hello World!", "text/plain"); });

    t = thread([&]()
               { svr.listen("0.0.0.0", 8888); });
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

int main(void)
{
  using namespace std;
  using namespace httplib;

  signal(SIGINT, handleSigInt);
  lsr.start("localhost", 8888);
  while (!(lsr.svr.is_running()))
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  lsr.t.join();
}