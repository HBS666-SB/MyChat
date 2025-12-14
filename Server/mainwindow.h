#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "basewidget/customwidget.h"

class TcpMsgServer;
class QButtonGroup;

namespace Ui {
class MainWindow;
}

class MainWindow : public CustomMoveWidget
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:

    void on_btnQuit_clicked();
    void sltButtonClicked(int index);

    void on_btnWinClose_clicked();

    void on_btnLogin_clicked();

    void on_btnUserInsert_clicked();

    void on_btnUserRefresh_clicked();

    void on_btnWinMin_clicked();

private:
    Ui::MainWindow *ui;

    QButtonGroup *m_buttonGroup;
    TcpMsgServer *m_tcpServer;
};

#endif // MAINWINDOW_H
