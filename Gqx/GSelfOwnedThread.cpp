#include <QWaitCondition>
#include <QMutex>

#include "GSelfOwnedThread.h"

class GSelfOwnedThreadPrivate
{
private:
	GSelfOwnedThreadPrivate() :
		m_pDummy( 0 ),
		m_pMutex( QMutex::NonRecursive ),
		m_bDummy( false )
	{}

	~GSelfOwnedThreadPrivate()
	{
		if( 0 != m_pDummy )
			delete m_pDummy;
	}

private:
	QObject			*m_pDummy;		// Объект необходим, чтобы поменять принадлежность элементов потоку.
	QWaitCondition	m_pWait;
	QMutex			m_pMutex;
	bool			m_bDummy;

	friend class GSelfOwnedThread;
};

GSelfOwnedThread::GSelfOwnedThread() :
	QThread( 0 ),
	d( new GSelfOwnedThreadPrivate() )
{
	start();

	d->m_pMutex.lock();
	forever {
		if( !d->m_bDummy ) {
			d->m_pWait.wait( &d->m_pMutex );
		} else {
			d->m_pMutex.unlock();
			break;
		}
	}

	Q_ASSERT( 0 != d->m_pDummy );
	moveToThread( d->m_pDummy->thread() );
}

GSelfOwnedThread::~GSelfOwnedThread()
{
	delete d;
}

void GSelfOwnedThread::run()
{
	d->m_pDummy = new QObject;

	d->m_pMutex.lock();
	d->m_bDummy = true;
	d->m_pMutex.unlock();

	d->m_pWait.wakeOne();

	d->m_pMutex.lock();
	forever {
		if( d->m_bDummy ) {
			d->m_pWait.wait( &d->m_pMutex );
		} else {
			d->m_pMutex.unlock();
			break;
		}
	}

	if( beforeExec() ) {
		exec();
		afterExec();
	};
}

void GSelfOwnedThread::releaseSemaphore()
{
	d->m_pMutex.lock();
	d->m_bDummy = false;
	d->m_pMutex.unlock();

	d->m_pWait.wakeOne();
}

bool GSelfOwnedThread::beforeExec()
{
	return true;
}

void GSelfOwnedThread::afterExec()
{
}

void GSelfOwnedThread::stopAndWait( int nExitCode /*= 0*/, unsigned long nWaitTime /*= ULONG_MAX*/ )
{
	QMetaObject::invokeMethod( this, "_exit", Qt::QueuedConnection, Q_ARG( int, nExitCode ) );
	QThread::wait( nWaitTime );
}

void GSelfOwnedThread::_exit( int nExitCode )
{
	QThread::exit( nExitCode );
}
