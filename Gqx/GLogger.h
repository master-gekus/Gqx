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
		Min			= (-127),
		Spam		= Min,
		Chatter		= (-64),
		Info		= 0,
		Warning		= 32,
		Error		= 64,
		Critical	= 127,
		Max			= 127
	};

	enum LogFlag {
		Date			= 0x0001,
		Time			= 0x0002,
		DateTime		= Date | Time,
		Level			= 0x0004,
		Full			= Level | DateTime,

		Unbuffered		= 0x0100,
		Reopen			= 0x0200,
		Overwrite		= 0x0400,
		NoLogFlags		= 0x0000
	};
	Q_DECLARE_FLAGS( LogFlags, LogFlag )

signals:
	void newLoggedEvent( GLoggerEvent const& pEvent );

public:
	static GLogger* instance();

public:
	static void start();
	static void stop();

	static int startOutput( QString strFileName = QString(), LogFlags nFlags = LogFlags( Full | Reopen ) );
	static void stopOutput( int nChannel );

	static void vwrite( int nLevel, const char *strFormat, va_list pArgs );

	static void GQX_PRINTF_ATTR(2,3) write( int nLevel, const char *strFormat, ... );

	static void GQX_PRINTF_ATTR(1,2) spam(		const char *strFormat, ... );
	static void GQX_PRINTF_ATTR(1,2) chatter(	const char *strFormat, ... );
	static void GQX_PRINTF_ATTR(1,2) info(		const char *strFormat, ... );
	static void GQX_PRINTF_ATTR(1,2) warning(	const char *strFormat, ... );
	static void GQX_PRINTF_ATTR(1,2) error(		const char *strFormat, ... );
	static void GQX_PRINTF_ATTR(1,2) critical(	const char *strFormat, ... );

};

#endif // GLOGGER_H
