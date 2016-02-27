#include <time.h>
#include <stdio.h>

#include <QString>
#include <QtTest>

#include "GJson.h"

#define SIMPLE_TEST_COUNT 10

namespace
{
  quint64 random_quint64();
}

class GJsonTest : public QObject
{
  Q_OBJECT

public:
  GJsonTest();

private:
  template<typename T>
  struct Rnd_Integral
  {
    static void create(T& v)
    {
      v = (T) random_quint64();
    }
  };

  template<typename T, bool set_zero = false>
  struct Rnd_String
  {
    static void create(T& v)
    {
      int count = 10 + (qrand() % 100);
      v.resize(count);
      int zero_index = set_zero ? (qrand() % count) : (-1);
      for (int i = 0; i < count; i++)
        v[i] = typename T::value_type((char)((i == zero_index)
                                            ? 0 : (qrand() & 0xFF)));
    }
  };

  template<typename T, class rnd = Rnd_Integral<T> >
  struct simple_test
  {
    static void init(int count)
    {
      QTest::addColumn<T>("value");
      for (int i = 0; i < count; i++)
        {
          T a;
          rnd::create(a);
          char test_name[32];
          sprintf(test_name, "test#%d", i + 1);
          QTest::newRow(test_name) << a;
        }
    }

    static void run()
    {
      QFETCH(T, value);

      // Check simple assignment
      GJson t1(value);
      QCOMPARE(t1.to<T>(), value);

      // Check conversion from/to JSON representation
      GJson t2 = GJson::fromJson(t1.toJson());
      QCOMPARE(t2.to<T>(), value);
      GJson t3 = GJson::fromJson(t1.toJson(GJson::MinSize));
      QCOMPARE(t3.to<T>(), value);

      // Check conversion from/to msgpack
      GJson t4 = GJson::msgunpack(t1.msgpack());
      QCOMPARE(t4.to<T>(), value);
    }
  };

#define INTEGRAL_TYPE_TEST(type) \
  void test_##type##_data() {simple_test<type>::init(SIMPLE_TEST_COUNT);} \
  void test_##type() {simple_test<type>::run();}

#define STRING_TYPE_TEST(type, use_zeros) \
  void test_##type##_##use_zeros##_data() \
    { \
      simple_test<type, Rnd_String<type,use_zeros> >::init(SIMPLE_TEST_COUNT); \
    } \
  void test_##type##_##use_zeros() \
    { \
      simple_test<type, Rnd_String<type,use_zeros> >::run(); \
    }

private:
  typedef unsigned int uint;
private slots:
  // Bool
  void test_bool_data()
  {
    QTest::addColumn<bool>("value");
    QTest::newRow("true") << true;
    QTest::newRow("false") << false;
  }
  void test_bool() {simple_test<bool>::run();}

  // Integral types
  INTEGRAL_TYPE_TEST(int)
  INTEGRAL_TYPE_TEST(uint)
  INTEGRAL_TYPE_TEST(qint64)
  INTEGRAL_TYPE_TEST(quint64)
  INTEGRAL_TYPE_TEST(float)
  INTEGRAL_TYPE_TEST(double)

  // "String" types
  STRING_TYPE_TEST(QByteArray, false)
  STRING_TYPE_TEST(QByteArray, true)
  STRING_TYPE_TEST(QString, true)
  STRING_TYPE_TEST(QString, false)
};

GJsonTest::GJsonTest()
{
  qsrand(time(0));
}

QTEST_APPLESS_MAIN(GJsonTest)

#include "gjson_test.moc"

namespace
{
  quint64 random_quint64()
  {
    int rnd_max_bits = 0;
    for (quint64 rnd_max = RAND_MAX; rnd_max != 0; rnd_max_bits++)
      rnd_max >>=1;

    quint64 cur_rnd = (quint64) qrand();
    for (int bits = 8 * sizeof(quint64); bits > 0; bits -= rnd_max_bits)
      {
        cur_rnd <<= rnd_max_bits;
        cur_rnd |= (quint64) qrand();
      }

    return cur_rnd;
  }

  template<typename T>
  T random_double()
  {
    T rnd = ((T)random_quint64())/((T)(random_quint64() + 1));

    // Normalizing...
    while (rnd  > 1.0)
      rnd /= 10.0;

    while (rnd  < 0.1)
      rnd *= 10.0;

    if (0 == (qrand() & 0x4))
      {
        int exp = qrand() % std::numeric_limits<T>::max_exponent10;
        while ((--exp) > 0)
          rnd *= 10;
      }
    else
      {
        int exp = qrand() % (-std::numeric_limits<T>::min_exponent10);
        while ((--exp) > 0)
          rnd /= 10;
      }

    return (0 == (qrand() & 0x4)) ? rnd : -rnd;
  }
}

template<>
void GJsonTest::Rnd_Integral<double>::create(double& v)
{
  v = random_double<double>();
}

template<>
void GJsonTest::Rnd_Integral<float>::create(float& v)
{
  v = random_double<float>();
}
