#include <stdio.h>
#include <stdarg.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef _WIN32
	#include <io.h>
	#define LOG_OPEN_MODE	S_IWRITE
	#define commit _commit
	#pragma warning(disable : 4996)
	#include <Windows.h>
#else
	#include <unistd.h>
	#define commit syncfs
	#define LOG_OPEN_MODE	( S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH )
#endif

#include <QCoreApplication>
#include <QByteArray>
#include <QDateTime>
#include <QEvent>
#include <QThread>
#include <QMap>
#include <QMutex>
#include <QDir>
#include <QFile>
#include <QFileInfo>

#include <QtDebug>

#include "GLogger.h"
#include "GLoggerEvent.h"
#include "GSelfOwnedThread.h"

#include "private/GLogger_p.h"
#include "private/GLoggerEvent_p.h"

class GMainLoggerThread;

static GMainLoggerThread	*_pMainLoggerThread	= 0;
static GLogger				_pMainLogger;
static int					_nLogEvent			= (-1);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// GInternalLoggerEvent - событие для передачи основному потоку
class GInternalLoggerEvent : public QEvent
{
public:
	GInternalLoggerEvent( qint8 nLevel, const char *strMessage ) :
		QEvent( (Type) _nLogEvent ),
		m_pData( new GLoggerEventPrivate( nLevel, strMessage ) )
	{
	}

public:
	GLoggerEventPrivate *data() const { return m_pData; }

private:
	GLoggerEventPrivate *m_pData;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// GMainLoggerThread - основной поток приёма и распределения событий
class GMainLoggerThread : public GSelfOwnedThread
{
private:
	GMainLoggerThread()
	{
		releaseSemaphore();
	}

	bool event( QEvent *pEvent )
	{
		if( pEvent->type() != _nLogEvent )
			return GSelfOwnedThread::event( pEvent );

		pEvent->accept();

		GInternalLoggerEvent *pLogEvent = dynamic_cast<GInternalLoggerEvent*>( pEvent );
		if( 0 == pLogEvent )
			return false;

		emit
			_pMainLogger.newLoggedEvent( GLoggerEvent( pLogEvent->data() ) );

		return true;
	}

	friend class GLogger;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// GOutputLoggerThread - потоки вывода лога
GOutputLoggerThread::GOutputLoggerThread( QByteArray const& strFileName, GLogger::LogFlags nFlags ) :
	m_strFileName( strFileName ),
	m_nFlags( nFlags )
{
	releaseSemaphore();
}

GOutputLoggerThread::~GOutputLoggerThread()
{

}

void GOutputLoggerThread::onNewLoggedEvent( GLoggerEvent const& pEvent )
{
	qDebug() << pEvent.level() << pEvent.datetime() << pEvent.message();
}

static QMap<int,GOutputLoggerThread*>	_pOutputs;
static int								_nCurOutputChannel;
static QMutex							_pOutputsMutex( QMutex::NonRecursive );

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// GLogger - запуск-останов
void GLogger::start()
{
	if( 0 > _nLogEvent )
		_nLogEvent = QEvent::registerEventType();

	if( 0 != _pMainLoggerThread ) return;

	_pMainLoggerThread = new GMainLoggerThread();
}

void GLogger::stop()
{
	if( 0 == _pMainLoggerThread ) return;

	QMetaObject::invokeMethod( _pMainLoggerThread, "quit", Qt::QueuedConnection );
	_pMainLoggerThread->wait();
	delete _pMainLoggerThread;
	_pMainLoggerThread = 0;
}

GLogger* GLogger::instance()
{
	return &_pMainLogger;
}

int GLogger::startOutput( QString strFileName, LogFlags nFlags )
{
	QByteArray strCanonicalFullName;

	if( !strFileName.isEmpty() ) {
		QFileInfo pInfo( strFileName );
		if( pInfo.isDir() ) return (-1);
		if( pInfo.exists() ) {
			if( !pInfo.isWritable() ) return (-1);
			strCanonicalFullName = QDir::toNativeSeparators( pInfo.canonicalFilePath() ).toUtf8();
		} else {
			// Файла нет - попробуем его сначала открыть (и заодно очистить, если что)
			QFile pFile( pInfo.absoluteFilePath() );
			if( !pFile.open(
					QIODevice::ReadWrite |
					( ( 0 != ( nFlags & Overwrite ) ) ? QIODevice::Truncate : QIODevice::Append )
				) ) return (-1);
			pFile.close();
			strCanonicalFullName = QDir::toNativeSeparators( pFile.fileName() ).toUtf8();
		}
	}

//	QMutexLocker pLock



	return (-1);
}

void GLogger::stopOutput( int nChannel )
{
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// GLogger - собственно запись в лог
void GLogger::vwrite( int nLevel, const char *strFormat, va_list pArgs )
{
	#ifdef _WIN32
		int nSize = _vscprintf( strFormat, pArgs );
	#else
		int nSize = vsnprintf( 0, 0, strFormat, pArgs );
	#endif

	char* strLine = (char*) alloca( nSize + 3 );

	vsnprintf( strLine, nSize + 3, strFormat, pArgs );

	QCoreApplication::postEvent( _pMainLoggerThread, new GInternalLoggerEvent( nLevel, strLine ) );
}

void GLogger::write( int nLevel, const char *strFormat, ... )
{
	va_list args;
	va_start( args, strFormat );
	vwrite( nLevel, strFormat, args );
	va_end( args );
}

void GLogger::spam( const char *strFormat, ... )
{
	va_list args;
	va_start( args, strFormat );
	vwrite( Spam, strFormat, args );
	va_end( args );
}

void GLogger::chatter( const char *strFormat, ... )
{
	va_list args;
	va_start( args, strFormat );
	vwrite( Chatter, strFormat, args );
	va_end( args );
}

void GLogger::info( const char *strFormat, ... )
{
	va_list args;
	va_start( args, strFormat );
	vwrite( Info, strFormat, args );
	va_end( args );
}

void GLogger::warning( const char *strFormat, ... )
{
	va_list args;
	va_start( args, strFormat );
	vwrite( Warning, strFormat, args );
	va_end( args );
}

void GLogger::error( const char *strFormat, ... )
{
	va_list args;
	va_start( args, strFormat );
	vwrite( Error, strFormat, args );
	va_end( args );
}

void GLogger::critical( const char *strFormat, ... )
{
	va_list args;
	va_start( args, strFormat );
	vwrite( Critical, strFormat, args );
	va_end( args );
}
