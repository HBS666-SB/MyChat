#ifndef LOGINWIDGET_H
#define LOGINWIDGET_H

#include <QWidget>
#include "basewidget/customwidget.h"

class MySocket;
namespace Ui {
class LoginWidget;
}

class LoginWidget : public CustomMoveWidget
{
    Q_OBJECT

public:
    explicit LoginWidget(QWidget *parent = nullptr);
    ~LoginWidget();

    void sendLogin();
    void loginSuccess();

private slots:
    void on_btnLogin_clicked();
    void sltStatus(const quint8 &status);

    void on_btnWinMenu_clicked();

    void on_btnCancel_clicked();

    void on_btnReister_clicked();

private:
    Ui::LoginWidget *ui;
    MySocket *m_tcpSocket;


};

#endif // LOGINWIDGET_H
