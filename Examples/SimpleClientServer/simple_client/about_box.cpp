#include "app.h"

#include "about_box.h"
#include "ui_about_box.h"

AboutBox::AboutBox(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::AboutBox)
{
  ui->setupUi(this);

  ui->labelVersion->setText(qApp->applicationVersion());
}

AboutBox::~AboutBox()
{
  delete ui;
}
