#ifndef ABOUT_BOX_H
#define ABOUT_BOX_H

#include <QDialog>

namespace Ui {
  class AboutBox;
}

class AboutBox : public QDialog
{
  Q_OBJECT

public:
  explicit AboutBox(QWidget *parent = 0);
  ~AboutBox();

private:
  Ui::AboutBox *ui;
};

#endif // ABOUT_BOX_H
