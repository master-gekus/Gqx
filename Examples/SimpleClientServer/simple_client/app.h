#ifndef CLIENTAPP_H
#define CLIENTAPP_H

#include <QApplication>

class ClientApp : public QApplication
{
  Q_OBJECT

public:
  ClientApp(int &argc, char **argv);
  ~ClientApp();

public:
  QIcon iconMain() const;

};

#ifdef qApp
#undef qApp
#endif
#define qApp (static_cast<ClientApp*>(QApplication::instance()))

#endif // CLIENTAPP_H
