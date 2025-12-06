#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QButtonGroup>
#include <QFileInfo>
#include <QMenu>
#include <qqcell.h>
#include "comapi/myapp.h"
#include <QInputDialog>
#include <QJsonObject>
#include <QMessageBox>
#include "netdb/databasemsg.h"
#include <netdb/databasemsg.h>
#include "comapi/unit.h"

MainWindow::MainWindow(MySocket *socket, QWidget *parent) :
    CustomMoveWidget(parent),
    ui(new Ui::MainWindow)
{
    m_tcpSocket = socket;
    m_tcpSocket->setParent(this);
    ui->setupUi(this);
    setQss(QString(":/qss/resource/qss/default.css"));
    setWindowFlags(Qt::FramelessWindowHint);
    ui->GCStackedWidget->setCurrentIndex(0);

    m_buttonGroup = new QButtonGroup(this);
    m_buttonGroup->addButton(ui->btnFrind,0);
    m_buttonGroup->addButton(ui->btnGroup,1);
    m_buttonGroup->addButton(ui->btnConversation,2);
    m_buttonGroup->addButton(ui->btnApplay,3);

    connect(m_buttonGroup,SIGNAL(buttonClicked(int)),this,SLOT(sltButtonClicked(int)));
    connect(m_tcpSocket,&MySocket::signalStatus,this,&MainWindow::sltStatus);
    connect(m_tcpSocket,&MySocket::signalConnectSuccess,this,[=](){
        qDebug() << "MainWindow连接服务器成功";
    });

    m_bQuit = false;

    InitSysMenu();
    InitSysTrayIcon();
    InitQQListMenu();

    setHead(MyApp::m_strHeadPath + MyApp::m_strHeadFile);
    ui->labelUser->setText(MyApp::m_strUserName);
}

MainWindow::~MainWindow()
{

    delete ui;
}

void MainWindow::sltButtonClicked(int index)
{
    ui->GCStackedWidget->setCurrentIndex(index);
}

void MainWindow::InitSysMenu()
{
    // 设置子菜单
    QMenu *sysmenu = new QMenu(this);
    sysmenu->addAction(tr("系统设置"));
    sysmenu->addAction("消息管理器");
    sysmenu->addAction("文件助手");
    sysmenu->addSeparator();
    sysmenu->addAction("修改密码");
    sysmenu->addAction("帮助");
    sysmenu->addAction("连接服务器");
    sysmenu->addSeparator();
    sysmenu->addAction("升级");

    // 添加菜单
    ui->btnSysMenu->setMenu(sysmenu);
    connect(sysmenu, SIGNAL(triggered(QAction*)), this, SLOT(SltSysmenuCliecked(QAction*)));
}


void MainWindow::InitSysTrayIcon()
{
    systemTrayIcon = new QSystemTrayIcon(this);
    systemTrayIcon->setIcon(QIcon(":/resource/background/app.png"));

    QMenu *m_trayMenu = new QMenu(this);
    m_trayMenu->addAction("我在线上");
    m_trayMenu->addAction("离线");
    m_trayMenu->addSeparator();
    m_trayMenu->addAction("显示主面板");
    m_trayMenu->addSeparator();
    m_trayMenu->addAction("退出");

    systemTrayIcon->setContextMenu(m_trayMenu);
    systemTrayIcon->show();

    connect(systemTrayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),this, SLOT(SltTrayIcoClicked(QSystemTrayIcon::ActivationReason)));
    connect(m_trayMenu, SIGNAL(triggered(QAction*)), this, SLOT(SltTrayIconMenuClicked(QAction*)));
}
void MainWindow::SltTrayIcoClicked(QSystemTrayIcon::ActivationReason reason)
{
    if(reason == QSystemTrayIcon::DoubleClick){
        if(this->isHidden()){
            this->show();
        }
    }
}
/**
 * @brief MainWindow::InitQQListMenu
 * 添加好友列表菜单
 */
void MainWindow::InitQQListMenu()
{
    //默认项
    QQCell *myFriend = new QQCell;
    myFriend->groupName = QString(tr("我的好友"));
    myFriend->isOpen = false;
    myFriend->type = QQCellType_Group;
    myFriend->name = QString(tr("我的好友"));
    myFriend->subTitle = QString("(0/0)");
    ui->frindListWidget->insertQQCell(myFriend);

    QQCell *blacklist = new QQCell;
    blacklist->groupName = QString(tr("黑名单"));
    blacklist->isOpen = false;
    blacklist->type = QQCellType_Group;
    blacklist->name = QString(tr("黑名单"));
    blacklist->subTitle = QString("(0/0)");
    ui->frindListWidget->insertQQCell(blacklist);

    connect(ui->frindListWidget, SIGNAL(onChildDidDoubleClicked(QQCell*)), this, SLOT(SltFriendsClicked(QQCell*)));

    //菜单
    QMenu *addFriend = new QMenu(this);
    addFriend->addAction(tr("添加好友"));
    addFriend->addAction(tr("刷新"));
    addFriend->addSeparator();
    addFriend->addAction(tr("删除该组"));
    ui->frindListWidget->setGroupPopMenu(addFriend);
    connect(addFriend,SIGNAL(triggered(QAction*)),this,SLOT(onAddFriendMenuDidSelected(QAction*)));

    //子菜单
    QMenu *childMenu = new QMenu(this);
    childMenu->addAction(tr("发送即时消息"));
    childMenu->addSeparator();
    childMenu->addAction("移动到黑名单");
    childMenu->addAction("删除联系人");
    //***********疑惑************
    QMenu *groupListMenu = new QMenu(tr("移动联系人至"));
    childMenu->addMenu(groupListMenu);

    // childMenu->addAction(tr("创建讨论组"));
    connect(childMenu, SIGNAL(triggered(QAction*)), this, SLOT(onChildPopMenuDidSelected(QAction*)));
    ui->frindListWidget->setChildPopMenu(childMenu);


}
// 托盘菜单
void MainWindow::SltTrayIconMenuClicked(QAction *action)
{
    if ("退出" == action->text()) {
        m_bQuit = true;
        //        m_tcpSocket->SltSendOffline();
        this->hide();
        QTimer::singleShot(500, this, SLOT(SltQuitApp()));
    }
    else if ("显示主面板" == action->text()) {
        this->show();
    }
    else if (!QString::compare("我在线上", action->text()))
    {
        m_tcpSocket->CheckConnected();
    }
    else if (!QString::compare("离线", action->text()))
    {
        m_tcpSocket->ColseConnected();
    }
}
void MainWindow::SltQuitApp()
{
    //    // 关闭所有的聊天窗口
    //    foreach (ChatWindow *window, m_chatFriendWindows) {
    //        window->close();
    //    }

    //    foreach (ChatWindow *window, m_chatGroupWindows) {
    //        window->close();
    //    }

    // 关闭socket
    m_tcpSocket->deleteLater();

    delete ui;
    qApp->quit();
}

void MainWindow::SltSysmenuCliecked(QAction *action)
{

}

void MainWindow::setHead(const QString &headFile)
{
    QString absPath = QFileInfo(headFile).absoluteFilePath();
    QPixmap pix(headFile);
    if (pix.isNull()) {
        qWarning() << "头像图片加载失败：" << headFile;
        return;
    }
    QString qss = QString("QWidget#widgetHead { "
                          "background-image: url(%1); "
                          "background-repeat: no-repeat; "
                          "background-position: center; "
                          "background-size: cover; "
                          "border-radius: 5px}")
            .arg(absPath);
    ui->widgetHead->setStyleSheet(qss);
    ui->widgetHead->setAutoFillBackground(true);
    ui->widgetHead->update();

}

void MainWindow::onAddFriendMenuDidSelected(QAction *action)
{
    if (!action->text().compare(tr("添加好友")))
    {
        bool isOk;
        QString name = QInputDialog::getText(this, "加好友", "请输入要添加的好友名称",QLineEdit::Normal,QString(),&isOk);

        if (!isOk) {
            return;
        }

        if(name.isEmpty())
        {
            QMessageBox::warning(this,"添加好友", "请输入用户名");
            return;
        }
        // 首先判断该用户是否已经是我的好友了
        if (DatabaseMsg::getInstance()->isMyFriend(MyApp::m_nId, name) == AddFriendFailed_IsHad) {
            QMessageBox::information(this,"添加好友", "该用户已经是你的好友了！");
            return;
        }else if(DatabaseMsg::getInstance()->isMyFriend(MyApp::m_nId, name) == AddFriendFailed_Readd){
            QMessageBox::warning(this,"添加好友", "已经提交过申请，请不要重复发送验证信息");
            return;
        }
        // 构建 Json 对象
        QJsonObject json;
        json.insert("id", m_tcpSocket->GetUserId());
        json.insert("name", name);

        m_tcpSocket->sendMessage(AddFriend, json);
    }
    else if (!action->text().compare(tr("刷新")))
    {
        // 上线的时候获取当前好友的状态
        //        QJsonArray friendArr = DataBaseMagr::Instance()->GetMyFriend(MyApp::m_nId);

        // 组织Jsonarror
        //        m_tcpSocket->SltSendMessage(RefreshFriends, friendArr);
    }
    else if (!action->text().compare(tr("删除该组")))
    {
        qDebug() << "delete group";
    }
}

void MainWindow::SltFriendsClicked(QQCell *action)
{

}

void MainWindow::onChildPopMenuDidSelected(QAction *action)
{

}

void MainWindow::sltStatus(const quint8 &status, const QJsonValue &dataVal)
{
    qDebug() << "mainwindow sltstatus";
    switch (status) {
    case AddFriendOk:
    {
        addFriend(dataVal);
        QMessageBox::information(this,"添加好友","好友申请发送成功！");
        break;
    }
    case AddFriendFailed:
    {
        QMessageBox::information(this,"添加好友","系统故障，添加好友失败");
        break;
    }
    case AddFriendFailed_NoneUser:
    {
        QMessageBox::warning(this,"添加好友","用户名不存在");
        break;
    }
    }
}
void MainWindow::addFriend(const QJsonValue &dataVal)
{
    if(!dataVal.isObject()){
        qDebug() << "添加好友逻辑有误";
        return;
    }
    QJsonObject jsonObj = dataVal.toObject();
    int userId = jsonObj.value("id").toInt();
    QString friendName = jsonObj.value("name").toString();
    DatabaseMsg::getInstance()->AddFriend(userId,friendName);

}
