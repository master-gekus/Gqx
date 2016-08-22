#include <QMetaObject>

#include <signal.h>
#if defined(Q_CC_MSVC)
  #include <io.h>
  #define isatty _isatty
  #include <Windows.h>
#else
  #include <unistd.h>
#endif

#include "logging.h"
#include "client_servicing_thread.h"
#include "app.h"

#define SERVER_PORT 6543

namespace
{
  TcpServer *tcp_server_ = 0;
  bool _initialize();
}

ServerApp::ServerApp(int& argc, char** argv) :
  QCoreApplication(argc, argv)
{
  setOrganizationName(QStringLiteral("Master Gekus"));
  setApplicationName(QStringLiteral("FamilyLand Server"));
  setApplicationVersion("Version " GIT_DESCRIBE " from " __DATE__ " " __TIME__);

  QMetaObject::invokeMethod(this, "initialize", Qt::QueuedConnection);
}

ServerApp::~ServerApp()
{
}

void
ServerApp::initialize()
{
  if (!_initialize())
    {
      shutdown();
      return;
    }
}

void
ServerApp::shutdown()
{
  if (GLogger::isRunning())
    {
      P_TRACE("This is a trace log message!");
      P_CHATTER("This is a chatter log message!");
      P_INFO("This is a info log message!");
      P_WARN("This is a warning log message!");
      P_ERROR("This is a error log message!");
      P_FATAL("This is a critical log message!");
    }

  if (GLogger::isRunning())
      INFO("%s starting shutdown process...",
           applicationName().toUtf8().constData());

  if (0 != tcp_server_)
    {
      if (GLogger::isRunning())
        INFO("Closing listening sockets...");
      tcp_server_->close();
      delete tcp_server_;
    }

  if (!ClientServicingThread::isListEmpty())
    {
      if (GLogger::isRunning())
        INFO("Stopping running threads...");

      ClientServicingThread::stopAll();

      if (GLogger::isRunning())
        INFO("All running threads are stopped.");
    }

  if (GLogger::isRunning())
    INFO("%s shutted down.", applicationName().toUtf8().constData());

  GLogger::stop();
  QCoreApplication::exit(0);
}

void
ServerApp::newIncomingConnection(qintptr socket_handle)
{
  ClientServicingThread::create(socket_handle);
}

void
TcpServer::incomingConnection(qintptr handle)
{
  emit
    newIncomingConnection(handle);
}

// ///////////////////////////////////////////////////////////////////////////
namespace
{
  void _signal_handler(int signal)
  {
    char buf[32];
    sprintf(buf, "SIGNAL-%d", signal);
    INFO("%s received.",
         (SIGINT == signal) ? "SIGINT" :
         (SIGTERM	== signal) ? "SIGTERM" : buf);

    QMetaObject::invokeMethod(qApp, "shutdown", Qt::QueuedConnection);
  }

  void _init_logging()
  {
    GLogger::start();
    if (isatty(fileno(stdout)))
      {
        GLogger::startOutput(QString(), GLogger::DefaultFlags, GLogger::Info);
      }
    else
      {
        GLogger::startOutput(QString(),
                             GLogger::LogFlags(GLogger::Full | GLogger::Unbuffered),
                             GLogger::Info);
      }

      GLogger::startOutput(QStringLiteral("spam_log.log"),
                           GLogger::LogFlags(GLogger::Full | GLogger::Unbuffered),
                           GLogger::Spam);

      GLogger::startOutput(QStringLiteral("info_log.log"),
                           GLogger::LogFlags(GLogger::Full | GLogger::Unbuffered),
                           GLogger::Info);

      GLogger::startOutput(QStringLiteral("error_log.log"),
                           GLogger::LogFlags(GLogger::Full | GLogger::Unbuffered),
                           GLogger::Error);

      GLogger::startOutput(QStringLiteral("critical_log.log"),
                           GLogger::LogFlags(GLogger::Full | GLogger::Unbuffered),
                           GLogger::Critical);

      TRACE("This is a trace log message!");
      CHATTER("This is a chatter log message!");
      INFO("This is a info log message!");
      WARN("This is a warning log message!");
      ERROR("This is a error log message!");
      FATAL("This is a critical log message!");
  }

  bool _start_listen()
  {
    tcp_server_ = new TcpServer();
    if (!tcp_server_->listen(QHostAddress::Any, SERVER_PORT))
      {
        ERROR("Can not start listening on port %d: %s.", SERVER_PORT,
              tcp_server_->errorString().toUtf8().constData());
        delete tcp_server_;
        tcp_server_ = 0;
        return false;
      }
    INFO("Start listening on port %d.", tcp_server_->serverPort());
#ifdef WIN32
    ::SetHandleInformation((HANDLE)tcp_server_->socketDescriptor(),
                           HANDLE_FLAG_INHERIT, 0 );
#endif
    qRegisterMetaType<qintptr>("qintptr");
    QObject::connect(tcp_server_, SIGNAL(newIncomingConnection(qintptr)), qApp,
                     SLOT(newIncomingConnection(qintptr)),
                     Qt::QueuedConnection);
    return true;
  }

  bool _initialize()
  {
    _init_logging();
    INFO("%s %s initializing...", qApp->applicationName().toUtf8().constData(),
         qApp->applicationVersion().toUtf8().constData());

    if (!_start_listen())
      return false;

    signal(SIGINT, _signal_handler);
    signal(SIGTERM, _signal_handler);
    INFO("%s started.", qApp->applicationName().toUtf8().constData());

    return true;
  }
}
