#include <time.h>
#include <stdio.h>

#include <QString>
#include <QtTest>

#include "GJson.h"

#define SIMPLE_TEST_COUNT 10

class GJsonTest : public QObject
{
  Q_OBJECT

public:
  GJsonTest();

private:
  template<typename T>
  struct Rnd_Int
  {
    static void create(T& v)
    {
      v = (T) qrand();
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

  template<typename T, class rnd = Rnd_Int<T> >
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
      GJson b(value);
      QCOMPARE(b.to<T>(), value);
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
  // Integral types
  INTEGRAL_TYPE_TEST(int)
  INTEGRAL_TYPE_TEST(uint)
  INTEGRAL_TYPE_TEST(qint64)
  INTEGRAL_TYPE_TEST(quint64)

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
