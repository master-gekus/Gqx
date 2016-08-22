#ifndef GLOGGER_H
#define GLOGGER_H

#include <QObject>
#include <QFlags>

class QEvent;
class GLoggerEvent;

#ifdef _WIN32
  #define GQX_PRINTF_ATTR(n,m)
#else
  #define GQX_PRINTF_ATTR(n,m) __attribute__((format(printf,n,m)))
#endif

class GLogger : public QObject
{
  friend class GMainLoggerThread;

  Q_OBJECT
public:
  enum LevelEnum {
    Min       = (-127),
    Spam      = Min,
    Chatter   = (-64),
    Info      = 0,
    Warning   = 32,
    Error     = 64,
    Critical  = 127,
    Max       = 127
  };

  enum LogFlag {
    Date      = 0x0001,
    Time      = 0x0002,
    DateTime  = Date | Time,
    Level     = 0x0004,
    Full      = Level | DateTime,

    Unbuffered  = 0x0100,
    Reopen      = 0x0200,
    Overwrite   = 0x0400,

    DefaultFlags  = 0xFFFF,
    NoLogFlags    = 0x0000
  };
  Q_DECLARE_FLAGS(LogFlags, LogFlag)

signals:
  void newLoggedEvent(GLoggerEvent const& event);

public:
  static GLogger* instance();

public:
  static void start();
  static void stop();
  static bool isRunning();

  static int startOutput(QString out_file_name = QString(),
                         LogFlags flags = DefaultFlags,
                         qint8 min_level = Min);
  static void stopOutput(int channel);

  static void vwrite(int level, const char *format, va_list args);

  static void GQX_PRINTF_ATTR(2,3) write(int level, const char *format, ...);

  static void GQX_PRINTF_ATTR(1,2) spam(const char *format, ...);
  static void GQX_PRINTF_ATTR(1,2) chatter(const char *format, ...);
  static void GQX_PRINTF_ATTR(1,2) info(const char *format, ...);
  static void GQX_PRINTF_ATTR(1,2) warning(const char *format, ...);
  static void GQX_PRINTF_ATTR(1,2) error(const char *format, ...);
  static void GQX_PRINTF_ATTR(1,2) critical(const char *format, ...);
};

#endif // GLOGGER_H
