#ifndef GIDLEHANDLER_H
#define GIDLEHANDLER_H

#include <QObject>

class GIdleHandlerPrivate;

class GIdleHandler : public QObject
{
  Q_OBJECT
public:
  explicit GIdleHandler(QObject *parent);
  ~GIdleHandler();

signals:
  /** @brief Parents goes into idle state.
   *
   * Signal is emitted when message queue of parent becomes empty.
   *
   * \warning Do not connect this signal to slot of parent object (i.e. object,
   * passed as a parent to constructor of \ref GIdleHandler) using
   * \ref Qt::QueuedConnection or Qt::BlockingQueuedConnection, because this
   * may result of continuous emission of signal. Signal already is emitted
   * asynchronously.
   */
  void idle();

public slots:

private:
  GIdleHandlerPrivate *d;
};

#endif // GIDLEHANDLER_H
