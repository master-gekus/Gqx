#include <QComboBox>

#include "app.h"
#include "about_box.h"

#include "main_window.h"
#include "ui_main_window.h"

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow),
  combo_host_(new QComboBox())
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
  }

}

MainWindow::~MainWindow()
{
  delete ui;
}

void
MainWindow::on_actionHelpAbout_triggered()
{
  AboutBox(this).exec();
}
