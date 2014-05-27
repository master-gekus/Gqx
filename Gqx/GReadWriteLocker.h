#ifndef GREADWRITELOCKER_H
#define GREADWRITELOCKER_H

class QReadWriteLock;

class GReadWriteLocker
{
public:
	explicit GReadWriteLocker( QReadWriteLock *pLocker, bool bForWrite = false );
	~GReadWriteLocker();

private:
	QReadWriteLock *m_pMutex;
};

#endif // GREADWRITELOCKER_H
