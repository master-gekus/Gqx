#include <QThread>
#include <QDateTime>
#include <QByteArray>

#include "GLoggerEvent.h"
#include "GLogger.h"
#include "private/GLoggerEvent_p.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Класс для регистрации типов
static class _gqx_logger_event_init_class {
public:
	_gqx_logger_event_init_class()
	{
		qRegisterMetaType<GLoggerEvent>( "GLoggerEvent" );
	}
} _gqx_logger_event_init;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// GLoggerEvent - необходимый минимум для QExplicitlySharedDataPointer
GLoggerEvent::GLoggerEvent() :
	m_d( new GLoggerEventPrivate( 0, 0 ) )
{
}

GLoggerEvent::GLoggerEvent( GLoggerEventPrivate *pSource ) :
	m_d( pSource )
{
}

GLoggerEvent::GLoggerEvent( const GLoggerEvent &pSource ) :
	m_d( pSource.m_d )
{
}

GLoggerEvent::~GLoggerEvent()
{
}

GLoggerEvent &GLoggerEvent::operator=( const GLoggerEvent &pSource )
{
	if( this != &pSource )
		m_d.operator=( pSource.m_d );
	return *this;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// GLoggerEvent - получение данных
qint8 GLoggerEvent::level() const
{
	return m_d->m_nLevel;
}

QDateTime GLoggerEvent::datetime() const
{
	return m_d->m_tTime;
}

QByteArray GLoggerEvent::message() const
{
	return m_d->m_strMessage;
}

QThread *GLoggerEvent::thread() const
{
	return m_d->m_pThread;
}
