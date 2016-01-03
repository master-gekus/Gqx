#ifdef _WIN32
  #ifndef _CRT_SECURE_NO_WARNINGS
    #define _CRT_SECURE_NO_WARNINGS
  #endif
  #include <float.h>
  #include <math.h>
  #ifndef isnan
    #define isnan _isnan
  #endif
#else
  #include <math.h>
  #include <stdio.h>
#endif

#include <QSharedData>
#include <QByteArray>
#include <QString>
#include <QVariant>
#include <QVector>
#include <QMap>

#ifndef GQX_JSON_NO_MSGPACK
  #ifdef _WIN32
    #pragma warning( disable: 4267 )
  #endif

  #include <msgpack.h>

  #ifdef _WIN32
    #pragma warning( default: 4267 )
    #pragma comment( lib, "Ws2_32.lib" )		// msgpack оттуда сосёт htohs и htohl
    #if defined( _DEBUG ) || defined( QT_DEBUG )
      #ifdef _WIN64
        #pragma comment( lib, "msgpackx64d.lib" )
      #else
        #pragma comment( lib, "msgpackd.lib" )
      #endif
    #else
      #ifdef _WIN64
        #pragma comment( lib, "msgpackx64.lib" )
      #else
        #pragma comment( lib, "msgpack.lib" )
      #endif
    #endif
  #endif
#endif

#include "GJson.h"

/*! **************************************************************************************************************
  \class GJson
  \ingroup json
  \reentrant

  \brief Класс GJson инкапсулирует работу со структурой JSON.

  Класс создан как заместитель появившихся с Qt 5.0 классов работы с JSON (QJsonValue, QJsonArray, QJsonObject).

  Создание класса обусловлено тем, что работа через три разных класса показалась достаточно неудобной, когда
  все действия можно объединить в одном классе. Кроме того, в класс добавлена дополнительная функциональность,
  не предусмотренная в штатной поддержке JSON, а именно:

  \list
  \li Поддержка дополнительного типа Integer - для точного хранения 64-х разрядных целых чисел. В версии Qt
    целые числа хранятся как qouble, что может привести к потере данных для больших чисел.
  \li Добавлена поддержка NaN (Not a Number) для типа double
  \li Поддержка хранения бинарных данных (без перекодировки) в объектах типа String
  \li Более свободный синтаксис при преобразовании из текста (допускаются запятые перед закрывающими "]" и
    "}", допускается указание имён полей объекта без кавычек, добавлена поддержка комментариев (до конца
    строки после символов "//")
  \li Расширенный вариант форматирования при преобразовании в текст
  \li Поддержка (отключаемая) сериализации в формат данных msgpack
  \li Поддержка (отключаемая) дополнительного оператора ()
  \endlist

  Макросы для отключения дополнительных возможностей
  \list
  \li	GQX_JSON_NO_MSGPACK	Отключить использование библиотеки msgpack
  \endlist
*****************************************************************************************************************/

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Класс для регистрации типов
static class _gqx_json_init_class {
public:
  _gqx_json_init_class()
  {
    qRegisterMetaType<GJson>( "GJson" );
  }
} _gqx_json_init;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// GJsonPrivate class
class GJsonPrivate : public QSharedData
{
private:
  GJsonPrivate( GJsonPrivate const& pSource ) :
    QSharedData(),
    m_nType( pSource.m_nType ),
    m_bValue( pSource.m_bValue ),
    m_iValue( pSource.m_iValue ),
    m_dValue( pSource.m_dValue ),
    m_strValue( pSource.m_strValue ),
    m_pArray( pSource.m_pArray ),
    m_pMap( pSource.m_pMap )
  {
    qDebug("GJsonPrivate: copy constructor called!");
  }

  GJsonPrivate( GJson::Type nType ) :
    m_nType( nType ),
    m_bValue( false ),
    m_iValue( 0 ),
    m_dValue( 0.0 ),
    m_strValue( GJson::Null == nType ? "null" : "" )
  {
  }

  GJsonPrivate( bool bValue ) :
    m_nType( GJson::Bool ),
    m_bValue( bValue ),
    m_iValue( bValue ? 1 : 0 ),
    m_dValue( bValue ? 1.0 : 0.0 ),
    m_strValue( bValue ? "true" : "false" )
  {
  }

  GJsonPrivate( double dValue ) :
    m_nType( GJson::Double ),
    m_bValue( (!isnan( dValue )) && ( dValue != 0.0 ) ),
    m_iValue( isnan( dValue ) ? 0 : int( dValue + 0.5 ) ),
    m_dValue( dValue ),
    m_strValue( QByteArray::number( dValue, 'g', 18 ) )
  {
  }

  GJsonPrivate( qint64 nValue ) :
    m_nType( GJson::Integer ),
    m_bValue( nValue != 0 ),
    m_iValue( nValue ),
    m_dValue( nValue ),
    m_strValue( QByteArray::number( nValue ) )
  {
  }

  GJsonPrivate( quint64 nValue ) :
    m_nType( GJson::Integer ),
    m_bValue( nValue != 0 ),
    m_iValue( nValue ),
    m_dValue( nValue ),
    m_strValue( QByteArray::number( nValue ) )
  {
  }

  GJsonPrivate( const QString &strValue ) :
    m_nType( GJson::String ),
    m_bValue( false ),
    m_iValue( 0 ),
    m_dValue( 0.0 ),
    m_strValue( strValue.toUtf8() )
  {
    _update_values_by_string();
  }

  GJsonPrivate( const QByteArray &pValue ) :
    m_nType( GJson::String ),
    m_bValue( false ),
    m_iValue( 0 ),
    m_dValue( 0.0 ),
    m_strValue( pValue )
  {
    _update_values_by_string();
  }

  GJsonPrivate( const QVariant &vValue ) :
    m_nType( GJson::Undefined ),
    m_bValue( false ),
    m_iValue( 0 ),
    m_dValue( 0.0 )
  {
    switch( vValue.type() ) {
      case QVariant::Bool:
        m_nType		= GJson::Bool;
        m_bValue	= vValue.toBool();
        m_iValue	= m_bValue ? 1 : 0;
        m_dValue	= m_bValue ? 1.0 : 0.0;
        m_strValue	= m_bValue ? "true" : "false";
        break;

      case QVariant::Int:
      case QVariant::UInt:
      case QVariant::LongLong:
      case QVariant::ULongLong:
        m_nType		= GJson::Integer;
        m_iValue	= vValue.toULongLong();
        m_dValue	= m_iValue;
        m_bValue	= ( 0 != m_iValue );
        m_strValue	= vValue.toString().toUtf8();
        break;

      case QVariant::Double:
        m_nType		= GJson::Double;
        m_iValue	= vValue.toULongLong();
        m_dValue	= m_iValue;
        m_bValue	= ( 0 != m_iValue );
        m_strValue	= vValue.toString().toUtf8();
        break;

      case QVariant::Char:
      case QVariant::String:
        m_nType		= GJson::String;
        m_strValue	= vValue.toString().toUtf8();
        _update_values_by_string();
        break;

      case QVariant::ByteArray:
        m_nType		= GJson::String;
        m_strValue	= vValue.toByteArray();
        _update_values_by_string();
        break;

      default:
        if( !vValue.canConvert<QByteArray>() ) break;
        m_nType		= GJson::String;
        m_strValue	= vValue.toByteArray();
        _update_values_by_string();
        break;
    }

  }

  void _update_values_by_string()
  {
    if( m_strValue.isEmpty() ) return;

    if(	( 0 == qstricmp( m_strValue.constData(), "n" ) ) ||
      ( 0 == qstricmp( m_strValue.constData(), "no" ) ) ||
      ( 0 == qstricmp( m_strValue.constData(), "false" ) ) )
      return;

    if(	( 0 == qstricmp( m_strValue.constData(), "y" ) ) ||
      ( 0 == qstricmp( m_strValue.constData(), "yes" ) ) ||
      ( 0 == qstricmp( m_strValue.constData(), "true" ) ) ) {
      return;
    }

    bool bOk;
    m_iValue = m_strValue.toLongLong( &bOk );
    if( !bOk ) m_iValue = 0; else m_bValue = ( 0 != m_iValue );

    m_dValue = m_strValue.toDouble( &bOk );
    if( !bOk ) m_dValue = 0; else m_bValue = ( 0.0 != m_dValue );
  }

  ~GJsonPrivate()
  {
  }

// Поддержка преобразования в строку
private:
  struct ToJsonContext;
  void to_json( int nIndent, ToJsonContext *pContext ) const;
  void string_to_json( int nIndent, ToJsonContext *pContext ) const;
  void array_to_json( int nIndent, ToJsonContext *pContext ) const;
  void object_to_json( int nIndent, ToJsonContext *pContext ) const;
  void object_element_to_json(int nIndent, ToJsonContext *pContext, const char *strName, int nLen, GJsonPrivate const& pValue, int nMaxWidth ) const;
  static void _to_oct( char* &d, unsigned char b );

// Поддержка разбора строки
private:
  struct FromJsonContext;

// Поддержка формата сериализации msgpack
private:
  #ifndef GQX_JSON_NO_MSGPACK
    void msgpack( msgpack_packer *pk ) const;
    static GJsonPrivate *msgunpack( msgpack_object const& pk );
  #endif

private:
  /*!	\brief Класс-ключ для QMap - чтобы не использовать достаточно "тяжёлый" класс QByteArray

    Собственно, класс представляет собой некоторую надстройку надо обычным char* - в частности,
    автоматически выполняет free.

    Для класса даже не требуется конструтора от char* - достаточно конструктора копирования,
    т.к. константная ссылка на экземпляр класса получается обычным приведением типов. При этом
    небольшими приёмами в коде достигает то, что реально объект класса создаётся только при
    добавлении элемента в QMap.
   */
  struct Key {
    char *d;

    explicit Key( Key const& pSource ) :
      d( qstrdup( pSource.d ) )
    {}

    ~Key()
    {
      // В отличии от strdup, qstrdup возвращает указатель, который удаляется
      // через delete! Cм. документацию по qt
      delete[] d;
    }

    bool operator <( Key const& pOther ) const
    {
      return ( qstrcmp( d, pOther.d ) < 0 );
    }
  };

private:
  GJson::Type		m_nType;
  bool			m_bValue;
  quint64			m_iValue;
  double			m_dValue;
  QByteArray		m_strValue;
  QVector<GJson>	m_pArray;
  QMap<Key,GJson>	m_pMap;

  friend class GJson;
  friend class QSharedDataPointer<GJsonPrivate>;
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// GJson - минимальные необходимые методы для QSharedData
GJson::GJson( Type nType /*= Null */ ) :
  m_d( new GJsonPrivate( nType ) )
{
}

GJson::GJson( GJsonPrivate *pSource ) :
  m_d( pSource )
{
}

GJson::~GJson()
{
}

GJson::GJson( const GJson &pSource ) :
  m_d( pSource.m_d )
{
}

GJson &GJson::operator=( const GJson &pSource )
{
  if( this != &pSource )
    m_d.operator=( pSource.m_d );
  return *this;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// GJson - конструкторы от основных типов
GJson::GJson( bool bValue ) :
  m_d( new GJsonPrivate( bValue ) )
{
}

GJson::GJson( double dValue ) :
  m_d( new GJsonPrivate( dValue ) )
{
}

GJson::GJson( int nValue ) :
  m_d( new GJsonPrivate( (qint64) nValue ) )
{
}

GJson::GJson( unsigned int nValue ) :
  m_d( new GJsonPrivate( (quint64) nValue ) )
{
}

GJson::GJson( qint64 nValue ) :
  m_d( new GJsonPrivate( nValue ) )
{
}

GJson::GJson( quint64 nValue ) :
  m_d( new GJsonPrivate( nValue ) )
{

}

GJson::GJson( const QString &strValue ) :
  m_d( new GJsonPrivate( strValue ) )
{

}

GJson::GJson( const char* strValue ) :
  m_d( new GJsonPrivate( QByteArray( strValue ) ) )
{

}

GJson::GJson( const QByteArray &pValue ) :
  m_d( new GJsonPrivate( pValue ) )
{

}

GJson::GJson( const QVariant &vValue ) :
  m_d( new GJsonPrivate( vValue ) )
{

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// GJson - получение значений
GJson::Type GJson::type() const
{
  return m_d->m_nType;
}

bool GJson::toBool() const
{
  return m_d->m_bValue;
}

int GJson::toInt() const
{
  return m_d->m_iValue;
}

unsigned int GJson::toUInt() const
{
  return m_d->m_iValue;
}

long long GJson::toLongLong() const
{
  return m_d->m_iValue;
}

unsigned long long GJson::toULongLong() const
{
  return m_d->m_iValue;
}

double GJson::toDouble() const
{
  return m_d->m_dValue;
}

QString GJson::toString() const
{
  return QString::fromUtf8( m_d->m_strValue );
}

QByteArray GJson::toByteArray() const
{
  return m_d->m_strValue;
}

const char* GJson::toStr() const
{
  return m_d->m_strValue.constData();
}

QVariant GJson::toVariant() const
{
  switch( m_d->m_nType ) {
    case Bool:		return QVariant( m_d->m_bValue );
    case Integer:	return QVariant( m_d->m_iValue );
    case Double:	return QVariant( m_d->m_dValue );
    case String:	return QVariant( QString::fromUtf8( m_d->m_strValue ) );
    default:		return QVariant();
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// GJson - работа с массивами и объектами

/*!	\brief	Количество элементов в массиве или объекте
  \return	Возвращает количество элеменов в массиве или объекте, или 0, если тип JSON не
      массив (Array) или объект (Object).
*/
int GJson::count() const
{
  switch( m_d->m_nType ) {
    case Array:		return m_d->m_pArray.size();
    case Object:	return m_d->m_pMap.size();
    default:		return 0;
  }
}

/*!
 * \brief Список ключей объета
 * \return  Возвращает списко ключей, если JSON является объектом (Object).
 *          В остальных случах возвращается пустой список. Порядок ключей в
 *          массиве не определён!
 */

QList<QByteArray> GJson::keys() const
{
  QList<QByteArray> list;

  if (Object == m_d->m_nType)
    for(QMap<GJsonPrivate::Key,GJson>::const_iterator i = m_d->m_pMap.constBegin();
        i != m_d->m_pMap.constEnd(); ++i)
      list.append(QByteArray(i.key().d));

  return list;
}

/*!	\brief	Изменить размер массива
  \param	nNewSize	Новый размер массива

  Изменяет размер массива. В случае, если тип JSON не массив (Array), вызывается assertion
*/
void GJson::resize( int nNewSize )
{
  if( ( Undefined == m_d->m_nType ) || ( Null == m_d->m_nType ) )
    m_d->m_nType = Array;

  Q_ASSERT( Array == m_d->m_nType );
  Q_ASSERT( 0 <= nNewSize );

  m_d->m_pArray.resize( nNewSize );
}

/*!	\brief	Элемент массива
  \param	nIndex	Индекс элемента (от 0)
  \return	Копию элемента массива.

  Если тип JSON не массив (Array) или индекс выходит за границы массива, возвращается JSON типа Undefined.
*/
GJson const GJson::at( int nIndex ) const
{
  if( ( Array != m_d->m_nType ) || ( 0 > nIndex ) || ( m_d->m_pArray.size() <= nIndex ) )
    return GJson( GJson::Undefined );

  return m_d->m_pArray[nIndex];
}

/*!	\brief	Ссылка на элемент массива
  \param	nIndex	Индекс элемента
  \return	Ссылку на элемент массива

  Возвращает ссылку на элемент массива для возможности дальнейшего изменения элемента.
  Если тим JSON не массив (Array) или индекс выходит за границы массива, генерируется assertion.
*/
GJson& GJson::at( int nIndex )
{
  Q_ASSERT( ( Array == m_d->m_nType ) && ( 0 <= nIndex ) && ( m_d->m_pArray.size() > nIndex ) );
  return m_d->m_pArray[nIndex];
}

/*!	\brief	Элемент объекта
  \param	strKey	Ключ элемента
  \return Копию элемента объекта

  Если тип JSON не объект (Object) или элемент с указанным ключом не найден, возвращается JSON типа Undefined.
*/
GJson const GJson::at(const char *strKey ) const
{
  if( Object != m_d->m_nType )
    return GJson( GJson::Undefined );

  QMap<GJsonPrivate::Key,GJson>::const_iterator i = m_d->m_pMap.constFind( *( (GJsonPrivate::Key const*)(&strKey) ) );

  if( i == m_d->m_pMap.constEnd() )
    return GJson( GJson::Undefined );

  return *i;
}

/*!	\brief	Ссылка на элемент объекта
  \param	strKey	Ключ элемента
  \return Ссылку на элемент объекта

  Если тип JSON не объект (Object), то для типов Null и Undefined JSON автоматически преобразуется в объет, иначе
  генерируется assertion.
  Если элемент с указаным ключом не найден, то он создаётся.
*/
GJson& GJson::at( const char *strKey )
{
  if( ( Undefined == m_d->m_nType ) || ( Null == m_d->m_nType ) )
    m_d.operator=( new GJsonPrivate( GJson::Object ) );

  Q_ASSERT( Object == m_d->m_nType );

  QMap<GJsonPrivate::Key,GJson>::iterator i = m_d->m_pMap.find( *( (GJsonPrivate::Key const*)(&strKey) ) );

  if( i != m_d->m_pMap.end() )
    return *i;

  return m_d->m_pMap[*( (GJsonPrivate::Key const*)(&strKey) )];
}

void GJson::append( GJson const& pValue )
{
  if( ( Undefined == m_d->m_nType ) || ( Null == m_d->m_nType ) )
    m_d->m_nType = Array;

  Q_ASSERT( Array == m_d->m_nType );

  m_d->m_pArray.append( pValue );
}

void GJson::prepend( GJson const& pValue )
{
  if( ( Undefined == m_d->m_nType ) || ( Null == m_d->m_nType ) )
    m_d->m_nType = Array;

  Q_ASSERT( Array == m_d->m_nType );

  m_d->m_pArray.prepend( pValue );
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// GJson - преобразование в строку
struct GJsonPrivate::ToJsonContext
{
  GJson::JsonFormat	m_nFlags;
  char				*m_pBuffer;
  size_t				m_nAlloc,
            m_nCurrent;

  ToJsonContext()
  {
    m_pBuffer	= 0;
    m_nAlloc	=
    m_nCurrent	= 0;
  }

  ~ToJsonContext()
  {
    if( 0 != m_pBuffer )
      free( m_pBuffer );
  }

  char* get_buffer( size_t nLen, int nIndent = (-1) )
  {
    /// \todo: Задавать размер табуляции!!!
    size_t nTabs	= 0;
    size_t nSpaces	= 0;
    if( nIndent >= 0 ) {
      nTabs	= ( 0 != (m_nFlags & GJson::UseSpaces) ) ? 0 : nIndent / 4;
      nSpaces	= ( 0 != (m_nFlags & GJson::UseSpaces) ) ? nIndent : ( nIndent % 4 );
      nLen += ( nTabs + nSpaces + 2 );
    };

    if( ( m_nCurrent + nLen ) >= m_nAlloc ) {
      m_nAlloc += ( ( ( nLen + m_nCurrent + 1 - m_nAlloc ) / 1024 ) + 1 ) * 1024;
      m_pBuffer = (char*) realloc( m_pBuffer, m_nAlloc );
      memset( m_pBuffer + m_nCurrent, 0, m_nAlloc - m_nCurrent );
    }

    if( nIndent >= 0 ) {
      m_pBuffer[m_nCurrent++] = '\r';
      m_pBuffer[m_nCurrent++] = '\n';
      while( nTabs-- )
        m_pBuffer[m_nCurrent++] = '\t';
      while( nSpaces-- )
        m_pBuffer[m_nCurrent++] = ' ';
    }

    return m_pBuffer + m_nCurrent;
  }

  void release_buffer( size_t nLen = 0 )
  {
    if( 0 == nLen )
      nLen = strlen( m_pBuffer + m_nCurrent );
    m_nCurrent += nLen;
    m_pBuffer[m_nCurrent] = '\0';
  }

  void append( char const* pToAdd, size_t nLen = 0 )
  {
    if( 0 == nLen )
      nLen = strlen( pToAdd );
    memmove( get_buffer( nLen ), pToAdd, nLen );
    m_nCurrent += nLen;
    m_pBuffer[m_nCurrent] = '\0';
  }
};

void GJsonPrivate::to_json( int nIndent, ToJsonContext *pContext ) const
{
  switch( m_nType ) {
    case GJson::Integer:
      sprintf( pContext->get_buffer( 64 ), "%lld", m_iValue );
      pContext->release_buffer();
      break;

    case GJson::Double:
      sprintf( pContext->get_buffer( 64 ), "%g", m_dValue );
      pContext->release_buffer();
      break;

    case GJson::Bool:
      if( m_bValue )
        pContext->append( "true" , 4 );
      else
        pContext->append( "false" , 5 );
      break;

    case GJson::String:
      string_to_json( nIndent, pContext );
      break;

    case GJson::Array:
      array_to_json( nIndent, pContext );
      break;

    case GJson::Object:
      object_to_json( nIndent, pContext );
      break;

    default:
      pContext->append( "null" , 4 );
      break;
  };
}

void GJsonPrivate::array_to_json( int nIndent, ToJsonContext *pContext ) const
{
  bool bSimpleArray = true;
  if( 0 != ( GJson::Indented & pContext->m_nFlags ) ) {
    if( ( 0 == ( GJson::InlineEmptyArrays & pContext->m_nFlags ) ) ||
      ( 0 != m_pArray.size() ) ) {
      if( 0 != ( GJson::InlineSimpleArrays & pContext->m_nFlags ) ) {
        foreach( GJson const& pElement, m_pArray ) {
          if( pElement.isCompound() ) {
            bSimpleArray = false;
            break;
          }
          if( pElement.isString() && ( 32 < pElement.m_d->m_strValue.size() ) ) {
            bSimpleArray = false;
            break;
          }
        }
      } else
        bSimpleArray = false;
    }
  };

  if( (!bSimpleArray) && ( 0 != pContext->m_nCurrent ) )
    pContext->get_buffer( 0, nIndent );
  pContext->append("[");

  for( int i = 0; i < m_pArray.size(); i++ ) {
    if( !bSimpleArray )
      pContext->get_buffer( 0, nIndent + 4 );		/// \todo TabSize!!!

    m_pArray[i].m_d->to_json( nIndent + 4, pContext );

    if( i != ( m_pArray.size() - 1 ) ) {
      // Пробел после запятой добавляется для случая "простого"
      // массива при включённой декорации
      if( bSimpleArray && ( 0 != ( GJson::Indented & pContext->m_nFlags ) ) )
        pContext->append(", ");
      else
        pContext->append(",");
    }
  }
  if( !bSimpleArray )
    pContext->get_buffer( 0, nIndent );
  pContext->append("]");
}

void GJsonPrivate::object_to_json( int nIndent, ToJsonContext *pContext ) const
{
  if( 0 == m_pMap.size() ) {
    // Пустой объект
    if( ( 0 == ( GJson::Indented & pContext->m_nFlags ) ) ||
      ( 0 != ( GJson::InlineEmptyObjects & pContext->m_nFlags ) ) ) {
      pContext->append("{}");
      return;
    }
  };

  // Получим список ключей - и заодно список длин этих ключей!
  QList<const char *>	pKeys;
  QList<int>			pKeyLens;
  for( QMap<GJsonPrivate::Key,GJson>::const_iterator i = m_pMap.constBegin(); i != m_pMap.constEnd(); ++i ) {
    pKeys.push_back( (const char*) i.key().d );
    pKeyLens.push_back( QObject::trUtf8( pKeys.last() ).length() );
  }

  // Посчитаем максимальную длину
  int nMaxWidth	= (-1);

  if( 0 != ( GJson::AlignObjects & pContext->m_nFlags ) )
    foreach( int nLen, pKeyLens )
      nMaxWidth = qMax( nMaxWidth, nLen );

//	if( 0 != pContext->m_nCurrent )
//		pContext->get_buffer( 0, nIndent );

  pContext->append("{");

  // Сначала выведеме все простые элементы - и посмотрим, есть ли сложные?
  bool bHasObjects = false;
  for( int i = 0; i < pKeys.size(); i++ ) {
    const char*		strKey		= pKeys[i];
    GJson const&	pElement	= m_pMap[*( (GJsonPrivate::Key const*) (&strKey) )];
    if( ( 0 != ( GJson::SimpleFieldsFirst & pContext->m_nFlags ) ) &&
      pElement.isCompound() ) {
      bHasObjects = true;
      continue;
    }

    object_element_to_json( nIndent, pContext, strKey, pKeyLens[i], *( pElement.m_d ), nMaxWidth );

    // Ну и наконец - запятая после объекта.
    if( ( i != ( pKeys.size() - 1 ) ) || bHasObjects )
      pContext->append(",");
  }

  if( bHasObjects ) {
    // Теперь составные элементы
    bool bFirstElement = true;
    for( int i = 0; i < pKeys.size(); i++ ) {
      const char*		strKey		= pKeys[i];
      GJson const&	pElement	= m_pMap[*( (GJsonPrivate::Key const*) (&strKey) )];
      if( !pElement.isCompound() ) continue;

      if( bFirstElement ) {
        bFirstElement = false;
      } else {
        pContext->append(",");
      }

      object_element_to_json( nIndent, pContext, strKey, pKeyLens[i], *( pElement.m_d ), nMaxWidth );
    }
  }

  if( 0 != ( GJson::Indented & pContext->m_nFlags ) )
    pContext->get_buffer( 0, nIndent );
  pContext->append("}");
}

void GJsonPrivate::object_element_to_json( int nIndent, ToJsonContext *pContext, const char* strName, int nLen, GJsonPrivate const& pValue, int nMaxWidth ) const
{
  int nDelta	= 0;
  if( nLen < nMaxWidth ) {
    nDelta	= nMaxWidth - nLen;
    nLen	= nMaxWidth;
  };

  char *d;
  if( 0 != ( GJson::Indented & pContext->m_nFlags ) )
    d = pContext->get_buffer( nLen + 4, nIndent + 4 );		/// \todo Tabsize
  else
    d = pContext->get_buffer( nLen + 3 );
  *(d++) = '"';
  strcpy( d, strName );
  d += qstrlen( d );
  *(d++) = '"';
  *(d++) = ':';
  if( 0 != ( GJson::Indented & pContext->m_nFlags ) ) *(d++) = ' ';
  while( nDelta-- )
    *(d++) = ' ';
  pContext->release_buffer();

  // Имя объекта записано - запишем сам объект!
  if( ( 0 != ( GJson::Indented & pContext->m_nFlags ) ) && ( GJson::String == pValue.m_nType ) ) {
    pValue.to_json( nIndent + nLen + 4 + 4, pContext );	// Здесь не размер табуляции, а просто место под ":"
  } else {
    pValue.to_json( nIndent + 4, pContext );			/// \todo Tabsize
  }
}

void GJsonPrivate::_to_oct( char* &d, unsigned char b )
{
  *(d++) = '\\';
  *(d++) = ( b / ( 8 * 8 ) ) + '0';
  *(d++) = ( ( b / 8 ) % 8 ) + '0';
  *(d++) = ( b % 8 ) + '0';
}

void GJsonPrivate::string_to_json( int nIndent, ToJsonContext *pContext ) const
{
  bool		bNeedSplit	=	( 0 != ( GJson::Indented & pContext->m_nFlags ) ) &&
                ( 0 != ( GJson::SplitStrings & pContext->m_nFlags ) );
  char*		d			=	pContext->get_buffer( m_strValue.size() * 4 + 2 );
  char const*	strStart	=	m_strValue.constData();
  char const*	strEnd		=	strStart + m_strValue.size();
  int			nLen		=	0;

  *(d++) = '"';
  for( char const* s = strStart; s < strEnd; s++ ) {
    if( bNeedSplit ) {
      // Тут нужно отрабатывать UTF-8 - символы, дабы не рвать последовательность посередине!
      // В принципе, подход очень простой - если символ имеет вид 10xxxxxxb - то это trail-символ
      // UTF-8 - и рвать перед ним низзя. Мало того, и длину увеличивать не надо. Правда,
      // это правило работает только при корректной UTF8-последовательности, но для
      // ясности мы считаем, что последовательноси у нас только корректные!
      if( (0xC0 & (*s)) != 0x80 ) {
        if( nLen >= 64 ) {
          *(d++) = '"'; pContext->release_buffer();
          d = pContext->get_buffer( ( strEnd - s ) * 4 + 2, nIndent );
          *(d++) = '"';
          nLen = 1;
        } else nLen++;
      }
    }
    switch( *s ) {
      case '\a':	*(d++) = '\\'; *(d++) = 'a'; nLen++; break;
      case '\b':	*(d++) = '\\'; *(d++) = 'b'; nLen++; break;
      case '\f':	*(d++) = '\\'; *(d++) = 'f'; nLen++; break;
      case '\n':	*(d++) = '\\'; *(d++) = 'n'; nLen++; break;
      case '\r':	*(d++) = '\\'; *(d++) = 'r'; nLen++; break;
      case '\t':	*(d++) = '\\'; *(d++) = 't'; nLen++; break;
      case '\v':	*(d++) = '\\'; *(d++) = 'v'; nLen++; break;
      case '"':	*(d++) = '\\'; *(d++) = '"'; nLen++; break;
      case '\\':	*(d++) = '\\'; *(d++) = '\\'; nLen++; break;
      default:
        if( ( ((unsigned char)(*s)) >= ((unsigned char)' ' ) ) && ( ((unsigned char)(*s)) < 127 ) ) {
          // ASCII
          *(d++) = *s;
        } else if( ( 0xC0 == (0xE0 & (*s)) ) && ( ( strEnd - s ) >= 2 ) &&
          ( 0x80 == (0xC0 & s[1]) ) ) {
          // Двойной UTF-8
          *(d++) = *(s++);
          *(d++) = *s;
        } else if( ( 0xE0 == (0xF0 & (*s)) ) && ( ( strEnd - s ) >= 3 ) &&
          ( 0x80 == (0xC0 & s[1]) ) && ( 0x80 == (0xC0 & s[2]) ) ) {
          // Тройной UTF-8 - да здравствуют Грузия, Индия и Китай!
          *(d++) = *(s++);
          *(d++) = *(s++);
          *(d++) = *s;
        } else {
          // Пишем в восьмеричном формате, потому что у него есть строгое ограничение
          // на длину последовательности - 3 символа - и нам не надо заботиться,
          // какой символ следующий. От казусов типа "\x03Back" ==> ";ack" мы избавлены!
          _to_oct( d, (unsigned char)(*s) );
          nLen += 3;
        };
        break;
    }
  }
  *(d++) = '"';
  pContext->release_buffer();
}

QByteArray GJson::toJson( JsonFormat nFormat /*= MaxReadable*/ ) const
{
  GJsonPrivate::ToJsonContext pContext;
  pContext.m_nFlags = nFormat;

  m_d->to_json( 0, &pContext );
  if( 0 != ( nFormat & Indented ) )
    pContext.append( "\r\n", 2 );

  return QByteArray( pContext.m_pBuffer, (int) pContext.m_nCurrent );
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// GJson - разборка строки
struct GJsonPrivate::FromJsonContext
{
  const char		*m_pCurrent,
          *m_pLast;
  int				m_nLine,
          m_nCol,
          m_nTabSize;
  GJsonParseError::ParseError m_nErrorCode;

  // Данные для сохранения позиции
  const char		*m_pSaveCurrent;
  int				m_nSaveLine,
          m_nSaveCol;

  void save()
  {
    m_pSaveCurrent	= m_pCurrent;
    m_nSaveLine		= m_nLine;
    m_nSaveCol		= m_nCol;
  }

  void load()
  {
    m_pCurrent	= m_pSaveCurrent;
    m_nLine		= m_nSaveLine;
    m_nCol		= m_nSaveCol;
  }

  inline bool has_symbols( int nNeeded )
  {
    return ( ( m_pCurrent + nNeeded ) <= m_pLast );
  }

  inline bool is_empty() const
  {
    return ( m_pCurrent >= m_pLast );
  }

  inline char cur() const
  {
    return *m_pCurrent;
  }

  bool next()
  {
    if( is_empty() ) return false;

    // Посмотрим, что за символ, чтобы рассчитать позицию в буфере
    switch( cur() ) {
      case '\n':
      case '\f':
        m_nLine++;
        m_nCol = 1;
        break;

      case '\t':
        m_nCol = ( ( m_nCol + m_nTabSize - 1 ) % m_nTabSize ) + 1;
        break;

      default:
        /// \todo Добавить учёт символов UTF-8 при расчёте позиции.
        m_nCol++;
        break;
    };

    m_pCurrent++;

    return !is_empty();
  }

  bool next( int nSymbols )
  {
    while( nSymbols-- )
      if( !next() ) return false;
    return true;
  }

  bool skip_spaces( bool bEofAsError = true )
  {
    for( ; !is_empty(); next() ) {
      switch( cur() ) {
        case ' ':
        case '\t':
        case '\f':
        case '\r':
        case '\n':
          break;

        case '/':
          if( ( !has_symbols(1) ) || ( '/' != m_pCurrent[1] ) ) return true;
          // Наткнулись на комментарий - надо прочитать всё до конца строки!
          next(2);
          while( true ) {
            if( is_empty() ) {
              if( bEofAsError )
                return error( GJsonParseError::EndOfData );
              return true;
            }
            if( ( '\r' == cur() ) || ( '\n' == cur() ) ) break;
            next();
          }
          break;

        default:
          if( cur() > ' ' ) return true;
          return error( GJsonParseError::InvalidCharacter );
      }
    }
    if( bEofAsError )
      return error( GJsonParseError::EndOfData );

    return true;
  }

  bool error( GJsonParseError::ParseError nCode )
  {
    m_nErrorCode = nCode;
    return false;
  }

  static bool is_ddigit( char c )
  {
    return ( ( c >= '0' ) && ( c <= '9') );
  }

  static bool is_odigit( char c )
  {
    return ( ( c >= '0' ) && ( c <= '7') );
  }

  static bool is_hdigit( char c )
  {
    return (
      ( ( c >= '0' ) && ( c <= '9') ) ||
      ( ( c >= 'a' ) && ( c <= 'f') ) ||
      ( ( c >= 'A' ) && ( c <= 'F') )
    );
  }

  static bool is_alpha( char c, bool bIncludeDigits )
  {
    return (
      ( '_' == c ) ||
      ( ( c >= 'a' ) && ( c <= 'z') ) ||
      ( ( c >= 'A' ) && ( c <= 'Z') ) || (
        bIncludeDigits && (
          ( c >= '0' ) && ( c <= '9')
        )
      )
    );
  }

  bool get_number( unsigned long long& nValue, bool(*pfnTest)(char), unsigned int nRadix, unsigned int nMinDigits = 0, unsigned int nMaxDigits = INT_MAX )
  {
    nValue = 0;
    unsigned int nCount;
    for( nCount = 0; nCount < nMaxDigits; nCount++, next() ) {
      if( is_empty() || !( pfnTest( cur() ) ) ) break;
      nValue = nValue * nRadix +
        ( ( cur() <= '9' ) ? ( cur() - '0' ) : ( toupper( cur() ) - 'A' + 10 ) );
    };
    if( nCount < nMinDigits )
      return error( GJsonParseError::IntExpected );
    return true;
  }

  bool get_decimal( unsigned long long& nValue, unsigned int nMinDigits = 0, unsigned int nMaxDigits = INT_MAX )
  {
    return get_number( nValue, is_ddigit, 10, nMinDigits, nMaxDigits );
  }

  bool get_octal( unsigned long long& nValue, unsigned int nMinDigits = 1, unsigned int nMaxDigits = 3 )
  {
    return get_number( nValue, is_odigit, 8, nMinDigits, nMaxDigits );
  }

  bool get_hex( unsigned long long& nValue, unsigned int nMinDigits = 1, unsigned int nMaxDigits = INT_MAX )
  {
    return get_number( nValue, is_hdigit, 16, nMinDigits, nMaxDigits );
  }

  bool get_name( QByteArray& strResult )
  {
    const char*	strStart	= m_pCurrent;
    int			nCount		= 0;

    // Даём возможность не заключать имена полей в кавычки!
    bool bInQuotes = false;

    if( '"' == cur() ) {
      if( !next() )
        return error( GJsonParseError::EndOfData );
      bInQuotes = true;
      strStart++;
    }

    /// \todo: Добавить возможность имён в UTF-8!

    if( !is_alpha( cur(), false ) )
      return error( GJsonParseError::InvalidName );

    while( true ) {
      nCount++;
      if( !next() )
        return error( GJsonParseError::EndOfData );
      if( !is_alpha( cur(), true ) ) break;
    };

    if( bInQuotes ) {
      if( '"' != cur() )
        return error( GJsonParseError::InvalidName );
      next();
    }

    strResult = QByteArray( strStart, nCount );
    return true;
  }

  GJsonPrivate *parse()
  {
    if( !skip_spaces() ) return 0;

    switch( cur() ) {
      case '{':	// Начало объекта
        return parse_object();
      case '[':	// Начало массива
        return parse_array();
      case '"':	// Значение - строка.
        return parse_string();
    }

    // Это может быть число или значение типа bool
    if( has_symbols( 4 ) && ( 0 == strncmp( "null", m_pCurrent, 4 ) ) ) {
      next( 4 );
      return new GJsonPrivate( GJson::Null );
    }

    if( has_symbols( 4 ) && ( 0 == strncmp( "true", m_pCurrent, 4 ) ) ) {
      next( 4 );
      return new GJsonPrivate( true );
    }

    if( has_symbols( 5 ) && ( 0 == strncmp( "false", m_pCurrent, 5 ) ) ) {
      next( 5 );
      return new GJsonPrivate( false );
    }

    return parse_numeric_value();
  }

  GJsonPrivate *parse_numeric_value()
  {
    // Разбираем форматы вида:
    // [+|-][N...][.][M...][(e|E)[+-]K[K...]]
    // 0[N...], где N - восьмеричная цифра
    // 0x[N...], где N - шестнадцатиричная цифра

    // Сначала отсечём очевидные формы

    // Шестнадцатиричное число
    if( has_symbols( 2 ) && ( 0 == qstrnicmp( "0x", m_pCurrent, 2 ) ) ) {
      next(2);
      unsigned long long nValue;
      if( !get_hex( nValue, 1, INT_MAX ) ) return 0;
      return new GJsonPrivate( nValue );
    };

    // Восьмеричное число.
    // Восьмеричное число должно не только начинаться на '0', но и
    // в конец не иметь чего-либо типа точки или буквы E - иначе это действительное
    // или целое число!
    if( has_symbols(1) && ( '0' == cur() ) ) {
      save();

      unsigned long long nValue;

      if( !get_octal( nValue, 1, INT_MAX ) ) return 0;

      if( is_empty() )
        return new GJsonPrivate( nValue );

      if( is_ddigit( cur() ) ) {
        error( GJsonParseError::InvalidNumber );
        return 0;
      }

      if( ( '.' != cur() ) && ( 'E' != toupper( cur() ) ) )
        return new GJsonPrivate( nValue );

      // Вернём на то, что было до попытки чтения восьмеричного числа.
      load();
    };

    bool				bIsNegative	= false,
              bIsExpNeg	= false,
              bIsIntExist	= false;
    unsigned long long	nValue		= 0,
              nExp		= 0;
    double				dFracPart	= 0.0,
              dDevider	= 10.0;

    // Общий знак числа
    if( ( '+' == cur() ) || ( '-' == cur() ) ) {
      bIsNegative = ( '-' == cur() );
      if( !next() ) {
        error( GJsonParseError::EndOfData );
        return 0;
      }
    }

    // Целая часть
    if( is_ddigit( cur() ) ) {
      if( !get_decimal( nValue ) ) return 0;
      bIsIntExist = true;
    };

    if( is_empty() || ( ( '.' != cur() ) && ( 'E' != toupper( cur() ) ) ) ) {
      // Не точка и не "E"- стало быть, это было просто целое число!
      if( !bIsIntExist ) {
        error( GJsonParseError::InvalidNumber );
        return 0;
      }
      return new GJsonPrivate( bIsNegative ? ( - ( (long long) nValue ) ) : ( (long long) nValue ) );
    }

    // Раз дошли сюда - стало быть, точно получили точку или 'E'!
    if( '.' == cur() ) {
      next();

      // Снимем дробную часть!
      unsigned int nCount;
      for( nCount = 0; nCount < INT_MAX; nCount++, next() ) {
        if( is_empty() || !is_ddigit( cur() ) ) break;
        dFracPart +=
          ( (double) ( cur() - '0' ) ) / dDevider;
        dDevider *= 10.0;
      };

      if( ( 0 == nCount ) && ( !bIsIntExist ) ) {		// Одиночная точка - это не число!
        error( GJsonParseError::InvalidNumber );
        return 0;
      }
    };

    // Цифры кончились - посмотрим, нет ли показателя экспоненты!
    if( ( !is_empty() ) && ( 'E' == toupper( cur() ) ) ) {
      if( !next() ) {
        error( GJsonParseError::EndOfData );
        return 0;
      }

      if( ( '+' == cur() ) || ( '-' == cur() ) ) {
        bIsExpNeg = ( '-' == cur() );
        if( !next() ) {
          error( GJsonParseError::EndOfData );
          return 0;
        }
      }

      if( !get_decimal( nExp, 1 ) ) {
        error( GJsonParseError::InvalidNumber );
        return 0;
      }
    }

    double dExp = (double) nExp;
    double dTemp = ( ( (double) nValue ) + dFracPart ) * pow( 10.0, bIsExpNeg ? (-dExp) : dExp );
    return new GJsonPrivate( (double)( bIsNegative ? (-dTemp) : dTemp ) );
  }


  GJsonPrivate *parse_array()
  {
    if( !next() ) {
      error( GJsonParseError::EndOfData );
      return 0;
    }

    GJsonPrivate *pArray = new GJsonPrivate( GJson::Array );

    while( true ) {
      // Некоторое расширение синтаксиса - мы допускаем существование запятой перед
      // закрывающей скобкой.
      if( !skip_spaces() ) break;

      if( ']' == cur() ) { next(); return pArray; };

      GJsonPrivate *pElement = parse();
      if( 0 == pElement ) break;

      if( !skip_spaces() ) break;

      pArray->m_pArray.push_back( GJson( pElement ) );

      if( ']' == cur() ) { next(); return pArray; };

      if( ',' == cur() ) { next(); continue; };

      error( GJsonParseError::SquareBracketExpected );
      break;
    }

    delete pArray;
    return 0;
  }

  GJsonPrivate *parse_object()
  {
    if( !next() ) {
      error( GJsonParseError::EndOfData );
      return 0;
    }

    GJsonPrivate *pMap = new GJsonPrivate( GJson::Object );

    while( true ) {
      if( !skip_spaces() ) break;

      // Поддержка пустых объектов
      if( '}' == cur() ) { next(); return pMap; }

      // Сформируем ключ
      QByteArray strName;
      if( !get_name( strName ) ) break;
      const char*	strKey = strName.constData();
      const Key&	pKey = *( (Key const*) (&strKey) );

      // Проверим, не дублируется ли имя?
      if( pMap->m_pMap.contains( pKey ) ) {
        error( GJsonParseError::DuplicateName );
        break;
      }

      // Проверим, что дальше идёт двоеточие!
      if( !skip_spaces() ) break;

      if( ':' != cur() ) {
        error( GJsonParseError::ColonExpected );
        break;
      }

      if( !next() ) {
        error( GJsonParseError::EndOfData );
        break;
      }

      // Теперь, собственно, разберём вложенный Json
      GJsonPrivate *pElement = parse();
      if( 0 == pElement ) break;

      if( !skip_spaces() ) break;

      pMap->m_pMap.insert( pKey, GJson( pElement ) );

      if( '}' == cur() ) { next(); return pMap; };

      if( ',' == cur() ) {
        if( next() ) {
          // SYNTEX:
          // Некоторое расширение синтаксиса - мы допускаем существование запятой перед
          // закрывающей скобкой.
          if( !skip_spaces() ) break;
          if( '}' == cur() ) { next(); return pMap; };
          // END SYNTEX
        };
        continue;
      };
      error( GJsonParseError::CurlyBracketExpected );
      break;
    }
    delete pMap;
    return 0;
  }

  GJsonPrivate *parse_string()
  {
    // Строки не расширяются - могут только сокращаться, поэтому вполне можно зааллокировать
    // количество символов, которые остались в разбираемой строке - больше точно не будет!
    QByteArray pResult;
    pResult.reserve( ( m_pLast - m_pCurrent ) + 2 );

    if( !next() ) {
      error( GJsonParseError::EndOfData );
      return 0;
    }

    while( !is_empty() ) {
      bool bStep = true;
      switch( cur() ) {
        case '"':
          if( has_symbols( 2 ) ) {
            next();
            if( '"' == cur() ) {
              // Две кавычки подряд внутри строки
              pResult += '"';
              break;
            };

            // Иначе - пропустим все пробелы и посмотрим, что там!
            if( !skip_spaces( false ) ) return 0;

            if( ( !is_empty() ) &&  ( '"' == cur() ) )	// Снова открывающая кавычка - пошли по-новой!
              break;
          }

          // Если дошли сюда - то значит после закрывающей кавычки нет новой открывающей -
          // и это признак конца строки.
          return new GJsonPrivate( pResult );

        case '\\':
          if( !next() ) {
            error( GJsonParseError::EndOfData );
            return 0;
          }

          switch( cur() ) {
            case 'a':	pResult += '\a'; break;
            case 'b':	pResult += '\b'; break;
            case 'f':	pResult += '\f'; break;
            case 'n':	pResult += '\n'; break;
            case 'r':	pResult += '\r'; break;
            case 't':	pResult += '\t'; break;
            case 'v':	pResult += '\v'; break;
            case '\'':	pResult += '\''; break;
            case '"':	pResult += '"'; break;
            case '\\':	pResult += '\\'; break;
            case '?':	pResult += '?'; break;

            case 'x':	{	// Это шестнадцатиричное предствление строки.
                if( !next() ) {
                  error( GJsonParseError::EndOfData );
                  return 0;
                }
                unsigned long long nValue;
                if( !get_hex( nValue )  ) return 0;
                pResult += (char)( nValue & 0xFF );
                bStep = false;
              };
              break;

            default:	// Все прочие
              if( is_odigit( cur() ) ) {
                // Восьмеричное значение
                unsigned long long nValue;
                if( !get_octal( nValue )  ) return 0;
                pResult += (char)( nValue & 0xFF );
                bStep = false;
                break;
              };

              // Неверная escape-последовательность - мы просто игнорируем!
              pResult += cur();
          };
          break;

        default:		// Просто символ - чисто перебросим!
          pResult += cur();
          break;
      };

      if( bStep )
        next();
    }

    error( GJsonParseError::EndOfData );
    return 0;
  }
};

GJson GJson::fromJson( const char *strData, int cbData, GJsonParseError *pError /*= 0*/ )
{
  GJsonPrivate::FromJsonContext pContext;

  pContext.m_pCurrent		= strData;
  pContext.m_pLast		= strData + cbData;
  pContext.m_nLine		=
  pContext.m_nCol			= 1;
  pContext.m_nErrorCode	= GJsonParseError::NoError;
  pContext.m_nTabSize		= 4;	/// \todo: Как-то нужно этим делом управлять!

  GJsonPrivate *pPriv = pContext.parse();

  if( 0 != pError ) {
    if( 0 != pPriv ) {
      pContext.skip_spaces( false );
      if( !pContext.is_empty() )
        pContext.m_nErrorCode = GJsonParseError::MoreData;
    }

    pError->error	= pContext.m_nErrorCode;
    pError->offset	= ( strData - pContext.m_pCurrent );
    pError->col		= pContext.m_nCol;
    pError->row		= pContext.m_nLine;
  }

  return ( 0 != pPriv ) ? GJson( pPriv ) : GJson( GJson::Undefined );
}

GJson GJson::fromJson( const char *strData, GJsonParseError *pError /*= 0*/ )
{
  return fromJson( strData, qstrlen( strData ), pError  );
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// GJson - поддержка формата сериализации msgpack
#ifndef GQX_JSON_NO_MSGPACK
void GJsonPrivate::msgpack( msgpack_packer *pk ) const
{
  switch( m_nType ) {
    case GJson::Integer:
      msgpack_pack_long_long( pk, m_iValue );
      break;

    case GJson::Double:
      msgpack_pack_double( pk, m_dValue );
      break;

    case GJson::String:
      msgpack_pack_str(pk, m_strValue.size());
      msgpack_pack_str_body(pk, m_strValue.constData(), m_strValue.size());
      break;

    case GJson::Bool:
      if( m_bValue )
        msgpack_pack_true( pk );
      else
        msgpack_pack_false( pk );
      break;

    case GJson::Array: {
        msgpack_pack_array( pk, m_pArray.size() );
        for( int i = 0; i < m_pArray.size(); i++ )
          m_pArray[i].m_d->msgpack( pk );
      };
      break;

    case GJson::Object: {
        msgpack_pack_map( pk, m_pMap.size() );
        for( QMap<GJsonPrivate::Key,GJson>::const_iterator i = m_pMap.constBegin(); i != m_pMap.constEnd(); ++i ) {
          const char*	strKey	= (const char*) i.key().d;
          int			cbKey	= qstrlen( strKey );
          msgpack_pack_str(pk, cbKey);
          msgpack_pack_str_body(pk, strKey, cbKey);
          i.value().m_d->msgpack( pk );
        };
      };
      break;

    default:
      msgpack_pack_nil( pk );
      break;;
  };
}

QByteArray GJson::msgpack() const
{
  msgpack_sbuffer*	pBuffer	= msgpack_sbuffer_new();
  msgpack_packer*		pk		= msgpack_packer_new( pBuffer, msgpack_sbuffer_write );

  m_d->msgpack( pk );

  QByteArray pResult( pBuffer->data, (int) ( pBuffer->size ) );

  msgpack_sbuffer_free( pBuffer );
  msgpack_packer_free( pk );

  return pResult;
}

GJsonPrivate* GJsonPrivate::msgunpack( msgpack_object const& pk )
{
  switch( pk.type ) {
    case MSGPACK_OBJECT_BOOLEAN:
      return new GJsonPrivate( pk.via.boolean );

    case MSGPACK_OBJECT_POSITIVE_INTEGER:
      return new GJsonPrivate( (quint64) pk.via.u64 );

    case MSGPACK_OBJECT_NEGATIVE_INTEGER:
      return new GJsonPrivate( (qint64) pk.via.i64 );

    case MSGPACK_OBJECT_FLOAT:
      return new GJsonPrivate(pk.via.f64);

    case MSGPACK_OBJECT_STR:
      return new GJsonPrivate(QByteArray(pk.via.str.ptr, (int) pk.via.str.size));

    case MSGPACK_OBJECT_ARRAY: {
        GJsonPrivate *pArray = new GJsonPrivate( GJson::Array );
        for( size_t i = 0; i < pk.via.array.size; i++ )
          pArray->m_pArray.push_back( GJson( msgunpack( pk.via.array.ptr[i] ) ) );
        return pArray;
      };
      break;

    case MSGPACK_OBJECT_MAP: {
        GJsonPrivate *pMap = new GJsonPrivate( GJson::Object );
        for( size_t i = 0; i < pk.via.map.size; i++ ) {
          // Тут интересный момент. Теоретически в MessagePack в map ключом может
          // выступать любой тип элемента. Но мы в качестве ключа принимаем только
          // строки - поэтому все остальные типы будем просто игнорировать.
          // По-хорошему, надо бы ещё и проверить на корректность переданное имя.
          if( pk.via.map.ptr[i].key.type != MSGPACK_OBJECT_STR ) continue;

          // msgpack не преобразует строки, а просто расставляет ссылки, поэтому
          // имя у нас не 0-терминировано. Придётся делать копию...
          size_t nLen = pk.via.map.ptr[i].key.via.str.size;
          char *strKey = (char *)alloca( nLen + 1 );
          memcpy(strKey, pk.via.map.ptr[i].key.via.str.ptr, nLen);
          strKey[nLen] = '\0';

          pMap->m_pMap.insert(
            *((Key const*) (&strKey)),
            GJson( msgunpack( pk.via.map.ptr[i].val ) )
          );
        };
        return pMap;
      };
      break;

    default:
      return new GJsonPrivate( GJson::Undefined );
  };
}

GJson GJson::msgunpack( void const *pData, int cbData, int* pOffset /*= 0*/ )
{
  size_t nOffset = 0;

  msgpack_unpacked msg;
  msgpack_unpacked_init(&msg);
  if( !msgpack_unpack_next(&msg, (const char*) pData, (size_t) cbData, &nOffset ) ) {
    msgpack_unpacked_destroy( &msg );
    return GJson( Undefined );
  }

  if( 0 != pOffset )
    *pOffset = (int) nOffset;

  GJsonPrivate *pPriv = GJsonPrivate::msgunpack( msg.data );

  msgpack_unpacked_destroy( &msg );

  return GJson( pPriv );
}

GJson GJson::msgunpack( char const *strData, int* pOffset /*= 0*/ )
{
  return msgunpack( strData, qstrlen( strData ), pOffset );
}

#endif		// #ifndef GQX_JSON_NO_MSGPACK
