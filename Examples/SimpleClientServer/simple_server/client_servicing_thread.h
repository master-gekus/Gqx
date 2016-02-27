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

public:
  const char* peerInfo() const;

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

#define C_TRACE(s,...) \
  GLogger::info("[%s]: " s, peerInfo(), ##__VA_ARGS__)
#define C_CHATTER(s,...) \
  GLogger::chatter("[%s]: " s, peerInfo(), ##__VA_ARGS__)
#define C_INFO(s,...) \
  GLogger::info("[%s]: " s, peerInfo(), ##__VA_ARGS__)
#define C_WARN(s,...) \
  GLogger::warning("[%s]: " s, peerInfo(), ##__VA_ARGS__)
#define C_ERROR(s,...) \
  GLogger::error("[%s]: " s, peerInfo(), ##__VA_ARGS__)
#define C_FATAL(s,...) \
  GLogger::critical("[%s]: " s, peerInfo(), ##__VA_ARGS__)


#endif // CLIENTSERVICINGTHREAD_H
