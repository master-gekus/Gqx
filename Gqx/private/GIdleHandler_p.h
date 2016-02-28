#ifndef GIDLEHANDLER_P_H
#define GIDLEHANDLER_P_H

#include <QTimer>

#include "GIdleHandler.h"

class GIdleHandlerPrivate : public QObject
{
  Q_OBJECT

private:
  explicit GIdleHandlerPrivate(GIdleHandler* owner);
  ~GIdleHandlerPrivate();

private:
  bool eventFilter(QObject *object, QEvent *) Q_DECL_OVERRIDE Q_DECL_FINAL;

private:
  GIdleHandler *owner_;
  QObject *object_;
  QTimer timer_;
  bool is_inside_slot_;

private slots:
  void onTimer();

  friend class GIdleHandler;
};



#endif // GIDLEHANDLER_P_H

