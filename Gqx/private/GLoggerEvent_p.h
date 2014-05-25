#ifndef GLOGGEREVENT_PRIVATE_H
#define GLOGGEREVENT_PRIVATE_H

class GLoggerEventPrivate : public QSharedData
{
private:
	GLoggerEventPrivate( qint8 nLevel, const char* strMessage ) :
		m_pThread( QThread::currentThread() ),
		m_nLevel( nLevel ),
		m_tTime( QDateTime::currentDateTime() ),
		m_strMessage( strMessage )
	{
	}

	~GLoggerEventPrivate()
	{
	}

private:
	QThread			*m_pThread;
	qint8			m_nLevel;
	QDateTime		m_tTime;
	QByteArray		m_strMessage;

	friend class GLoggerEvent;
	friend class GInternalLoggerEvent;
	friend class GMainLoggerThread;
	friend class QExplicitlySharedDataPointer<GLoggerEventPrivate>;
};

#endif // GLOGGEREVENT_PRIVATE_H
