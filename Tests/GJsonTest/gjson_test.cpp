#include <time.h>

#include <QString>
#include <QtTest>

#include "GJson.h"

class GJsonTest : public QObject
{
  Q_OBJECT

public:
  GJsonTest();

private:
  template<typename T>
  void create_random(T& v)
  {
    v = (T) qrand();
  }

  template<typename T>
  void do_simple_test()
  {
    T a;
    create_random(a);
    GJson b(a);
    QCOMPARE((T)b, a);
  }

private Q_SLOTS:
  void testInt() {do_simple_test<int>();}
  void testUInt() {do_simple_test<unsigned int>();}
  void testInt64() {do_simple_test<qint64>();}
  void testUInt64() {do_simple_test<quint64>();}
};

GJsonTest::GJsonTest()
{
  qsrand(time(0));
}

QTEST_APPLESS_MAIN(GJsonTest)

#include "gjson_test.moc"
