#include "tcp_connector.h"
#include "tcp_connector_p.h"

// ///////////////////////////////////////////////////////////////////////////
TcpConnectorPrivate::TcpConnectorPrivate(TcpConnector* owner) :
  owner_(owner),
  state_(TcpConnector::Disconnected)
{

}

TcpConnectorPrivate::~TcpConnectorPrivate()
{

}

// ///////////////////////////////////////////////////////////////////////////
TcpConnector::TcpConnector(QObject *parent) :
  QObject(parent),
  d(new TcpConnectorPrivate(this))
{

}

TcpConnector::~TcpConnector()
{
  delete d;
}

TcpConnector::State
TcpConnector::state() const
{
  return d->state_;
}
