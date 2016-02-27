#ifndef CLIENT_SERVICING_THREAD_P
#define CLIENT_SERVICING_THREAD_P

#include <QByteArray>
#include <QHostAddress>

#include "client_servicing_thread.h"

class ClientServicingThreadFinishCatcher;
class QTcpSocket;

class ClientServicingThreadPrivate : public QObject
{
  Q_OBJECT

private:
  ClientServicingThreadPrivate(ClientServicingThread *owner,
                               quintptr socket_handle);
  ~ClientServicingThreadPrivate();


private:
  static ClientServicingThreadFinishCatcher finish_catcher_;
  ClientServicingThread *owner_;
  quintptr socket_handle_;
  QTcpSocket *socket_;
  QHostAddress peer_addr_,
               nat_addr_;
  quint16      peer_port_,
               nat_port_;
  QByteArray   peer_info_;
  bool disconnect_requested_;
  QByteArray data_;

private slots:
  void onSocketDisconnected();
  void onSocketReadyRead();

private:
  inline const char* peerInfo() const
    { return peer_info_.constData(); }

  void update_peer_info();

private:
  Q_DISABLE_COPY(ClientServicingThreadPrivate)

  friend class ClientServicingThread;
};

class ClientServicingThreadFinishCatcher : public QObject
{
  Q_OBJECT

private:
  ClientServicingThreadFinishCatcher();
  ~ClientServicingThreadFinishCatcher();

private:
  Q_DISABLE_COPY(ClientServicingThreadFinishCatcher)

private slots:
  void onThreadFinished();

  friend class ClientServicingThreadPrivate;
};


#endif // CLIENT_SERVICING_THREAD_P

