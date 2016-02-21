#ifndef GJSON_H
#define GJSON_H

#include <QSharedDataPointer>
#include <QObject>
#include <QString>
#include <QVariant>

#include <QIODevice>

class GJsonPrivate;

struct GJsonParseError
{
  enum ParseError {
    NoError					= 0,
    MoreData				= 1,
    InvalidCharacter		= (-1),
    EndOfData				= (-2),
    IntExpected				= (-3),
    InvalidNumber			= (-4),
    SquareBracketExpected	= (-5),
    CurlyBracketExpected	= (-6),
    ColonExpected			= (-7),
    InvalidName				= (-8),
    DuplicateName			= (-9),
  };

  ParseError	error;
  int			offset,
        col,row;

  inline bool isOk() { return ( error >= 0 ); }
  QString errorString();
};

class GJson
{
public:
  enum Type {
    Null		= 0x0,
    Bool		= 0x1,
    Integer		= 0x2,
    Double		= 0x3,
    String		= 0x4,
    Array		= 0x5,
    Object		= 0x6,
    Undefined	= (-1)
  };

public:
  enum JsonFormats {
    Compact				= 0x0000,
    Indented			= 0x0001,
    UseSpaces			= 0x0002,
    AlignObjects		= 0x0004,
    SplitStrings		= 0x0008,
    InlineEmptyArrays	= 0x0010,
    InlineEmptyObjects	= 0x0020,
    InlineSimpleArrays	= 0x0040,
    SplitSimpleArrays	= 0x0080,
    SimpleFieldsFirst	= 0x0100,

    MaxReadable			= (
                Indented			|
                AlignObjects		|
                SplitStrings		|
                InlineEmptyArrays	|
                InlineEmptyObjects	|
                InlineSimpleArrays	|
                SplitSimpleArrays	|
                SimpleFieldsFirst
              ),
    MinSize				= Compact
  };
  Q_DECLARE_FLAGS(JsonFormat, JsonFormats)

public:
  GJson( Type nType = Null );

public:
  ~GJson();

private:
  GJson( GJsonPrivate *pSource );

public:
  GJson( const GJson &pSource );
  GJson( bool bValue );
  GJson( double dValue );
  GJson( int nValue );
  GJson( unsigned int nValue );
  GJson( qint64 nValue );
  GJson( quint64 nValue );
  GJson( const QString &strValue );
  GJson( const char* strValue );
  GJson( const QByteArray &pValue );
  GJson( const QVariant &vValue );

  GJson &operator=( const GJson &pSource );

public:
  Type type() const;
  inline bool isNull() const { return type() == Null; }
  inline bool isBool() const { return type() == Bool; }
  inline bool isInteger() const { return type() == Integer; }
  inline bool isDouble() const { return type() == Double; }
  inline bool isNumeric() const { return ( type() == Double ) || ( type() == Integer ); }
  inline bool isString() const { return type() == String; }
  inline bool isArray() const { return type() == Array; }
  inline bool isObject() const { return type() == Object; }
  inline bool isCompound() const { return ( type() == Object ) || ( type() == Array ); }
  inline bool isUndefined() const { return type() == Undefined; }
  inline bool isSet() const { return ( type() != Null ) && ( type() != Undefined ); }

public:
  bool toBool() const;
  int toInt() const;
  unsigned int toUInt() const;
  long long toLongLong() const;
  unsigned long long toULongLong() const;
  double toDouble() const;
  QString toString() const;
  QByteArray toByteArray() const;
  const char* toStr() const;
  QVariant toVariant() const;

  inline operator bool () const { return toBool(); }
  inline operator int () const { return toInt(); }
  inline operator unsigned int () const { return toUInt(); }
  inline operator long long () const { return toLongLong(); }
  inline operator unsigned long long () const { return toULongLong(); }
  inline operator double () const { return toDouble(); }
  inline operator QString () const { return toString(); }
  inline operator QByteArray () const { return toByteArray(); }
  inline operator const char* () const { return toStr(); }
  inline operator QVariant () const { return toVariant(); }

  template<typename T> inline T to() const { return (T)(*this);}

public:
  int count() const;
  void resize( int nNewSize );
  GJson const at( int nIndex ) const;
  GJson& at( int nIndex );
  GJson const at( const char* strKey ) const;
  GJson& at( const char* strKey );

  inline GJson const operator []( int nIndex ) const { return at(nIndex); }
  inline GJson& operator []( int nIndex ) { return at(nIndex); }

  inline GJson const at( QByteArray const& strKey ) const { return at( strKey.constData() ); }
  inline GJson const at( QString const& strKey ) const { return at( strKey.toUtf8().constData() ); }
  inline GJson const operator []( const char* strKey ) const { return at(strKey); }
  inline GJson const operator []( QString const& strKey ) const { return at(strKey); }
  inline GJson const operator []( QByteArray const& strKey ) const { return at(strKey); }

  inline GJson& at( QByteArray const& strKey ) { return at( strKey.constData() ); }
  inline GJson& at( QString const& strKey ) { return at( strKey.toUtf8().constData() ); }
  inline GJson& operator []( const char* strKey ) { return at(strKey); }
  inline GJson& operator []( QString const& strKey ) { return at(strKey); }
  inline GJson& operator []( QByteArray const& strKey ) { return at(strKey); }

public:
  QList<QByteArray> keys() const;

public:
  void append( GJson const& pValue );
  void prepend( GJson const& pValue );

public:
  static GJson fromJson( const char *strData, int cbData, GJsonParseError *pError = 0 );
  static GJson fromJson( const char *strData, GJsonParseError *pError = 0 );
  static inline GJson fromJson( QByteArray const& pData, GJsonParseError *pError = 0 )
    { return fromJson( pData.constData(), pData.size(), pError ); }
  QByteArray toJson( JsonFormat nFormat = MaxReadable ) const;

#ifndef GQX_JSON_NO_MSGPACK
public:
  QByteArray msgpack() const;
  static GJson msgunpack( void const *pData, int cbData, int* pOffset = 0 );
  static GJson msgunpack(char const *strData, int* pOffset = 0 );
  static inline GJson msgunpack( QByteArray const& pData, int* pOffset = 0 )
    { return msgunpack( pData.constData(), pData.size(), pOffset ); }
#endif		// #ifndef GQX_JSON_NO_MSGPACK

private:
  QSharedDataPointer<GJsonPrivate> m_d;

  friend class GJsonPrivate;
};

Q_DECLARE_METATYPE( GJson )

template<>
inline QString GJson::to() const
{
  return this->toString();
}

template<>
inline QByteArray GJson::to() const
{
  return this->toByteArray();
}

#endif // GJSON_H
