#ifndef CLIENTAPP_H
#define CLIENTAPP_H

#include <QApplication>

class GSocketConnector;

class ClientApp : public QApplication
{
  Q_OBJECT

public:
  ClientApp(int &argc, char **argv);
  ~ClientApp();

public:
  QIcon iconMain() const;
  inline GSocketConnector* connector() const
    { return connector_; }

private:
  GSocketConnector *connector_;
};

#ifdef qApp
#undef qApp
#endif
#define qApp (static_cast<ClientApp*>(QApplication::instance()))

#define DEFAULT_PORT 6543

#endif // CLIENTAPP_H
