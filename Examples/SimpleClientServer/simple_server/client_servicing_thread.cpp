#include <QReadWriteLock>
#include <QReadLocker>
#include <QWriteLocker>
#include <QMap>
#include <QEventLoop>
#include <QTcpSocket>
#include <QHostAddress>

#include "logging.h"

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
  socket_(0),
  peer_port_(0),
  nat_port_(0),
  disconnect_requested_(false)
{
}

ClientServicingThreadPrivate::~ClientServicingThreadPrivate()
{
}

void
ClientServicingThreadPrivate::onSocketDisconnected()
{
  if (!disconnect_requested_)
    C_WARN("Connection lost.");

  QMetaObject::invokeMethod(owner_, "_exit", Qt::QueuedConnection,
                            Q_ARG(int, 0));
}

void
ClientServicingThreadPrivate::onSocketReadyRead()
{
  socket_->write(socket_->readAll());
}

void
ClientServicingThreadPrivate::update_peer_info()
{
  peer_info_ = peer_addr_.toString().toUtf8() + ":"
               + QByteArray::number(peer_port_);
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
    {
      thr->d->disconnect_requested_ = true;
      QMetaObject::invokeMethod(thr.data(), "_exit", Qt::QueuedConnection,
                                Q_ARG(int, 0));
    }

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

  d->peer_addr_ = d->socket_->peerAddress();
  d->peer_port_ = d->socket_->peerPort();
  d->update_peer_info();

  C_TRACE("Connection established.");
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

const char*
ClientServicingThread::peerInfo() const
{
  return d->peer_info_.constData();
}
