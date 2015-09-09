#ifndef GJSONDISTRIBUTOR_H
#define GJSONDISTRIBUTOR_H

#include <QObject>
#include <QMetaType>
#include <QExplicitlySharedDataPointer>

class GJson;

//////////////////////////////////////////////////////////////////////////////
/// \brief "Распределяемый" Json
///
/// Этот класс нужен только в отладочной версии - для того, чтобы ловить
/// "нераспределённые" Json-ы. В релизе он заменяется на просто GJson
#ifdef QT_DEBUG
	class GDistributableJsonPrivate;

	class GDistributableJson
	{
	public:
		GDistributableJson();
		GDistributableJson( GJson const& pJson );
		GDistributableJson( GDistributableJson const& pSource );

		~GDistributableJson();

	private:
		QExplicitlySharedDataPointer<GDistributableJsonPrivate> d;

		friend class GJsonDistributor;
	};
	Q_DECLARE_METATYPE( GDistributableJson )
#else
	typedef GJson GDistributableJson;
#endif
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
/// \brief Класс-распределитель GJson-ов по объектам
class GJsonDistributor : public QObject
{
	Q_OBJECT
private:
	QObject *m_pReciver;

public:
	GJsonDistributor( QObject *pSource, const char *strSignal, QObject *pTarget );
	virtual ~GJsonDistributor();

private slots:
	void onJsonRecieved( GDistributableJson const pJsonDistributable );
	void onRecieverDestroyed();
};
//////////////////////////////////////////////////////////////////////////////

#endif // GJSONDISTRIBUTOR_H
