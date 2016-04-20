#ifndef G_SOCKET_CONNECTOR_TCP_SOCKET_ENGINE_PRIVATE_H_INCLUDED
#define G_SOCKET_CONNECTOR_TCP_SOCKET_ENGINE_PRIVATE_H_INCLUDED

#include <QHostAddress>
#include <QTcpSocket>

#include "GSocketConnectorTcpSocketEngine.h"

class GSocketConnectorTcpSocketEnginePrivate : public QObject
{
  Q_OBJECT

private:
  explicit GSocketConnectorTcpSocketEnginePrivate(GSocketConnectorTcpSocketEngine* owner);
  ~GSocketConnectorTcpSocketEnginePrivate();

private:
  GSocketConnectorTcpSocketEngine* owner_;
  QTcpSocket socket_;
  QString host_name_;
  QHostAddress host_address;
  quint16 port_;

private:
  void connect_to_socket_signals();

private slots:
  void onSocketConnected();
  void onSocketDisconnected();
  void onSocketError(QAbstractSocket::SocketError);
  void onSocketStateChanged(QAbstractSocket::SocketState);

  friend class GSocketConnectorTcpSocketEngine;
};


#endif // G_SOCKET_CONNECTOR_TCP_SOCKET_ENGINE_PRIVATE_H_INCLUDED
