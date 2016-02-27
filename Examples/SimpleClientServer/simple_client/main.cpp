#include "app.h"
#include "main_window.h"

int main(int argc, char *argv[])
{
  ClientApp a(argc, argv);
  MainWindow w;
  w.show();

  return a.exec();
}
