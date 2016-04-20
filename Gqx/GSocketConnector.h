#ifndef G_SOCKET_CONNECTOR_H_INCLUDED
#define G_SOCKET_CONNECTOR_H_INCLUDED

#include <QObject>
#include <QAbstractSocket>

class GSocketConnectorAbstractEngine;

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
    NoError,
    NoEngine,
    EngineInvalidParameters,
    EngineError,

    UnknownConnectorError,
  };

public:
  explicit GSocketConnector(QObject *parent = 0);
  ~GSocketConnector();

public:
  ConnectorState state() const;
  ConnectorError error() const;
  QString errorString() const;

  GSocketConnectorAbstractEngine *engine() const;
  void setEngine(GSocketConnectorAbstractEngine *engine = 0);

  void connectToServer();
  inline void connectToServer(GSocketConnectorAbstractEngine *engine);
  void disconnectFromServer();

protected:
  void setError(ConnectorError err, QString error_string = QString());
  void setState(ConnectorState state);

protected:
  virtual void socketConnected();
  virtual void socketDisconnected();

signals:
  void connected();
  void disconnected();
  void error(GSocketConnector::ConnectorError error);
  void stateChanged(GSocketConnector::ConnectorState state);

public slots:

private:
  GSocketConnectorPrivate *d;

  friend class GSocketConnectorAbstractEngine;
};

inline void
GSocketConnector::connectToServer(GSocketConnectorAbstractEngine *engine)
{
  setEngine(engine);
  connectToServer();
}

Q_DECLARE_METATYPE(GSocketConnector::ConnectorState)
Q_DECLARE_METATYPE(GSocketConnector::ConnectorError)

#endif // G_SOCKET_CONNECTOR_H_INCLUDED
