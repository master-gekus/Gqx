#include <QIcon>

#include "GUiHelpers.h"

#include "tcp_connector.h"

#include "app.h"

ClientApp::ClientApp(int &argc, char **argv) :
  QApplication(argc, argv),
  connector_(new TcpConnector(this))
{
  setOrganizationName(QStringLiteral("Master Gekus"));
  setApplicationName(QStringLiteral("Gqx Simple Client"));
  setApplicationVersion("Version " GIT_DESCRIBE " from " __DATE__ " " __TIME__);
}

ClientApp::~ClientApp()
{
}

QIcon ClientApp::iconMain() const
{
  static QIcon icon_;
  GUiHelpers::loadIcon(icon_, QStringLiteral(":/res/main_icon"));
  return icon_;
}
