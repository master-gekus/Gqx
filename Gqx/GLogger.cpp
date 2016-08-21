#include <stdio.h>
#include <stdarg.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef _WIN32
  #include <io.h>
  #define LOG_OPEN_MODE	S_IWRITE
  #define LOG_OPEN_FLAGS	O_CREAT | O_WRONLY | O_TEXT
  #define commit _commit
  #pragma warning(disable : 4996)
  #include <Windows.h>
#else
  #include <unistd.h>
  #define commit syncfs
  #define LOG_OPEN_MODE	( S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH )
  #define LOG_OPEN_FLAGS	O_CREAT | O_WRONLY
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

static GMainLoggerThread				*_pMainLoggerThread	= 0;
static GLogger							_pMainLogger;
static int								_nLogEvent			= (-1);
static QMap<int,GOutputLoggerThread*>	_pOutputs;
static int								_nCurOutputChannel	= 0;
static QMutex							_pOutputsMutex( QMutex::NonRecursive );

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
  m_nFlags( nFlags ),
  m_hFile( -1 )
{
  releaseSemaphore();
}

GOutputLoggerThread::~GOutputLoggerThread()
{

}

bool GOutputLoggerThread::beforeExec()
{
  if( !GSelfOwnedThread::beforeExec() ) return false;

  if( ( 0 == ( m_nFlags && GLogger::Reopen ) ) && ( !_open_log() ) ) return false;

  return true;
}

void GOutputLoggerThread::afterExec()
{
  if( 0 != ( m_nFlags && GLogger::Reopen ) ) return;

}

bool GOutputLoggerThread::_open_log()
{
  if( m_strFileName.isEmpty() ) {
    #ifdef _WIN32
      m_hFile = fileno( stdout );
    #else
      m_hFile = STDOUT_FILENO;
    #endif
    return true;
  }

  m_hFile = ::open( m_strFileName.constData(), LOG_OPEN_FLAGS, LOG_OPEN_MODE );
  if( 0 > m_hFile )
    return false;

  lseek( m_hFile, 0, SEEK_END );

  return true;
}

void GOutputLoggerThread::_close_log()
{
  if( !m_strFileName.isEmpty() )
    close( m_hFile );
  m_hFile = (-1);
}

void GOutputLoggerThread::onNewLoggedEvent( GLoggerEvent const& pEvent )
{
  if( ( 0 != ( m_nFlags && GLogger::Reopen ) ) && ( !_open_log() ) ) return;

  // Собсвтенно, запись в лог...
  char *strLine = (char*) alloca( pEvent.message().size() + 128 );
  char *s = strLine;
  if( 0 != ( m_nFlags & GLogger::DateTime ) ) {
    *(s++) = '[';

    if( 0 != ( m_nFlags & GLogger::Date ) ) {
      QDate tDate = pEvent.datetime().date();
      sprintf( s, "%04d/%02d/%02d", tDate.year(), tDate.month(), tDate.day() );
      s += 10;
    };

    if( 0 != ( m_nFlags & GLogger::Time ) ) {
      if( 0 != ( m_nFlags & GLogger::Date ) )
        *(s++) = ' ';

      QTime tTime = pEvent.datetime().time();

      sprintf( s, "%02d:%02d:%02d", tTime.hour(), tTime.minute(), tTime.second() );
      s += 8;
    }

    *(s++) = ']';
    if( 0 != ( m_nFlags & GLogger::Level ) )
      *(s++) = ' ';
  }

  if( 0 != ( m_nFlags & GLogger::Level ) ) {
    sprintf( s, "<%+04i>", pEvent.level() );
    s += 6;
  }
  if( s != strLine ) {
    *(s++) = ':';
    *(s++) = ' ';
  }
  strcpy( s, pEvent.message().constData() );
  s += pEvent.message().size();
  *(s++) = '\n';
  *s = '\0';

  // В случае, если мы в консоли под виндами - надо сделать перекодировочку!
  #ifdef _WIN32
    if( m_strFileName.isEmpty() ) {
      int nLen = ::MultiByteToWideChar( CP_UTF8, 0, strLine, -1, 0, 0 ) + 1;
      wchar_t *wstrLine = (wchar_t*) alloca( nLen * sizeof( wchar_t ) );
      ::MultiByteToWideChar( CP_UTF8, 0, strLine, -1, wstrLine, nLen + 1 );

      // Под результат переаллокировать строку не будем - она по-любому будет
      // не длинее, чем тот же вариант в UTF-8.
      s = strLine +
        ::WideCharToMultiByte( ::GetConsoleCP(), 0, wstrLine, -1, strLine, pEvent.message().size() + 128, 0, 0 )
        - 1;
    }
  #endif

  if( write( m_hFile, strLine, s - strLine ) ){};

  if( 0 != ( m_nFlags && GLogger::Reopen ) ) {
    _close_log();
  } else if( 0 != ( m_nFlags && GLogger::Unbuffered ) ) {
    commit( m_hFile );
  }

//	qDebug() << pEvent.level() << pEvent.datetime() << pEvent.message().constData();
}

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

  _pMainLoggerThread->stopAndWait();
  delete _pMainLoggerThread;
  _pMainLoggerThread = 0;

  // Теперь надо аккуратно завершить все "выводящие" потоки
  QMutexLocker pLock( &_pOutputsMutex );
  foreach( GOutputLoggerThread *pThread, _pOutputs ) {
    pThread->stopAndWait();
    delete pThread;
  }
  _pOutputs.clear();
}

bool
GLogger::isRunning()
{
  return (0 != _pMainLoggerThread);
}

GLogger* GLogger::instance()
{
  return &_pMainLogger;
}

int GLogger::startOutput(QString out_file_name, LogFlags flags)
{
  if( 0 == _pMainLoggerThread ) {
    qDebug( "GLogger::startOutput(): ERROR: GLogger is not started." );
    return (-1);
  }

  if( DefaultFlags == flags )
    flags = out_file_name.isEmpty() ?
      LogFlags( Unbuffered ) : LogFlags( Full | Reopen );

  QByteArray strCanonicalFullName;

  if( !out_file_name.isEmpty() ) {
    QFileInfo pInfo( out_file_name );
    if( pInfo.isDir() ) {
      qDebug( "GLogger::startOutput(): ERROR: \"%s\" is folder.", pInfo.absoluteFilePath().toUtf8().constData() );
      return (-1);
    }

    QFile pFile( pInfo.absoluteFilePath() );
    if( !pFile.open(
        QIODevice::ReadWrite |
        ( ( 0 != ( flags & Overwrite ) ) ? QIODevice::Truncate : QIODevice::Append )
      ) ) {
      qDebug( "GLogger::startOutput(): ERROR: Can not open file \"%s\" for writing. %s",
        pInfo.absoluteFilePath().toUtf8().constData(),
        pFile.errorString().toUtf8().constData()
      );
      return (-1);
    }
    pFile.close();
    strCanonicalFullName = QDir::toNativeSeparators( pFile.fileName() ).toUtf8();
  }

  QMutexLocker pLock( &_pOutputsMutex );

  foreach( int i, _pOutputs.keys() ) {
    GOutputLoggerThread *pThread = _pOutputs[i];
    #ifdef _WIN32
      if( 0 == qstricmp( pThread->m_strFileName.constData(), strCanonicalFullName.constData() ) ) {
    #else
      if( pThread->m_strFileName == strCanonicalFullName  ) {
    #endif

      // Собственно, поток нашли - подправим флаги и вернём индекс
      pThread->m_nFlags = flags;
      return i;
    }
  }

  // Потока нет - стало быть, нужно создать!
  int nNewIndex = ++_nCurOutputChannel;

  GOutputLoggerThread *pThread = new GOutputLoggerThread( strCanonicalFullName, flags );
  _pMainLogger.connect( &_pMainLogger, SIGNAL(newLoggedEvent(GLoggerEvent)), pThread, SLOT(onNewLoggedEvent(GLoggerEvent)), Qt::QueuedConnection );
  _pOutputs[nNewIndex] = pThread;

  return nNewIndex;
}

void GLogger::stopOutput(int channel)
{
  GOutputLoggerThread *pThread = 0;

  {
    QMutexLocker pLock( &_pOutputsMutex );

    if( !_pOutputs.contains( channel ) ) return;

    pThread = _pOutputs[channel];
    _pOutputs.remove( channel );
  }

  pThread->stopAndWait();
  delete pThread;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// GLogger - собственно запись в лог
void GLogger::vwrite(int level, const char *format, va_list args)
{
  if(0 == _pMainLoggerThread)
    {
      qDebug( "GLogger::vwrite():: GLogger not started!" );
      return;
    }


  #ifdef _WIN32
    int nSize = _vscprintf( strFormat, pArgs );
  #else
    va_list pCopy;
    va_copy( pCopy, args );
    int nSize = vsnprintf( 0, 0, format, pCopy );
  #endif

  char* strLine = (char*) alloca( nSize + 3 );

  vsnprintf( strLine, nSize + 3, format, args );

  QCoreApplication::postEvent( _pMainLoggerThread, new GInternalLoggerEvent( level, strLine ) );
}

void GLogger::write(int level, const char *format, ...)
{
  va_list args;
  va_start( args, format );
  vwrite( level, format, args );
  va_end( args );
}

void GLogger::spam(const char *format, ...)
{
  va_list args;
  va_start(args, format);
  vwrite(Spam, format, args);
  va_end(args);
}

void GLogger::chatter(const char *format, ...)
{
  va_list args;
  va_start(args, format);
  vwrite(Chatter, format, args);
  va_end(args);
}

void GLogger::info(const char *format, ...)
{
  va_list args;
  va_start(args, format);
  vwrite(Info, format, args);
  va_end(args);
}

void GLogger::warning(const char *format, ...)
{
  va_list args;
  va_start(args, format);
  vwrite(Warning, format, args);
  va_end(args);
}

void GLogger::error(const char *format, ...)
{
  va_list args;
  va_start(args, format);
  vwrite(Error, format, args);
  va_end(args);
}

void GLogger::critical(const char *format, ...)
{
  va_list args;
  va_start(args, format);
  vwrite(Critical, format, args);
  va_end(args);
}
