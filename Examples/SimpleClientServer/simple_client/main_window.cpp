#include <QComboBox>
#include <QSettings>

#include "GSocketConnector.h"

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
  ui->actionFileConnect->setEnabled(!is_connecting);
  ui->actionFileDisconnect->setEnabled(is_connecting);
}

void
MainWindow::on_actionHelpAbout_triggered()
{
  AboutBox(this).exec();
}
