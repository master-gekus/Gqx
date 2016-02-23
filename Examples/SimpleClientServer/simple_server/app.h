#ifndef SERVERAPP_H
#define SERVERAPP_H

#include <QCoreApplication>
#include <QTcpServer>

class ServerApp : public QCoreApplication
{
  Q_OBJECT

public:
  ServerApp(int &argc, char **argv);
  ~ServerApp();

private slots:
  void initialize();
  void shutdown();
  void newIncomingConnection(qintptr socket_handle);
};

class TcpServer : public QTcpServer
{
  Q_OBJECT

protected:
  void incomingConnection(qintptr handle) Q_DECL_OVERRIDE Q_DECL_FINAL;

signals:
  void newIncomingConnection(qintptr socket_handle);
};

Q_DECLARE_METATYPE(qintptr)

#ifdef qApp
#undef qApp
#endif
#define qApp (static_cast<ServerApp*>(QCoreApplication::instance()))

#endif // SERVERAPP_H
