#ifndef G_SOCKET_CONNECTOR_PRIVATE_H_INCLUDED
#define G_SOCKET_CONNECTOR_PRIVATE_H_INCLUDED

#include "GSocketConnector.h"

class GSocketConnectorPrivate : public QObject
{
  Q_OBJECT

private:
  explicit GSocketConnectorPrivate(GSocketConnector* owner);
  ~GSocketConnectorPrivate();

private:
  GSocketConnector* owner_;
  GSocketConnector::ConnectorState state_;
  GSocketConnectorAbstractEngine *engine_;
  GSocketConnector::ConnectorError error_;
  QString error_string_;

  friend class GSocketConnector;
};


#endif // G_SOCKET_CONNECTOR_PRIVATE_H_INCLUDED
