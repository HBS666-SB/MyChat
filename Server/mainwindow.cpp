#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "netdb/myserver.h"
#include <QDebug>
#include <QButtonGroup>
#include <QJsonObject>
#include <QStandardItemModel>
#include "netdb/databasemag.h"
#include "comapi/myapp.h"
#include "comapi/global.h"

MainWindow::MainWindow(QWidget *parent) :
    CustomMoveWidget(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint);
    setQss(QString(":/qss/resource/qss/default.css"));
    ui->stackedWidgetFunc->setCurrentIndex(0);
    ui->stackedWidgetMain->setCurrentIndex(0);
    MyApp::InitApp(".");

    m_buttonGroup = new QButtonGroup(this);
    m_buttonGroup->addButton(ui->btnUserInfoPage,1);
    m_buttonGroup->addButton(ui->btnScorePage,0);
    m_buttonGroup->addButton(ui->btnScoreInsertPage,2);
    m_buttonGroup->addButton(ui->btnUsersPage,3);
    m_buttonGroup->addButton(ui->btnPasswordPage,4);
    m_buttonGroup->addButton(ui->btnDataBackup,5);

    connect(m_buttonGroup,SIGNAL(buttonClicked(int)),this,SLOT(sltButtonClicked(int)));

    m_tcpServer = new TcpMsgServer(this);
    m_tcpServer->StartListen(60101);
    m_tcpFileServer = new TcpFileServer(this);
    m_tcpFileServer->StartListen(60102);
    ui->labelHostAddr->setText("本机IP：" + myHelper::GetIP());
    DataBaseMag::getInstance()->openDatabase(QString("%1server.db").arg(MyApp::m_strDatabasePath)); //打开数据库

}

MainWindow::~MainWindow()
{
    DataBaseMag::getInstance()->changeAllUserStatus();
    DataBaseMag::getInstance()->closeDatabase();
    delete ui;
}

void MainWindow::on_btnQuit_clicked()
{
    close();
}

void MainWindow::sltButtonClicked(int index)
{
    ui->stackedWidgetFunc->setCurrentIndex(index);
}

void MainWindow::on_btnWinClose_clicked()
{
    close();
}

void MainWindow::on_btnLogin_clicked()
{
    QString userName = ui->lineEditUser->text();
    QString passwd = ui->lineEditPasswd->text();
    QJsonObject jsonObj = DataBaseMag::getInstance()->userLogin(userName,passwd);
    qDebug() << jsonObj;
    if(jsonObj.value("msg") == "ok"){
        ui->stackedWidgetMain->setCurrentIndex(1);
    }
}

void MainWindow::on_btnUserInsert_clicked() //用户注册
{
    QString name = ui->lineEditAddUser->text();
    QString passwd = ui->lineEditAddPasswd->text();
    if(DataBaseMag::getInstance()->userRegister(name,passwd) == RegisterOk){
        qDebug() << "注册成功";
    }else {
        qDebug() << "注册失败";
    }
}

void MainWindow::on_btnUserRefresh_clicked()    //显示所有用户
{
    QList<QVariantMap> users = DataBaseMag::getInstance()->getAllUser();
    QStandardItemModel *userModel = new QStandardItemModel(this);

    QStringList headers = {"用户ID", "用户名", "密码", "头像路径", "在线状态", "最后登录时间"};
    userModel->setHorizontalHeaderLabels(headers);
    userModel->removeRows(0, userModel->rowCount());    //清空

    if (!users.isEmpty()) {
        for (int i = 0; i < users.size(); ++i) {
            QVariantMap user = users[i];

            QStandardItem *idItem = new QStandardItem(QString::number(user["id"].toInt()));
            QStandardItem *nameItem = new QStandardItem(user["name"].toString());
            QStandardItem *passwdItem = new QStandardItem(user["passwd"].toString());
            QStandardItem *headItem = new QStandardItem(user["head"].toString());
            QString statusText = (user["status"].toInt() == OnLine) ? "在线" : "离线";
            QStandardItem *statusItem = new QStandardItem(statusText);
            QString lastTime = user["lasttime"].toString().isEmpty() ? "-" : user["lasttime"].toString();
            QStandardItem *lastTimeItem = new QStandardItem(lastTime);

            userModel->setItem(i, 0, idItem);
            userModel->setItem(i, 1, nameItem);
            userModel->setItem(i, 2, passwdItem);
            userModel->setItem(i, 3, headItem);
            userModel->setItem(i, 4, statusItem);
            userModel->setItem(i, 5, lastTimeItem);
        }
//        qDebug() << "成功加载" << users.size() << "条用户数据";
    } else {
        QStandardItem *emptyItem = new QStandardItem("暂无用户数据");
        userModel->setItem(0, 0, emptyItem);
        qDebug() << "无用户数据";
    }

    // 绑定模型到TableView
    ui->tableViewUsers->setModel(userModel);

    // 设置TableView样式
    ui->tableViewUsers->setEditTriggers(QAbstractItemView::NoEditTriggers); // 禁止编辑
    ui->tableViewUsers->setSelectionBehavior(QAbstractItemView::SelectRows); // 整行选中
    ui->tableViewUsers->setSelectionMode(QAbstractItemView::SingleSelection); // 单选
    ui->tableViewUsers->horizontalHeader()->setStretchLastSection(true); // 最后一列拉伸
    ui->tableViewUsers->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed); // 固定列宽模式
    // 逐个设置列宽
    ui->tableViewUsers->setColumnWidth(0, 40);   // 用户ID列宽80px
    ui->tableViewUsers->setColumnWidth(1, 80);  // 用户名列宽120px
    ui->tableViewUsers->setColumnWidth(2, 80);   // 密码列宽80px
    ui->tableViewUsers->setColumnWidth(3, 100);  // 头像路径列宽200px
    ui->tableViewUsers->setColumnWidth(4, 40);   // 在线状态列宽80px
    ui->tableViewUsers->setColumnWidth(5, 100);

    ui->tableViewUsers->verticalHeader()->setVisible(false); // 隐藏行号
}

void MainWindow::on_btnWinMin_clicked()
{
//    this->hide();
}
