#include <QReadWriteLock>
#include <QReadLocker>
#include <QWriteLocker>
#include <QMap>
#include <QEventLoop>
#include <QTcpSocket>

#include "client_servicing_thread.h"
#include "client_servicing_thread_p.h"

namespace
{
  QReadWriteLock running_threads_lock_(QReadWriteLock::Recursive);
  QMap<QThread const*,ClientServicingThreadPtr> running_threads_;
}

// ////////////////////////////////////////////////////////////////////////////
ClientServicingThreadFinishCatcher::ClientServicingThreadFinishCatcher()
{
}

ClientServicingThreadFinishCatcher::~ClientServicingThreadFinishCatcher()
{
}

void
ClientServicingThreadFinishCatcher::onThreadFinished()
{
  ClientServicingThread *thr = dynamic_cast<ClientServicingThread*>(sender());
  Q_ASSERT(0 != thr);

  QWriteLocker lock(&running_threads_lock_);
  Q_ASSERT(running_threads_.contains(thr));
  running_threads_.remove(thr);
}

// ////////////////////////////////////////////////////////////////////////////
ClientServicingThreadFinishCatcher ClientServicingThreadPrivate::finish_catcher_;

ClientServicingThreadPrivate::ClientServicingThreadPrivate(ClientServicingThread *owner,
                                                           quintptr socket_handle) :
  owner_(owner),
  socket_handle_(socket_handle),
  socket_(0)
{
}

ClientServicingThreadPrivate::~ClientServicingThreadPrivate()
{
}

void
ClientServicingThreadPrivate::onSocketDisconnected()
{
  QMetaObject::invokeMethod(owner_, "_exit", Qt::QueuedConnection,
                            Q_ARG(int, 0));
}

void
ClientServicingThreadPrivate::onSocketReadyRead()
{
  socket_->write(socket_->readAll());
}

// ////////////////////////////////////////////////////////////////////////////
ClientServicingThread::ClientServicingThread(qintptr socket_handle) :
  d(new ClientServicingThreadPrivate(this, socket_handle))
{
  running_threads_lock_.lockForWrite();
  Q_ASSERT(!running_threads_.contains(this));
  running_threads_.insert(this, ClientServicingThreadPtr(this));
  running_threads_lock_.unlock();

  connect(this, SIGNAL(finished()),
          &ClientServicingThreadPrivate::finish_catcher_,
          SLOT(onThreadFinished()),
          Qt::QueuedConnection);

  d->moveToThread(thread());

  releaseSemaphore();
}

ClientServicingThread::~ClientServicingThread()
{
  delete d;
}

ClientServicingThreadPtr
ClientServicingThread::create(qintptr socket_handle)
{
  ClientServicingThread *thr = new ClientServicingThread(socket_handle);

  QReadLocker lock(&running_threads_lock_);
  Q_ASSERT(running_threads_.contains(thr));
  return running_threads_.value(thr, ClientServicingThreadPtr(0));
}

ClientServicingThreadList
ClientServicingThread::list()
{
  QReadLocker lock(&running_threads_lock_);
  return running_threads_.values();
}

bool
ClientServicingThread::isListEmpty()
{
  QReadLocker lock(&running_threads_lock_);
  return running_threads_.isEmpty();
}

void
ClientServicingThread::stopAll()
{
  for (auto const& thr : list())
    QMetaObject::invokeMethod(thr.data(), "_exit", Qt::QueuedConnection,
                              Q_ARG(int, 0));

  QEventLoop theLoop;
  while(!isListEmpty())
    theLoop.processEvents(QEventLoop::WaitForMoreEvents);
}

bool
ClientServicingThread::beforeExec()
{
  if (!GSelfOwnedThread::beforeExec())
    return false;

  d->socket_ = new QTcpSocket();
  connect(d->socket_, SIGNAL(disconnected()), d, SLOT(onSocketDisconnected()));
  connect(d->socket_, SIGNAL(readyRead()), d, SLOT(onSocketReadyRead()));
  d->socket_->setSocketDescriptor(d->socket_handle_);

  return true;
}

void
ClientServicingThread::afterExec()
{
  if (0 != d->socket_)
    {
      delete d->socket_;
      d->socket_ = 0;
    }

  GSelfOwnedThread::afterExec();
}
