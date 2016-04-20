#include "private/GSocketConnector_p.h"
#include "private/GSocketConnectorTcpSocketEngine_p.h"

GSocketConnectorTcpSocketEnginePrivate::GSocketConnectorTcpSocketEnginePrivate(GSocketConnectorTcpSocketEngine* owner) :
  owner_(owner),
  port_(0)
{
}

GSocketConnectorTcpSocketEnginePrivate::~GSocketConnectorTcpSocketEnginePrivate()
{
}

void
GSocketConnectorTcpSocketEnginePrivate::connect_to_socket_signals()
{
  connect(&socket_, SIGNAL(connected()), SLOT(onSocketConnected()));
  connect(&socket_, SIGNAL(disconnected()), SLOT(onSocketDisconnected()));
  connect(&socket_, SIGNAL(error(QAbstractSocket::SocketError)),
          SLOT(onSocketError(QAbstractSocket::SocketError)));
  connect(&socket_, SIGNAL(stateChanged(QAbstractSocket::SocketState)),
          SLOT(onSocketStateChanged(QAbstractSocket::SocketState)));
}

void
GSocketConnectorTcpSocketEnginePrivate::onSocketConnected()
{
  qDebug("GSocketConnectorTcpSocketEnginePrivate::onSocketConnected()");
}

void
GSocketConnectorTcpSocketEnginePrivate::onSocketDisconnected()
{
  qDebug("GSocketConnectorTcpSocketEnginePrivate::onSocketDisconnected()");
}

void
GSocketConnectorTcpSocketEnginePrivate::onSocketError(QAbstractSocket::SocketError)
{
  qDebug("GSocketConnectorTcpSocketEnginePrivate::onSocketError()");
}

void
GSocketConnectorTcpSocketEnginePrivate::onSocketStateChanged(QAbstractSocket::SocketState)
{
  qDebug("GSocketConnectorTcpSocketEnginePrivate::onSocketStateChanged()");

  switch(socket_.state())
    {
    case QAbstractSocket::HostLookupState:
      owner_->setState(GSocketConnector::HostLookupState);
      break;

    case QAbstractSocket::ConnectingState:
      owner_->setState(GSocketConnector::ConnectingState);
      break;

    case QAbstractSocket::UnconnectedState:
      owner_->socketDisconnected();
      break;

    case QAbstractSocket::ConnectedState:
      owner_->socketConnected();
      break;

    default:
      break;
    }
}

GSocketConnectorTcpSocketEngine::GSocketConnectorTcpSocketEngine(const QString &hostName,
                                                                 quint16 port) :
  d(new GSocketConnectorTcpSocketEnginePrivate(this))
{
  d->host_name_ = hostName;
  d->port_ = port;
  d->connect_to_socket_signals();
}

GSocketConnectorTcpSocketEngine::GSocketConnectorTcpSocketEngine(const QHostAddress &address,
                                                                 quint16 port) :
  d(new GSocketConnectorTcpSocketEnginePrivate(this))
{
  d->host_address = address;
  d->port_ = port;
  d->connect_to_socket_signals();
}

GSocketConnectorTcpSocketEngine::~GSocketConnectorTcpSocketEngine()
{
  delete d;
}

QIODevice*
GSocketConnectorTcpSocketEngine::socket() const
{
  return &(d->socket_);
}

void
GSocketConnectorTcpSocketEngine::connect_to_server()
{
  Q_ASSERT(0 != connector_);
  Q_ASSERT(QTcpSocket::UnconnectedState == d->socket_.state());

  if (!d->host_address.isNull())
    {
      setState(GSocketConnector::ConnectingState);
      d->socket_.connectToHost(d->host_address, d->port_);
    }
  else if (!d->host_name_.isEmpty())
    {
      setState(GSocketConnector::HostLookupState);
      d->socket_.connectToHost(d->host_name_, d->port_);
    }
  else
    {
      setError(GSocketConnector::EngineInvalidParameters,
               QStringLiteral("Neither host name nor host address specified."));
    }
}

void
GSocketConnectorTcpSocketEngine::disconnect_from_server()
{
  Q_ASSERT(0 != connector_);

  d->socket_.disconnectFromHost();
}
