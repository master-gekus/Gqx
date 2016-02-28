#ifndef TCP_CONNECTOR_P
#define TCP_CONNECTOR_P

#include "tcp_connector.h"

class TcpConnectorPrivate : public QObject
{
  Q_OBJECT

private:
  explicit TcpConnectorPrivate(TcpConnector* owner);
  ~TcpConnectorPrivate();

private:
  TcpConnector* owner_;
  TcpConnector::State state_;

  friend class TcpConnector;
};


#endif // TCP_CONNECTOR_P

