#include "GSocketConnectorAbstractEngine.h"

GSocketConnectorAbstractEngine::GSocketConnectorAbstractEngine() :
  connector_(0)
{
}

GSocketConnectorAbstractEngine::~GSocketConnectorAbstractEngine()
{
}

void
GSocketConnectorAbstractEngine::attach_to_connector(GSocketConnector* connector)
{
  if (connector_ == connector)
    return;

  if (0 != connector_)
    {

    }

  connector_ = connector;

  if (0 != connector_)
    {

    }
}
