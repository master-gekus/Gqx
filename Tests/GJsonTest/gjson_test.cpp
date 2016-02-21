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
    QCOMPARE(b.to<T>(), a);
  }

private Q_SLOTS:
  void testInt() {do_simple_test<int>();}
  void testUInt() {do_simple_test<unsigned int>();}
  void testInt64() {do_simple_test<qint64>();}
  void testUInt64() {do_simple_test<quint64>();}
  void testQString() {do_simple_test<QString>();}
  void testQByteArray() {do_simple_test<QByteArray>();}
};

GJsonTest::GJsonTest()
{
  qsrand(time(0));
}

QTEST_APPLESS_MAIN(GJsonTest)

#include "gjson_test.moc"

template<>
void GJsonTest::create_random(QByteArray& v)
{
  int count = 10 + (qrand() % 100);
  v.resize(count);
  for (int i = 0; i < count; i++)
    v[i] = (char)(qrand() & 0xFF);
}

template<>
void GJsonTest::create_random(QString& v)
{
  int count = 10 + (qrand() % 100);
  v.resize(count);
  for (int i = 0; i < count; i++)
    v[i] = QChar((char)(qrand() & 0xFF));
}
