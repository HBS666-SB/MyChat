#include "loginwidget.h"
#include "ui_loginwidget.h"
#include "mainwindow.h"
#include "netdb/mysocket.h"
#include "comapi/myapp.h"
#include "comapi/unit.h"

LoginWidget::LoginWidget(QWidget *parent) :
    CustomMoveWidget(parent),
    ui(new Ui::LoginWidget)
{
    ui->setupUi(this);
    setQss(QString(":/qss/resource/qss/default.css"));
    setWindowFlags(Qt::FramelessWindowHint);

    m_tcpSocket = new MySocket(this);
    connect(m_tcpSocket,&MySocket::signalStatus,this,&LoginWidget::sltStatus);

    m_tcpSocket->connectToHost(MyApp::m_strHostAddr,MyApp::m_nMsgPort);
}

LoginWidget::~LoginWidget()
{
    delete ui;
}

void LoginWidget::on_btnLogin_clicked()
{
    m_tcpSocket->signalStatus(Login);

}

void LoginWidget::sltStatus(const quint8 &status)
{
    switch (status) {
        case Login:
    {
//        qDebug() << "用户登录！！！";
        sendLogin();
        break;
    }
    case LoginSuccess:
        this->hide();
        MainWindow *mainwdo = new MainWindow;
        mainwdo->show();
    }
}
void LoginWidget::sendLogin()
{
    QJsonObject json;
    json.insert("name",ui->lineEditUser->text());
    json.insert("passwd",ui->lineEditPasswd->text());
    m_tcpSocket->sendMessage(Login, json);
}
