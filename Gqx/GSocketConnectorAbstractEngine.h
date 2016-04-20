#ifndef G_SOCKET_CONNECTOR_ABSTRACT_ENGINE_H_INCLUDED
#define G_SOCKET_CONNECTOR_ABSTRACT_ENGINE_H_INCLUDED

#include "GSocketConnector.h"

class QIODevice;
class GSocketConnector;
class GSocketConnectorPrivate;

class GSocketConnectorAbstractEngine : public QObject
{
  Q_OBJECT

protected:
  GSocketConnectorAbstractEngine();
public:
  ~GSocketConnectorAbstractEngine();

private:
  Q_DISABLE_COPY(GSocketConnectorAbstractEngine)

private:
  virtual QIODevice* socket() const = 0;
  virtual void connect_to_server() = 0;
  virtual void disconnect_from_server() = 0;

private:
  virtual void attach_to_connector(GSocketConnector* connector);

protected:
  GSocketConnector* connector_;

protected:
  inline void setError(GSocketConnector::ConnectorError error,
                       QString error_string = QString());
  inline void setState(GSocketConnector::ConnectorState state);
  inline void socketConnected();
  inline void socketDisconnected();

  friend class GSocketConnector;
  friend class GSocketConnectorPrivate;
};

inline void
GSocketConnectorAbstractEngine::setError(GSocketConnector::ConnectorError error,
                                         QString error_string)
{
  connector_->setError(error, error_string);
}

inline void
GSocketConnectorAbstractEngine::setState(GSocketConnector::ConnectorState state)
{
  connector_->setState(state);
}

inline void
GSocketConnectorAbstractEngine::socketConnected()
{
  connector_->socketConnected();
}

inline void
GSocketConnectorAbstractEngine::socketDisconnected()
{
  connector_->socketDisconnected();
}

#endif // G_SOCKET_CONNECTOR_ABSTRACT_ENGINE_H_INCLUDED
