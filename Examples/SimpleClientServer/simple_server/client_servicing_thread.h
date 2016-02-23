#ifndef CLIENTSERVICINGTHREAD_H
#define CLIENTSERVICINGTHREAD_H

#include <QSharedPointer>
#include <QList>

#include "GSelfOwnedThread.h"

class GJson;
class ClientServicingThread;
class ClientServicingThreadPtr;
typedef QList<ClientServicingThreadPtr> ClientServicingThreadList;

class ClientServicingThreadPrivate;
class ClientServicingThread : public GSelfOwnedThread
{
  Q_OBJECT

private:
  explicit ClientServicingThread(qintptr socket_handle);
  ~ClientServicingThread();

public:
  static ClientServicingThreadPtr create(qintptr socket_handle);
  static ClientServicingThreadList list();
  static bool isListEmpty();
  static void stopAll();

protected:
  bool beforeExec() Q_DECL_OVERRIDE Q_DECL_FINAL;
  void afterExec() Q_DECL_OVERRIDE Q_DECL_FINAL;

protected slots:

signals:

private:
  ClientServicingThreadPrivate *d;

private:
  Q_DISABLE_COPY(ClientServicingThread)

  friend class ClientServicingThreadPrivate;
  friend class ClientServicingThreadPtr;
};

class ClientServicingThreadPtr : public QSharedPointer<ClientServicingThread>
{
private:
  inline explicit ClientServicingThreadPtr(ClientServicingThread *p) :
    QSharedPointer<ClientServicingThread>(p, deleter)
  {}

private:
  static inline void deleter(ClientServicingThread *obj)
    { if (obj) delete obj; }

  friend class ClientServicingThread;
};

#endif // CLIENTSERVICINGTHREAD_H
