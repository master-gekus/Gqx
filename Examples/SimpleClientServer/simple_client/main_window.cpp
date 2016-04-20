#include <QComboBox>
#include <QSettings>
#include <QUrl>
#include <QMessageBox>
#include <QRegExp>

#include "GSocketConnector.h"
#include "GSocketConnectorTcpSocketEngine.h"

#include "app.h"
#include "about_box.h"

#include "main_window.h"
#include "ui_main_window.h"

#define SETTINGS_GROUP QStringLiteral("Main Window")
#define SETTINGS_GEOMETRY QStringLiteral("Geometry")
#define SETTINGS_STATE QStringLiteral("State")

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow),
  combo_host_(new QComboBox()),
  idle_handler_(this)
{
  ui->setupUi(this);
  setWindowIcon(qApp->iconMain());

  { // Setting up Host combo-box
    QSizePolicy sp = combo_host_->sizePolicy();
    sp.setHorizontalStretch(1);
    combo_host_->setSizePolicy(sp);
    combo_host_->setEditable(true);
    combo_host_->setMinimumWidth(150);

    // Inserting right after FileDisconnect action
    QList<QAction*> list = ui->mainToolBar->actions();
    int index = list.indexOf(ui->actionFileDisconnect);
    QAction *action = ui->mainToolBar->insertWidget(list[index+1], combo_host_);
    action->setVisible(true);
    combo_host_->setEditText(QStringLiteral("localhost"));
  }

  QSettings settings;
  settings.beginGroup(SETTINGS_GROUP);
  restoreGeometry(settings.value(SETTINGS_GEOMETRY).toByteArray());
  restoreState(settings.value(SETTINGS_STATE).toByteArray());

  connect(&idle_handler_, SIGNAL(idle()), SLOT(onIdle()));

  GSocketConnector *connector = qApp->connector();
  connect(connector, SIGNAL(connected()), SLOT(onConnectorConnected()));
  connect(connector, SIGNAL(disconnected()), SLOT(onConnectorDisconnected()));
  connect(connector, SIGNAL(error(GSocketConnector::ConnectorError)),
          SLOT(onConnectorError(GSocketConnector::ConnectorError)));
  connect(connector, SIGNAL(stateChanged(GSocketConnector::ConnectorState)),
          SLOT(onConnectorStateChanged(GSocketConnector::ConnectorState)));
}

MainWindow::~MainWindow()
{
  delete ui;
}

void
MainWindow::closeEvent(QCloseEvent* event)
{
  QSettings settings;
  settings.beginGroup(SETTINGS_GROUP);
  settings.setValue(SETTINGS_GEOMETRY, saveGeometry());
  settings.setValue(SETTINGS_STATE, saveState());

  QMainWindow::closeEvent(event);
}

void
MainWindow::onIdle()
{
  bool is_connecting = (GSocketConnector::UnconnectedState
                        != qApp->connector()->state());
  combo_host_->setEnabled(!is_connecting);
  ui->actionFileConnect->setEnabled(!is_connecting);
  ui->actionFileDisconnect->setEnabled(is_connecting);
}

void
MainWindow::onConnectorConnected()
{
  qDebug("MainWindow::onConnectorConnected()");
}

void
MainWindow::onConnectorDisconnected()
{
  qDebug("MainWindow::onConnectorDisconnected()");
}

void
MainWindow::onConnectorError(GSocketConnector::ConnectorError)
{
  qDebug("MainWindow::onConnectorError()");
}

void
MainWindow::onConnectorStateChanged(GSocketConnector::ConnectorState)
{
  qDebug("MainWindow::onConnectorStateChanged()");
}

void
MainWindow::on_actionHelpAbout_triggered()
{
  AboutBox(this).exec();
}

void
MainWindow::on_actionFileConnect_triggered()
{
  if (GSocketConnector::UnconnectedState != qApp->connector()->state())
    return;

  static QRegExp url_re(QStringLiteral(
    "((tcp|ssl|local)\\:(//)?)?(\\w+(\\.(\\w+))*)(\\:(\\d{1,5}))?"));

//  qDebug("reg_exp is %s", url_re.isValid() ? "valid" : "INVALID");

  if (!url_re.exactMatch(combo_host_->currentText().trimmed()))
    {
      QMessageBox::critical(this, this->windowTitle(),
                            QStringLiteral("Error parsing URL."));
      return;
    }

  QString scheme = url_re.cap(2);
  if (scheme.isEmpty())
    scheme = QStringLiteral("tcp");

  QString host = url_re.cap(4);

  QString port_str = url_re.cap(8);
  quint16 port = DEFAULT_PORT;
  if (!port_str.isEmpty())
    {
      bool ok = false;
      port = port_str.toUShort(&ok);
      if (!ok)
        {
          QMessageBox::critical(this, this->windowTitle(),
                                QStringLiteral("Invalid port."));
          return;
        }
    }

  QString resul_url = QStringLiteral("%1://%2:%3").arg(scheme, host).arg(port);
  combo_host_->setCurrentText(resul_url);

  GSocketConnectorAbstractEngine *engine = 0;

  if (scheme == QStringLiteral("tcp"))
    {
      engine = new GSocketConnectorTcpSocketEngine(host, port);
    }
//  else if (scheme == QStringLiteral("ssl"))
//    {
//      engine = new GSocketConnectorSslSocketEngine(host, port);
//    }
//  else if (scheme == QStringLiteral("local"))
//    {
//      engine = new GSocketConnectorLocalSocketEngine(host);
//    }
  else
    {
      QMessageBox::critical(this, this->windowTitle(),
                            QStringLiteral("Invalid port."));
      return;
      Q_ASSERT(false);
    }

  if (!engine)
    return;

  qApp->connector()->connectToServer(engine);
}

void
MainWindow::on_actionFileDisconnect_triggered()
{
  if (GSocketConnector::UnconnectedState == qApp->connector()->state())
    return;

  qApp->connector()->disconnectFromServer();
}
