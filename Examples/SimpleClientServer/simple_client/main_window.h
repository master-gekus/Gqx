#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>

#include "GIdleHandler.h"
#include "GSocketConnector.h"

class QComboBox;

namespace Ui {
  class MainWindow;
}

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = 0);
  ~MainWindow();

protected:
  void closeEvent(QCloseEvent* event) Q_DECL_OVERRIDE Q_DECL_FINAL;

private:
  Ui::MainWindow *ui;
  QComboBox *combo_host_;
  GIdleHandler idle_handler_;

private slots:
  void on_actionHelpAbout_triggered();
  void on_actionFileConnect_triggered();
  void on_actionFileDisconnect_triggered();

private slots:
  void onIdle();

  void onConnectorConnected();
  void onConnectorDisconnected();
  void onConnectorError(GSocketConnector::ConnectorError);
  void onConnectorStateChanged(GSocketConnector::ConnectorState);

};

#endif // MAIN_WINDOW_H
