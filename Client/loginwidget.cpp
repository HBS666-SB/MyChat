#include "loginwidget.h"
#include "ui_loginwidget.h"
#include "mainwindow.h"
#include "netdb/mysocket.h"
#include "netdb/databasemsg.h"
#include "comapi/myapp.h"
#include "comapi/unit.h"

#include <QMessageBox>

#include <comapi/myapp.h>

LoginWidget::LoginWidget(QWidget *parent) :
    CustomMoveWidget(parent),
    ui(new Ui::LoginWidget)
{
    ui->setupUi(this);
    setQss(QString(":/qss/resource/qss/default.css"));
    setWindowFlags(Qt::FramelessWindowHint);
    MyApp::InitApp(".");
    m_tcpSocket = new MySocket(this);
    connect(m_tcpSocket,&MySocket::signalStatus,this,&LoginWidget::sltStatus);
    connect(m_tcpSocket,&MySocket::signalConnectSuccess,this,[=]()
    {
        ui->labelWinTitle->setText("已连接服务器");
    });
    m_tcpSocket->connectToHost(MyApp::m_strHostAddr,MyApp::m_nMsgPort);

}

LoginWidget::~LoginWidget()
{
    delete ui;
}

void LoginWidget::loginSuccess(const QJsonValue &dataVal)
{
    if(!dataVal.isObject())
    {
        qDebug() << "登录功能出错";
        return;
    }
    QJsonObject jsonObj = dataVal.toObject();
    MyApp::m_nId = jsonObj.value("id").toInt();
    MyApp::m_strHeadFile = jsonObj.value("head").toString();

    disconnect(m_tcpSocket,&MySocket::signalStatus,this,&LoginWidget::sltStatus);
    MyApp::m_strUserName = ui->lineEditUser->text();
    MyApp::m_strPassword = ui->lineEditPasswd->text();
    //打开该用户的数据库
    qDebug() << "MyApp::m_nId" << MyApp::m_nId;
    DatabaseMsg::getInstance()->OpenUserDatabase(QString("%1user%2.db").arg(MyApp::m_strDatabasePath).arg(MyApp::m_nId));
    DatabaseMsg::getInstance()->OpenMessageDatabase(QString("%1msg%2.db").arg(MyApp::m_strDatabasePath).arg(MyApp::m_nId));

    this->hide();
    MainWindow *mainwdo = new MainWindow(m_tcpSocket);
    mainwdo->show();
}

void LoginWidget::sltStatus(const quint8 &status, const QJsonValue &dataVal)
{
    switch (status) {
    case LoginSuccess:
    {
        loginSuccess(dataVal);
        break;
    }
    case LoginRepeat:
    {
        QMessageBox::warning(this,"登录","该账号已在线");
        break;
    }
    case LoginPasswdError:
    {
        QMessageBox::warning(this,"登录","用户名或密码错误");
        break;
    }
    case RegisterOk:
    {
        QMessageBox::information(this,"注册","新用户注册成功");
        break;
    }
    case RegisterFailed:
    {
        QMessageBox::critical(this,"注册","注册失败");
        break;
    }

    }
}

void LoginWidget::on_btnLogin_clicked()     //登录
{
    QJsonObject json;
    json.insert("name",ui->lineEditUser->text());
    json.insert("passwd",ui->lineEditPasswd->text());
    m_tcpSocket->sendMessage(Login, json);

}

void LoginWidget::on_btnReister_clicked()   //注册
{
    QString name = ui->lineEditUser->text();
    QString passwd = ui->lineEditPasswd->text();
    if(name.isEmpty() || passwd.isEmpty()){
        QMessageBox::warning(this,"注册","用户名或密码不能为空");
        return;
    }
    QJsonObject json;
    json.insert("name",name);
    json.insert("passwd",passwd);
    m_tcpSocket->sendMessage(Register, json);
}

void LoginWidget::on_btnWinMenu_clicked()
{
    ui->stackedWidget->setCurrentIndex(1);
}

void LoginWidget::on_btnCancel_clicked()
{
    ui->stackedWidget->setCurrentIndex(0);
}

