#include <QReadWriteLock>

#include "GReadWriteLocker.h"

GReadWriteLocker::GReadWriteLocker( QReadWriteLock *pLocker, bool bForWrite ) :
	m_pMutex( pLocker )
{
	if( bForWrite )
		m_pMutex->lockForWrite();
	else
		m_pMutex->lockForRead();
}

GReadWriteLocker::~GReadWriteLocker()
{
	m_pMutex->unlock();
}
