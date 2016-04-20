#include "GSocketConnector.h"
#include "private/GSocketConnector_p.h"

#include "GSocketConnectorAbstractEngine.h"

// ///////////////////////////////////////////////////////////////////////////
GSocketConnectorPrivate::GSocketConnectorPrivate(GSocketConnector* owner) :
  owner_(owner),
  state_(GSocketConnector::UnconnectedState),
  engine_()
{

}

GSocketConnectorPrivate::~GSocketConnectorPrivate()
{
  if (engine_)
    engine_->deleteLater();
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

GSocketConnectorAbstractEngine*
GSocketConnector::engine() const
{
  return d->engine_;
}

void
GSocketConnector::setError(ConnectorError err, QString error_string)
{
  d->error_ = err;

  if (NoError == d->error_)
    {
      d->error_string_ = QStringLiteral("No error.");
    }
  else
    {
      if (!error_string.isEmpty())
        {
          d->error_string_ = error_string;
        }
      else
        {
          switch (d->error_)
            {
            case NoEngine:
              d->error_string_ = QStringLiteral("No engine specified.");
              break;

            case EngineInvalidParameters:
              d->error_string_ = QStringLiteral("Invalid engine parameters.");
              break;

            case EngineError:
              d->error_string_ = QStringLiteral("Error in engine.");
              break;

            default:
              d->error_string_ = QStringLiteral("Unknown error.");
              break;
            }
        }
      emit
        error(d->error_);
    }
}

void
GSocketConnector::setState(ConnectorState state)
{
  if (d->state_ == state)
    return;

  d->state_ = state;

  emit
    stateChanged(d->state_);

  switch (d->state_)
    {
    case UnconnectedState:
      emit
        disconnected();
      break;

    case ConnectedState:
      emit
        connected();
      break;

    default:
      break;
    }
}

void
GSocketConnector::socketConnected()
{
  setState(ConnectedState);
}

void
GSocketConnector::socketDisconnected()
{
  setState(UnconnectedState);
}

void
GSocketConnector::setEngine(GSocketConnectorAbstractEngine *engine)
{
  if (UnconnectedState != d->state_)
    {
      qDebug("GSocketConnector::setEngine(): Engine can be changed only in "
             "Unconnected State.");
      return;
    }

  if (d->engine_ == engine)
    return;

  if (0 != d->engine_)
    {
      d->engine_->attach_to_connector(0);
      d->engine_->deleteLater();
    }

  d->engine_ = engine;

  if (0 != d->engine_)
    d->engine_->attach_to_connector(this);
}

void
GSocketConnector::connectToServer()
{
  if (UnconnectedState != d->state_)
    return;

  if (0 == d->engine_)
    {
      setError(NoEngine);
      return;
    }

  d->engine_->connect_to_server();
}

void
GSocketConnector::disconnectFromServer()
{
  if (UnconnectedState == d->state_)
    return;

  Q_ASSERT(0 != d->engine_);

  d->engine_->disconnect_from_server();
}
