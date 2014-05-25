#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class GJson;

#include <GJson.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();

private:
	Ui::MainWindow *ui;

private slots:
	void testJson( GJson pJson );

};

#endif // MAINWINDOW_H
