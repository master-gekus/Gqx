#include <QEvent>
#include <QTimerEvent>
#include <QStateMachine>

#include "GIdleHandler.h"
#include "private/GIdleHandler_p.h"

// ///////////////////////////////////////////////////////////////////////////
GIdleHandlerPrivate::GIdleHandlerPrivate(GIdleHandler* owner) :
  owner_(owner),
  object_(owner->parent()),
  timer_(this)
{
  if (0 == object_)
    {
      qDebug("GIdleHandler: no QObject specified.");
      return;
    }

  object_->installEventFilter(this);

  connect(&timer_, SIGNAL(timeout()), SLOT(onTimer()), Qt::QueuedConnection);
  timer_.setSingleShot(true);
  timer_.start(0);
}

GIdleHandlerPrivate::~GIdleHandlerPrivate()
{
}

void
GIdleHandlerPrivate::onTimer()
{
  emit
    owner_->idle();
}

bool
GIdleHandlerPrivate::eventFilter(QObject *object, QEvent*)
{
  if (object != object_)
    return false;

  if (!timer_.isActive())
    timer_.start(0);

  return false; // We are just watching, not catching!
}

// ///////////////////////////////////////////////////////////////////////////
GIdleHandler::GIdleHandler(QObject *parent) :
  QObject(parent),
  d(new GIdleHandlerPrivate(this))
{
}

GIdleHandler::~GIdleHandler()
{
  delete d;
}
