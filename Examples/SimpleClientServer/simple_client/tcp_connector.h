#ifndef TCPCONNECTOR_H
#define TCPCONNECTOR_H

#include <QObject>

class TcpConnectorPrivate;
class TcpConnector : public QObject
{
  Q_OBJECT

public:
  enum State
  {
    Disconnected,
    LookingForHost,
    Connecting,
    Connected,
    Reconnecting
  };

public:
  explicit TcpConnector(QObject *parent = 0);
  ~TcpConnector();

public:
  State state() const;

signals:

public slots:

private:
  TcpConnectorPrivate *d;
};

#endif // TCPCONNECTOR_H
