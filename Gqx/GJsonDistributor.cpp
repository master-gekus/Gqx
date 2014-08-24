#include <QMetaMethod>

#include "GJson.h"
#include "GJsonDistributor.h"

#ifdef QT_DEBUG

//////////////////////////////////////////////////////////////////////////////
/// GDistributableJsonPrivate - внутренние данные для GDistributableJson
class GDistributableJsonPrivate : public QSharedData
{
public:
	GDistributableJsonPrivate( GJson const& pJson ) :
		m_pJson( pJson ),
		m_bProcessed( false )
	{
	}

	~GDistributableJsonPrivate()
	{
		if( !m_bProcessed )
			qDebug( "Unprocessed Json:\n%s", m_pJson.toJson().constData() );
	}

private:
	GJson	m_pJson;
	bool	m_bProcessed;

private:
	GDistributableJsonPrivate();
	Q_DISABLE_COPY(GDistributableJsonPrivate)

	friend class GJsonDistributor;
};
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
/// GDistributableJson - конструктор/деструктор
GDistributableJson::GDistributableJson() :
	d( 0 )
{
	Q_ASSERT( false );
}

GDistributableJson::GDistributableJson( GJson const& pJson ) :
	d( new GDistributableJsonPrivate( pJson ) )
{
}

GDistributableJson::GDistributableJson( GDistributableJson const& pSource ) :
	d( pSource.d )
{
}

GDistributableJson::~GDistributableJson()
{
}
//////////////////////////////////////////////////////////////////////////////

#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Класс для регистрации типов
static class _gqx_json_distributor_init_class {
public:
	_gqx_json_distributor_init_class()
	{
		qRegisterMetaType<GDistributableJson>( "GDistributableJson" );
	}
} _gqx_json_distributor_init;

//////////////////////////////////////////////////////////////////////////////
/// GJsonDistributor - конструктор/деструктор
GJsonDistributor::GJsonDistributor( QObject *pSource, const char *strSignal, QObject *pTarget ) :
	m_pReciver( pTarget )
{
	Q_ASSERT( 0 != pSource );
	Q_ASSERT( 0 != m_pReciver );
	connect( pSource, strSignal, SLOT(onJsonRecieved(GDistributableJson)), Qt::QueuedConnection );
	connect( m_pReciver, SIGNAL(destroyed()), SLOT(onRecieverDestroyed()) );
}

GJsonDistributor::~GJsonDistributor()
{
}

//////////////////////////////////////////////////////////////////////////////
/// GJsonDistributor - слоты
void GJsonDistributor::onRecieverDestroyed()
{
	m_pReciver = 0;
	delete this;
}

void GJsonDistributor::onJsonRecieved( GDistributableJson const pJsonDistributable )
{
	if( 0 == m_pReciver ) return;

	#ifdef QT_DEBUG
		GJson const& pJson = pJsonDistributable.d.constData()->m_pJson;
	#else
		GJson const& pJson = pJsonDistributable;
	#endif

	if( !pJson["command"].isSet() ) {
		qDebug( "GJsonDistributor::onJsonRecieved(): Json without \"command\" field - skipped!" );
		return;
	}
	QByteArray strCommand = pJson["command"];
	strCommand[0] = toupper( strCommand[0] );

	// Посмотрим, есть ли такой метод?
	int nMethod = m_pReciver->metaObject()->indexOfSlot( QMetaObject::normalizedSignature(
		"on" + strCommand + "(GJson const&)"
	));
	if( nMethod < 0 ) return;

	QMetaMethod pMethod = m_pReciver->metaObject()->method( nMethod );
	bool bProcessed = true;
	if( 0 == strcmp( pMethod.typeName(), "bool" ) ) {
		// Метод возвращает значение типа bool
		pMethod.invoke(
			m_pReciver,
			Qt::DirectConnection,
			Q_RETURN_ARG( bool, bProcessed ),
			Q_ARG( GJson const&, pJson )
		);
	} else {
		// Метод ничего не возвращает - значит, просто считаем, что обработано
		pMethod.invoke(
			m_pReciver,
			Qt::DirectConnection,
			Q_ARG( GJson const&, pJson )
		);
	}

	#ifdef QT_DEBUG
		if( bProcessed )
			const_cast<GDistributableJsonPrivate*>( pJsonDistributable.d.constData() )->m_bProcessed = true;
	#endif
}
