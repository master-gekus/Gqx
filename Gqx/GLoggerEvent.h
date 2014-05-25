#ifndef GLOGGEREVENT_H
#define GLOGGEREVENT_H

#include <QExplicitlySharedDataPointer>
#include <QMetaType>

//#include "GLogger.h"

class QByteArray;
class QDateTime;
class QThread;
class GLoggerEventPrivate;

class GLoggerEvent
{
private:
	explicit GLoggerEvent( GLoggerEventPrivate *pEvent );

public:
	GLoggerEvent();
	explicit GLoggerEvent( GLoggerEvent const& pOther );

public:
	GLoggerEvent& operator=( GLoggerEvent const& pOther );

public:
	~GLoggerEvent();

public:
	qint8 level() const;
	QDateTime datetime() const;
	QByteArray message() const;
	QThread* thread() const;

private:
	QExplicitlySharedDataPointer<GLoggerEventPrivate> m_d;

	friend class GLogger;
	friend class GMainLoggerThread;
};

Q_DECLARE_METATYPE( GLoggerEvent )

#endif // GLOGGEREVENT_H
