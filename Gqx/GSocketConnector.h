#ifndef G_SOCKET_CONNECTOR_H_INCLUDED
#define G_SOCKET_CONNECTOR_H_INCLUDED

#include <QObject>
#include <QAbstractSocket>

class GSocketConnectorPrivate;
class GSocketConnector : public QObject
{
  Q_OBJECT

public:
  enum ConnectorState
  {
    UnconnectedState,
    HostLookupState,
    ConnectingState,
    SslHandshakingState,
    AuthorizingState,
    ConnectedState,
    ReconnectingState
  };

  enum ConnectorError
  {
    NoConnectorError,



    UnknownConnectorError,
  };

public:
  explicit GSocketConnector(QObject *parent = 0);
  ~GSocketConnector();

public:
  ConnectorState state() const;

signals:

public slots:

private:
  GSocketConnectorPrivate *d;
};

#endif // G_SOCKET_CONNECTOR_H_INCLUDED
