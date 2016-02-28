#ifndef CLIENTAPP_H
#define CLIENTAPP_H

#include <QApplication>

class TcpConnector;

class ClientApp : public QApplication
{
  Q_OBJECT

public:
  ClientApp(int &argc, char **argv);
  ~ClientApp();

public:
  QIcon iconMain() const;
  inline TcpConnector* connector() const
    { return connector_; }

private:
  TcpConnector *connector_;
};

#ifdef qApp
#undef qApp
#endif
#define qApp (static_cast<ClientApp*>(QApplication::instance()))

#endif // CLIENTAPP_H
