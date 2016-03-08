#include "GSocketConnector.h"
#include "private/GSocketConnector_p.h"

// ///////////////////////////////////////////////////////////////////////////
GSocketConnectorPrivate::GSocketConnectorPrivate(GSocketConnector* owner) :
  owner_(owner),
  state_(GSocketConnector::UnconnectedState)
{

}

GSocketConnectorPrivate::~GSocketConnectorPrivate()
{

}

// ///////////////////////////////////////////////////////////////////////////
GSocketConnector::GSocketConnector(QObject *parent) :
  QObject(parent),
  d(new GSocketConnectorPrivate(this))
{

}

GSocketConnector::~GSocketConnector()
{
  delete d;
}

GSocketConnector::ConnectorState
GSocketConnector::state() const
{
  return d->state_;
}
