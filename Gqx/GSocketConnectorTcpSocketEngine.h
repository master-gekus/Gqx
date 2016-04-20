#ifndef G_SOCKET_CONNECTOR_TCP_SOCKET_ENGINE_H_INCLUDED
#define G_SOCKET_CONNECTOR_TCP_SOCKET_ENGINE_H_INCLUDED

#include "GSocketConnectorAbstractEngine.h"

class QHostAddress;
class GSocketConnectorTcpSocketEnginePrivate;

class GSocketConnectorTcpSocketEngine : public GSocketConnectorAbstractEngine
{
  Q_OBJECT

public:
  GSocketConnectorTcpSocketEngine(const QString &hostName, quint16 port);
  GSocketConnectorTcpSocketEngine(const QHostAddress &address, quint16 port);
  ~GSocketConnectorTcpSocketEngine();

private:
  Q_DISABLE_COPY(GSocketConnectorTcpSocketEngine)

private:
  QIODevice* socket() const Q_DECL_OVERRIDE;
  void connect_to_server() Q_DECL_OVERRIDE;
  void disconnect_from_server() Q_DECL_OVERRIDE;

private:
  GSocketConnectorTcpSocketEnginePrivate* d;

  friend class GSocketConnectorTcpSocketEnginePrivate;
};

#endif // G_SOCKET_CONNECTOR_TCP_SOCKET_ENGINE_H_INCLUDED
