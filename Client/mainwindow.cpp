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
#include <basewidget/iteminfo.h>
#include <comapi/global.h>
#include <uipage/chatwindow.h>

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

void MainWindow::onChildPopMenuDidSelected(QAction *action)
{
    QQCell *cell = ui->frindListWidget->GetRightClickedCell();
    if (nullptr == cell) return;
    if(!action->text().compare(tr("发送即时消息")))
    {
        qDebug() << "send message" << cell->id;
        SltFriendsClicked(cell);

    }
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
        if(name == MyApp::m_strUserName){
            QMessageBox::warning(this,"添加好友", "不能添加自己为好友");
            return;
        }
        // 首先判断该用户是否已经是我的好友了
        if (DatabaseMsg::getInstance()->isMyFriend(name)) {
            QMessageBox::information(this,"添加好友", "该用户已经是你的好友了！");
            return;
        }
        // 构建 Json 对象
        QJsonObject json;
        json.insert("id", m_tcpSocket->GetUserId());
        json.insert("name", name);

        m_tcpSocket->sendMessage(AddFriend, json);
        QMessageBox::information(this,"添加好友","好友申请发送成功");
    }
    else if (!action->text().compare(tr("刷新")))
    {
        QJsonObject jsonObj;
        jsonObj.insert("id",MyApp::m_nId);

        // 组织Jsonarror
        m_tcpSocket->sendMessage(RefreshFriends, QJsonValue(jsonObj));
    }
    else if (!action->text().compare(tr("删除该组")))
    {
        qDebug() << "delete group";
    }
}

void MainWindow::SltFriendsClicked(QQCell *cell)
{
    for(ChatWindow *window : m_chatFriendWindows){
        if(window->getUserId() == cell->id){
            qDebug() << cell->id;
            window->show();
            return;
        }
    }
    ChatWindow *chatWindow = new ChatWindow();

    connect(chatWindow,&ChatWindow::signalSendMessage, m_tcpSocket,&MySocket::sendMessage);
    connect(chatWindow,&ChatWindow::signalClose, this,&MainWindow::SltFriendChatWindowClose);
    chatWindow->setCell(cell);
    chatWindow->show();

    m_chatFriendWindows.append(chatWindow);
}

void MainWindow::sltStatus(const quint8 &status, const QJsonValue &dataVal)
{
    switch (status) {
    case AddFriendFailed_NoneUser:
    {
        QMessageBox::warning(this,"添加好友","用户名不存在");
        break;
    }
    case AddFriendRequist:
    {
        addFriendRequist(dataVal);
        break;
    }
    case AddFriendReply:
    {
        addFriendReply(dataVal);    //收到回复
        break;
    }
    case GetMyFriends:
    {
        showServerFriendInfo(dataVal);  //从服务器获取好友信息
        break;
    }
    case RefreshFriends:
    {
        reFreshFriends(dataVal);
        break;
    }
    case SendMsg:
    {
        receiveMessage(dataVal);
    }
    }
}


void MainWindow::SltFriendChatWindowClose()
{
    ChatWindow *chatWindow = (ChatWindow*)sender();

    disconnect(chatWindow,&ChatWindow::signalSendMessage, m_tcpSocket,&MySocket::sendMessage);
    disconnect(chatWindow,&ChatWindow::signalClose, this,&MainWindow::SltFriendChatWindowClose);

    if(!this->isVisible() && m_chatFriendWindows.size() == 1){
        this->show();
    }
    m_chatFriendWindows.removeOne(chatWindow);
//    chatWindow->deleteLater();
}

void MainWindow::addFriendRequist(const QJsonValue &dataVal)
{
    if(!dataVal.isObject()){
        qDebug() << "添加好友转发有误";
        return;
    }
    QJsonObject jsonObj = dataVal.toObject();
    //要对方的名字
    QString name = jsonObj.value("name").toString();
    int id = jsonObj.value("id").toInt();               //好友Id
    QMessageBox::StandardButton result = QMessageBox::question(
                this,"添加好友",
                QString("'%1'添加你为好友").arg(name),
                QMessageBox::Ok | QMessageBox::Cancel,
                QMessageBox::Ok);
    if(result == QMessageBox::Ok){
        qDebug() << "已同意好友申请";
        DatabaseMsg::getInstance()->AddFriend(id,name,1);

        QJsonObject resObj;
        resObj.insert("id",MyApp::m_nId);
        resObj.insert("name",name);
        qDebug() << "输出测试resObj.value(name):" << resObj.value("name") << "name" << name;
        resObj.insert("msg","accept");
        m_tcpSocket->sendMessage(AddFriendReply,resObj);   //发送回复消息
    }else {
        qDebug() << "已拒绝" << name <<"的好友申请";

        QJsonObject resObj;
        resObj.insert("id",MyApp::m_nId);
        resObj.insert("name",name);
        resObj.insert("msg","refuse");
        m_tcpSocket->sendMessage(AddFriendReply,resObj);   //发送回复消息
    }
}

void MainWindow::addFriendReply(const QJsonValue &dataVal)
{
    if(!dataVal.isObject()){
        qDebug() << "添加好友回复出错";
        return;
    }
    QJsonObject jsonObj = dataVal.toObject();
    int id = jsonObj.value("id").toInt();
    QString friendName = jsonObj.value("name").toString();
    QString msg = jsonObj.value("msg").toString();
    if(msg.compare("accept") == 0){
        DatabaseMsg::getInstance()->AddFriend(id,friendName,1);
        qDebug() << "对方已同意你的好友申请";
        return;
    }
    DatabaseMsg::getInstance()->removeFriend(friendName);
    qDebug() << "对方拒绝了你的好友申请";

}

void MainWindow::showServerFriendInfo(const QJsonValue &dataVal)
{
    if(!dataVal.isArray()){
        qDebug() << "获取好友信息通信有误";
        return;
    }
    QJsonArray array = dataVal.toArray();
    int size = array.size();
    for(int i = 0;i < size; i++){
        QJsonObject jsonObj = array.at(i).toObject();
        int status = jsonObj.value("status").toInt();
        QString strHead = jsonObj.value("head").toString();

        if(!QFile::exists(MyApp::m_strHeadFile + strHead)){
            QJsonObject jsonObj;
            jsonObj.insert("from", MyApp::m_nId);
            jsonObj.insert("msg",strHead);
            m_tcpSocket->sendMessage(GetFile, jsonObj);

            myHelper::Sleep(100);
        }

        QQCell* cell = new QQCell;
        cell->groupName = QString(tr("我的好友"));
        cell->iconPath = MyApp::m_strHeadPath + jsonObj.value("head").toString();
        cell->type = QQCellType_Child;
        cell->name = jsonObj.value("name").toString();
        cell->subTitle = QString("当前用户状态：%1").arg(OnLine == status ? tr("在线") : tr("离线"));
        cell->id = jsonObj.value("id").toInt();
        cell->status = status;
        //        qDebug() << "获取好友信息" << cell->id << cell->name;
        ui->frindListWidget->insertQQCell(cell);
    }

    ui->frindListWidget->upload();
}

void MainWindow::reFreshFriends(const QJsonValue &dataVal)
{
    if (dataVal.isArray()) {
        QJsonArray array = dataVal.toArray();
        int nSize = array.size();
        //为了刷新出新添加的好友和头像灰度改变，先删除，再添加
        QList<QQCell *> groups = ui->frindListWidget->getCells();
        if (!groups.isEmpty()) {
            QQCell* myFriendGroup = groups.at(0); // 我的好友分组

            for (QQCell* oldChild : myFriendGroup->childs) {
                delete oldChild;
            }
            myFriendGroup->childs.clear();
        }


        for (int i = 0; i < nSize; ++i) {
            QJsonObject jsonObj = array.at(i).toObject();
            int nId = jsonObj.value("id").toInt();
            int nStatus = jsonObj.value("status").toInt();
            QString strHead = jsonObj.value("head").toString();

            QQCell* cell = new QQCell;
            cell->groupName = QString(tr("我的好友"));
            cell->iconPath = MyApp::m_strHeadPath + strHead;
            cell->type = QQCellType_Child;
            cell->name = jsonObj.value("name").toString();
            cell->subTitle = QString("当前用户状态：%1 ").arg(OnLine == nStatus ? tr("在线") : tr("离线"));
            cell->id = nId;
            cell->status = nStatus;
            ui->frindListWidget->insertQQCell(cell);
            //            qDebug() << "获取好友信息" << cell->id << cell->name;
        }

        ui->frindListWidget->upload();
    }
}

void MainWindow::receiveMessage(const QJsonValue &dataVal)
{
    if(!dataVal.isObject()){
        qDebug() << "消息解析失败";
        return;
    }
    QJsonObject jsonObj = dataVal.toObject();
    int id = jsonObj.value("id").toInt();   //好友的Id
    QString msg = jsonObj.value("msg").toString();
    QString head = jsonObj.value("head").toString();

    foreach(ChatWindow* window, m_chatFriendWindows){
        if(window->getUserId() == id){
            window->AddMessage(dataVal);
            return;
        }
    }

    //下面写没有开启这个好友聊天窗口时的逻辑（已经包含上线转发）
    ItemInfo *itemInfo = new ItemInfo(this);
    itemInfo->SetName(DatabaseMsg::getInstance()->getFriendName(id));
    itemInfo->SetDatetime(QDateTime::currentDateTime().toString("MM-dd HH:mm"));
    itemInfo->SetHeadPixmap(MyApp::m_strHeadPath + head);
    itemInfo->SetText(msg);
    itemInfo->SetOrientation(Left);
    DatabaseMsg::getInstance()->AddHistoryMsg(id, itemInfo);


    delete itemInfo;
    itemInfo = nullptr;

}


