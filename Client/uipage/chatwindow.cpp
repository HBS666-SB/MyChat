#include "chatwindow.h"
#include "ui_chatwindow.h"
#include <QPalette>
#include <QDebug>
#include <QJsonObject>
#include <QToolTip>
#include <QDateTime>
#include <QFileDialog>
#include <QIODevice>
#include <qthread.h>
#include "comapi/unit.h"
#include <basewidget/iteminfo.h>
#include <comapi/global.h>
#include <comapi/myapp.h>
#include <QKeyEvent>
#include <QMenu>
#include <netdb/filesocket.h>
#include "netdb/databasemsg.h"
#include "face/facedialog.h"

ChatWindow::ChatWindow(CustomMoveWidget *parent) : CustomMoveWidget(parent),
    ui(new Ui::ChatWindow)
{
    ui->setupUi(this);
    setQss(":/qss/resource/qss/default.css");
    setWindowFlags(Qt::FramelessWindowHint);
    ui->widgetFileBoard->hide();

    QMenu *menu = new QMenu(this);
    QAction *actionEnter = menu->addAction(QIcon(""), tr("按Enter键发送"));
    QAction *actionCtrlEnter = menu->addAction(QIcon(""), tr("按Ctrl+Enter键发送"));

    QActionGroup *actionGroup = new QActionGroup(this);
    actionGroup->addAction(actionEnter);
    actionGroup->addAction(actionCtrlEnter);
    actionEnter->setCheckable(true);
    actionCtrlEnter->setCheckable(true);
    actionEnter->setChecked(true); // 默认
    m_isEnterSend = true;

    ui->btnAction->setMenu(menu);

    connect(actionEnter, &QAction::triggered, this, [=]()
    {
        m_isEnterSend = true;
        qDebug() << "切换为：按Enter键发送消息"; });
    connect(actionCtrlEnter, &QAction::triggered, this, [=]()
    {
        m_isEnterSend = false;
        qDebug() << "切换为：按Ctrl+Enter键发送消息"; });

    // 快捷键
    ui->btnSendMsg->setShortcut(QKeySequence("alt+s"));
    ui->btnClose->setShortcut(QKeySequence("alt+c"));

    ui->textEditMsg->setFocus();
    ui->textEditMsg->installEventFilter(this);

    count = 0;
    setAttribute(Qt::WA_DeleteOnClose); // 关闭窗口即释放内存
    connect(ui->widgetBubble, SIGNAL(signalWheelUp()), this, SLOT(sltWheelUp()));
    connect(ui->widgetBubble, &BubbleList::signalDownloadFile, this, &ChatWindow::sltDownloadFile);

    m_isGroup = false;
}
int ChatWindow::count = 0;

ChatWindow::~ChatWindow()
{
    delete ui;
}

void ChatWindow::setCell(QQCell *cell, const quint8 &type)
{
    if (!m_cell)
    {
        qDebug() << "setCell为空";
    }
    m_cell = cell;
    ui->labelWinTitle->setText(QString("与 %1 聊天中").arg(cell->name));
    this->setWindowTitle(QString("与 %1 聊天中").arg(cell->name));
    ui->widgetBubble->addItems(getHistoryMsg());

    m_nChatType = type;

    //        if (0 == type) {
    //            // 加载历史
    //            ui->widgetBubble->addItems(DataBaseMagr::Instance()->QueryHistory(m_cell->id, 10));
    //            ui->tableViewGroups->setVisible(false);
    //            ui->widgetFileInfo->setVisible(true);
    //            ui->widgetFileBoard->setVisible(false);
    //            // 链接文件服务器,方便下载文件
    //            m_tcpFileSocket->ConnectToServer(MyApp::m_strHostAddr, MyApp::m_nFilePort, m_cell->id);
    //        }
    //        else {
    //            ui->tableViewGroups->setVisible(true);
    //            ui->widgetFileInfo->setVisible(false);
    //            ui->widgetFileBoard->setVisible(true);
    //            // 添加群组人员
    //            m_model->setColumnCount(2);
    //            m_model->setRowCount(3);
    //            m_model->setHorizontalHeaderLabels(QStringList() << "好友" << "状态");
    //        }
}

int ChatWindow::getUserId()
{
    if (!m_cell)
    {
        qCritical() << "[ChatWindow] getUserId: m_cell是空指针！";
        return -1; // 返回无效ID，避免崩溃
    }
    return m_cell->id;
}

QString ChatWindow::getUserName()
{
    if (!m_cell)
    {
        qCritical() << "[ChatWindow] getUserName: m_cell是空指针！";
        return "-1"; // 返回无效ID，避免崩溃
    }
    return m_cell->name;
}

QString ChatWindow::getUserHead()
{
    if (!m_cell)
    {
        qCritical() << "[ChatWindow] getUserId: m_cell是空指针！";
        return "-1"; // 返回无效ID，避免崩溃
    }
    return m_cell->iconPath;
}

void ChatWindow::AddMessage(const QJsonValue &jsonVal) // 接收消息
{
    QJsonObject jsonObj = jsonVal.toObject();
    QString msg = jsonObj.value("msg").toString();
    int type = jsonObj.value("type").toInt();
    if(type == Face && getIsGroup()) {
        msg = QString::number(jsonObj.value("msg").toInt());
    }
    QString userName = jsonObj.value("name").toString();
    QString head = jsonObj.value("head").toString();
    ItemInfo *itemInfo = new ItemInfo(this);
    if(getIsGroup()){
        itemInfo->SetName(userName);
    } else {
        itemInfo->SetName(m_cell->name);
    }

    itemInfo->SetDatetime(QDateTime::currentDateTime().toString("MM-dd HH:mm"));
    itemInfo->SetHeadPixmap(MyApp::m_strHeadPath + head);
    if (Text == static_cast<quint8>(type)) {
        itemInfo->SetText(msg);
    } else if (Face == static_cast<quint8>(type)) {
        itemInfo->SetFace(msg.toInt());
    } else if (Files == static_cast<quint8>(type))
    {
        itemInfo->SetText(msg);
        itemInfo->SetFileSizeString(jsonObj.value("size").toString());
    }
    qDebug() << "AddMessage【chatwindow144】" << "type" << type << "msg" << msg;
    itemInfo->SetMsgType(static_cast<quint8>(type));
    itemInfo->SetOrientation(Left);

    ui->widgetBubble->addItem(itemInfo);
    if(getIsGroup()) {
        int userId = jsonObj.value("userId").toInt();
        int groupId = jsonObj.value("id").toInt();
        DatabaseMsg::getInstance()->AddGroupHistoryMsg(userId, groupId, itemInfo);
    }else {
        DatabaseMsg::getInstance()->AddHistoryMsg(m_cell->id, itemInfo);
    }
}

QVector<ItemInfo *> ChatWindow::getHistoryMsg()
{
    QVector<ItemInfo *> itemArr;
    QVector<QJsonObject> vJsonObj;
    //    name, head, datetime, fizesize, content, type, direction
    if(getIsGroup()){
        qDebug() << "获取群组聊天记录";
        vJsonObj = DatabaseMsg::getInstance()->getGroupHistoryMsg(getUserId(), count++);
    } else{
        vJsonObj = DatabaseMsg::getInstance()->getHistoryMsg(m_cell->id, count++);
    }
    foreach (QJsonObject obj, vJsonObj)
    {
        ItemInfo *item = new ItemInfo(this);
        item->SetName(obj.value("name").toString());
        item->SetHeadPixmap(obj.value("head").toString());
        item->SetDatetime(obj.value("datetime").toString());
        item->SetFileSizeString(obj.value("filesize").toString());
        item->SetMsgType(static_cast<quint8>(obj.value("type").toInt()));
        if (item->GetMsgType() == Text || item->GetMsgType() == Files)
        {
            item->SetText(obj.value("content").toString());
        }
        else if (item->GetMsgType() == Face)
        {
            item->SetFace(obj.value("content").toString().toInt());
        }
        if(getIsGroup()){
            int userId = obj.value("user_id").toInt();
            item->SetOrientation(userId != MyApp::m_nId ? Left : Right);
        } else {
            item->SetOrientation(m_cell->name == obj.value("name").toString() ? Left : Right);
        }

        itemArr.append(item);
    }
    return itemArr;
}

void ChatWindow::SetIsGroup()
{
    m_isGroup = true;
}

bool ChatWindow::getIsGroup()
{
    return m_isGroup;
}

void ChatWindow::sltWheelUp()
{
    ui->widgetBubble->addItems(getHistoryMsg());
}

void ChatWindow::sendFaceMsg(int index)
{
    QJsonObject jsonObj;
    jsonObj.insert("id", getUserId());
    jsonObj.insert("msg", index);
    jsonObj.insert("type", Face);
    qDebug() << "发送信号";
    if(getIsGroup()){
        emit signalSendMessage(SendGroupMsg, QJsonValue(jsonObj));
    } else {
        emit signalSendMessage(SendFace, QJsonValue(jsonObj));
    }
    ItemInfo *itemInfo = new ItemInfo(this);
    itemInfo->SetName(MyApp::m_strUserName);
    itemInfo->SetDatetime(QDateTime::currentDateTime().toString("MM-dd HH:mm"));
    itemInfo->SetHeadPixmap(MyApp::m_strHeadPath + MyApp::m_strHeadFile);
    itemInfo->SetFace(index);
    itemInfo->SetOrientation(Right);
    itemInfo->SetText(QString::number(index + 1));
    itemInfo->SetMsgType(Face);

    ui->widgetBubble->addItem(itemInfo);
    ui->textEditMsg->clear();
    if(getIsGroup()){
        DatabaseMsg::getInstance()->AddGroupHistoryMsg(MyApp::m_nId, getUserId(), itemInfo);
    } else {
        DatabaseMsg::getInstance()->AddHistoryMsg(m_cell->id, itemInfo);
    }
}

void ChatWindow::sltDownloadFile(const QString &fileName)
{
    qDebug() << "触发文件下载，文件名：" << fileName;

    // 创建子线程和FileSocket对象
    QThread *fileRecvThread = new QThread();
    FileSocket *work = new FileSocket();
    work->moveToThread(fileRecvThread); // 移到子线程
    work->setUserId(QString::number(MyApp::m_nId)); // 设置当前用户ID
    work->setSavePath(MyApp::m_strRecvPath); // 设置接收保存路径

    // 显示文件进度面板
    ui->widgetFileInfo->show();
    ui->widgetFileBoard->show();
    ui->progressBar->setValue(0); // 重置进度条

    // 连接信号槽
    // 主线程 → 子线程：触发连接服务器
    connect(this, &ChatWindow::signalConnectFileServer,
            work, &FileSocket::connectToFileServer, Qt::QueuedConnection);

    // 子线程 → 主线程：进度更新
    connect(work, &FileSocket::progressUpdated, this, [=](quint64 processed, quint64 total) {
        if (total == 0) return;

        // 计算百分比
        int progress = static_cast<int>((processed * 100) / total);

        //仅数值变化时更新UI，避免高频刷新卡顿
        static quint64 lastProcessed = 0;
        static int lastProgress = 0;
        if (processed == lastProcessed && progress == lastProgress) return;

        // 批量更新UI
        ui->widgetFileBoard->setUpdatesEnabled(false);
        // 调用myHelper::CalcSize格式化大小（GB/MB/KB）
        ui->lineEditTotalSize->setText(myHelper::CalcSize(static_cast<qint64>(total)));
        ui->lineEditCurrSize->setText(myHelper::CalcSize(static_cast<qint64>(processed)));
        ui->progressBar->setValue(progress);
        ui->widgetFileBoard->setUpdatesEnabled(true);

        // 更新最后一次值
        lastProcessed = processed;
        lastProgress = progress;
    }, Qt::QueuedConnection);

    // 子线程 → 主线程：下载完成
    connect(work, &FileSocket::recvFinished, this, [=](const QString &savePath) {
        QMessageBox::information(this, tr("文件下载"),
                                 tr("文件下载成功！保存路径：\n%1").arg(savePath));
        ui->widgetFileBoard->hide();
        // 停止线程并清理
        fileRecvThread->quit();
    }, Qt::QueuedConnection);

    // 子线程 → 主线程：下载失败
    connect(work, &FileSocket::sendFailed, this, [=](const QString &errMsg) {
        QMessageBox::critical(this, tr("文件下载失败"), errMsg);
        ui->widgetFileBoard->hide();
        // 停止线程并清理
        fileRecvThread->quit();
    }, Qt::QueuedConnection);

    // 子线程 → 主线程：连接成功后触发下载请求
    connect(work, &FileSocket::fileServerConnected, this, [=]() {
        work->requestFile(fileName); // 发送下载请求
    }, Qt::QueuedConnection);

    connect(fileRecvThread, &QThread::finished, work, &FileSocket::deleteLater, Qt::QueuedConnection);
    connect(fileRecvThread, &QThread::finished, fileRecvThread, &QThread::deleteLater, Qt::QueuedConnection);

    // 启动线程 → 触发连接服务器
    fileRecvThread->start();
    emit signalConnectFileServer(MyApp::m_strHostAddr, MyApp::m_nFilePort);
}
void ChatWindow::changeEvent(QEvent *event)
{
    QWidget::changeEvent(event);
    switch (event->type())
    {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void ChatWindow::keyPressEvent(QKeyEvent *event)
{
}

bool ChatWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == ui->textEditMsg && event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        bool isEnterKey = (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter);

        if (isEnterKey)
        {
            if (m_isEnterSend)
            {
                if (keyEvent->modifiers() == Qt::NoModifier || keyEvent->modifiers() == Qt::KeypadModifier)
                {
                    on_btnSendMsg_clicked();
                    return true;
                }

                else if (keyEvent->modifiers() == Qt::ShiftModifier)
                {
                    return false;
                }
            }
            else
            {

                if (keyEvent->modifiers() == Qt::ControlModifier)
                {
                    on_btnSendMsg_clicked();
                    return true;
                }
            }
        }
    }
    return CustomMoveWidget::eventFilter(watched, event);
}

void ChatWindow::on_btnSendMsg_clicked()
{
    QString msg = ui->textEditMsg->toPlainText().trimmed();

    if (msg.isEmpty())
    {
        QPoint point = ui->btnSendMsg->mapToGlobal(QPoint(0, -20));
        QToolTip::showText(point, tr("发送消息不能为空"));
        return;
    }
    QJsonObject jsonObj;
    jsonObj.insert("id", getUserId());
    jsonObj.insert("msg", msg);
    jsonObj.insert("type", Text);
    qDebug() << "发送信号";
    if(getIsGroup()){
        emit signalSendMessage(SendGroupMsg, QJsonValue(jsonObj));
    } else {
        emit signalSendMessage(SendMsg, QJsonValue(jsonObj));
    }

    ItemInfo *itemInfo = new ItemInfo(this);
    itemInfo->SetName(MyApp::m_strUserName);
    itemInfo->SetDatetime(DATE_TIME);
    itemInfo->SetHeadPixmap(MyApp::m_strHeadPath + MyApp::m_strHeadFile);
    itemInfo->SetText(msg);
    itemInfo->SetOrientation(Right);

    ui->widgetBubble->addItem(itemInfo);
    ui->textEditMsg->clear();
    if(getIsGroup()){
        DatabaseMsg::getInstance()->AddGroupHistoryMsg(MyApp::m_nId, getUserId(), itemInfo);
    } else {
        DatabaseMsg::getInstance()->AddHistoryMsg(m_cell->id, itemInfo);
    }
}

void ChatWindow::on_btnSendFile_clicked()
{
    // 选择文件
    QFileDialog fileDialog(this);
    fileDialog.setWindowTitle(tr("选择要发送的文件"));
    fileDialog.setDirectory(QDir::homePath());
    fileDialog.setNameFilter(tr("所有文件 (*.*)"));
    fileDialog.setFileMode(QFileDialog::ExistingFile);
    fileDialog.setViewMode(QFileDialog::Detail);

    QString filePath = fileDialog.getOpenFileName();
    if (filePath.isEmpty())
    {
        qDebug() << "未选择任何文件，取消发送";
        return;
    }

    // 获取文件信息
    QFileInfo fileInfo(filePath);
    QString fileName = fileInfo.fileName();
    qint64 fileSize = fileInfo.size();
    QString fileSizeStr = myHelper::CalcSize(fileSize); // 格式化文件大小

    // 构造文件消息JSON（通知接收方）
    QJsonObject jsonObj;
    jsonObj.insert("id", MyApp::m_nId); // 发送方ID
    jsonObj.insert("to", getUserId());   // 接收方ID
    jsonObj.insert("msg", fileName);    // 文件名
    jsonObj.insert("size", fileSizeStr); // 格式化后的文件大小
//    jsonObj.insert("head", MyApp::m_strHeadFile);
    jsonObj.insert("type", Files); // 消息类型：文件
    if(getIsGroup()){
        emit signalSendMessage(SendGroupMsg, jsonObj);
    } else {
        emit signalSendMessage(SendFile, jsonObj);
    }
    // 本地UI显示文件消息
    ItemInfo *itemInfo = new ItemInfo(this);
    itemInfo->SetName(MyApp::m_strUserName);
    itemInfo->SetDatetime(QDateTime::currentDateTime().toString("MM-dd HH:mm"));
    itemInfo->SetHeadPixmap(MyApp::m_strHeadPath + MyApp::m_strHeadFile);
    itemInfo->SetText(fileName); // 显示文件名
    itemInfo->SetFileSizeString(fileSizeStr); // 显示格式化后的文件大小
    itemInfo->SetOrientation(Right);
    itemInfo->SetMsgType(Files); // 消息类型：文件

    ui->widgetBubble->addItem(itemInfo);
    itemInfo->SetText(filePath);
    if(getIsGroup()){
        DatabaseMsg::getInstance()->AddGroupHistoryMsg(MyApp::m_nId, getUserId(), itemInfo);
    } else {
        DatabaseMsg::getInstance()->AddHistoryMsg(m_cell->id, itemInfo);
    }

    // 显示文件发送进度面板
    ui->widgetFileBoard->show();
    ui->progressBar->setValue(0);

    // 创建子线程和FileSocket
    QThread *fileSendThread = new QThread();
    FileSocket *work = new FileSocket();
    work->moveToThread(fileSendThread);
    work->setFilePath(filePath);       // 设置发送文件路径
    work->setLoadSize(8 * 1024);       // 8KB分片
    work->setUserId(QString::number(MyApp::m_nId)); // 设置用户ID

    // 信号连接
    // 主线程 → 子线程：连接服务器、启动发送
    connect(this, &ChatWindow::signalConnectFileServer, work, &FileSocket::connectToFileServer);
    connect(this, &ChatWindow::signalStartSend, work, &FileSocket::startSendFile);

    // 子线程 → 主线程：进度更新（UI显示）
    connect(work, &FileSocket::progressUpdated, this, [=](quint64 sent, quint64 total) {
        if (total == 0) return;
        int progress = static_cast<int>((sent * 100) / total);

        // 节流：仅变化时更新
        static quint64 lastSent = 0;
        static int lastProgress = 0;
        if (sent == lastSent && progress == lastProgress) return;

        // 批量更新UI，大小显示调用CalcSize
        ui->widgetFileBoard->setUpdatesEnabled(false);
        ui->lineEditTotalSize->setText(myHelper::CalcSize(static_cast<qint64>(total)));
        ui->lineEditCurrSize->setText(myHelper::CalcSize(static_cast<qint64>(sent)));
        ui->progressBar->setValue(progress);
        ui->widgetFileBoard->setUpdatesEnabled(true);

        lastSent = sent;
        lastProgress = progress;
    }, Qt::QueuedConnection);

    // 子线程 → 主线程：发送完成
    connect(work, &FileSocket::sendFinished, this, [=]() {
        QMessageBox::information(this, tr("文件发送"), tr("文件「%1」发送成功！").arg(fileName));
        ui->widgetFileBoard->hide();
        fileSendThread->quit();
    }, Qt::QueuedConnection);

    // 子线程 → 主线程：发送失败
    connect(work, &FileSocket::sendFailed, this, [=](const QString &errMsg) {
        QMessageBox::critical(this, tr("文件发送失败"), errMsg);
        ui->widgetFileBoard->hide();
        fileSendThread->quit();
    }, Qt::QueuedConnection);

    // 连接成功后自动启动发送
    connect(work, &FileSocket::fileServerConnected, this, &ChatWindow::signalStartSend);

    // 线程结束清理
    connect(fileSendThread, &QThread::finished, work, &FileSocket::deleteLater);
    connect(fileSendThread, &QThread::finished, fileSendThread, &QThread::deleteLater);

    // 启动线程并连接服务器
    fileSendThread->start();
    emit signalConnectFileServer(MyApp::m_strHostAddr, MyApp::m_nFilePort);

    qDebug() << "开始发送文件：" << fileName << "大小：" << fileSizeStr;
}
// fileName
void ChatWindow::on_btnClose_clicked()
{
    emit signalClose();
    close();
}

void ChatWindow::on_btnWinClose_clicked()
{
    emit signalClose();
    close();
}

void ChatWindow::on_toolButton_3_clicked()
{
    faceDialog *face = new faceDialog;
    face->show();
    connect(this, &ChatWindow::signalClose, face, &faceDialog::sltClose);
    connect(face, &faceDialog::signalSelectFaceIndex, this, &ChatWindow::sendFaceMsg);
}
