#ifndef GLOGGER_PRIVATE_H
#define GLOGGER_PRIVATE_H

#include <QByteArray>
#include "GSelfOwnedThread.h"
#include "GLogger.h"
#include "GLoggerEvent.h"


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// GOutputLoggerThread - поток вывода лога
class GOutputLoggerThread : public GSelfOwnedThread
{
  Q_OBJECT

private:
  GOutputLoggerThread(const QByteArray &file_name, GLogger::LogFlags flags,
                      qint8 min_level);
  ~GOutputLoggerThread();

private slots:
  void onNewLoggedEvent(GLoggerEvent const& event);

protected:
  bool beforeExec();
  void afterExec();

private:
  QByteArray file_name_;
  GLogger::LogFlags flags_;
  int file_;
  qint8 min_level_;

private:
  bool _open_log();
  void _close_log();

  friend class GLogger;
};

#endif // GLOGGER_PRIVATE_H
