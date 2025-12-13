#include "chatwindow.h"
#include "ui_chatwindow.h"
#include <QPalette>
#include <QDebug>
#include <QJsonObject>
#include <QToolTip>
#include <QDateTime>
#include "comapi/unit.h"
#include <basewidget/iteminfo.h>
#include <comapi/myapp.h>
#include <QKeyEvent>
#include <QMenu>
#include "netdb/databasemsg.h"
#include "face/facedialog.h"

ChatWindow::ChatWindow(CustomMoveWidget *parent) :
    CustomMoveWidget(parent),
    ui(new Ui::ChatWindow)
{
    ui->setupUi(this);
    setQss(":/qss/resource/qss/default.css");
    setWindowFlags(Qt::FramelessWindowHint);
    ui->widgetFileBoard->hide();

    QMenu *menu = new QMenu(this);
    QAction *actionEnter = menu->addAction(QIcon(""),tr("按Enter键发送"));
    QAction *actionCtrlEnter = menu->addAction(QIcon(""),tr("按Ctrl+Enter键发送"));

    QActionGroup *actionGroup = new QActionGroup(this);
    actionGroup->addAction(actionEnter);
    actionGroup->addAction(actionCtrlEnter);
    actionEnter->setCheckable(true);
    actionCtrlEnter->setCheckable(true);
    actionEnter->setChecked(true);  //默认
    m_isEnterSend = true;

    ui->btnAction->setMenu(menu);

    connect(actionEnter, &QAction::triggered, this, [=]() {
        m_isEnterSend = true;
        qDebug() << "切换为：按Enter键发送消息";
    });
    connect(actionCtrlEnter, &QAction::triggered, this, [=]() {
        m_isEnterSend = false;
        qDebug() << "切换为：按Ctrl+Enter键发送消息";
    });

    // 快捷键
    ui->btnSendMsg->setShortcut(QKeySequence("alt+s"));
    ui->btnClose->setShortcut(QKeySequence("alt+c"));


    ui->textEditMsg->setFocus();
    ui->textEditMsg->installEventFilter(this);

    count = 0;
    setAttribute(Qt::WA_DeleteOnClose); //关闭窗口即释放内存
    connect(ui->widgetBubble,SIGNAL(signalWheelUp()), this, SLOT(sltWheelUp()));

}
int ChatWindow::count = 0;

ChatWindow::~ChatWindow()
{
    delete ui;

}

void ChatWindow::setCell(QQCell *cell, const quint8 &type)
{
    if(!m_cell){
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
    if(!m_cell){
        qCritical() << "[ChatWindow] getUserId: m_cell是空指针！";
        return -1; // 返回无效ID，避免崩溃
    }
    return m_cell->id;
}

QString ChatWindow::getUserName()
{
    if(!m_cell){
        qCritical() << "[ChatWindow] getUserName: m_cell是空指针！";
        return "-1"; // 返回无效ID，避免崩溃
    }
    return m_cell->name;
}

QString ChatWindow::getUserHead()
{
    if(!m_cell){
        qCritical() << "[ChatWindow] getUserId: m_cell是空指针！";
        return "-1"; // 返回无效ID，避免崩溃
    }
    return m_cell->iconPath;
}

void ChatWindow::AddMessage(const QJsonValue &jsonVal)  //接收消息
{
    QJsonObject jsonObj = jsonVal.toObject();
    QString msg = jsonObj.value("msg").toString();

    ItemInfo *itemInfo = new ItemInfo(this);
    itemInfo->SetName(m_cell->name);
    itemInfo->SetDatetime(QDateTime::currentDateTime().toString("MM-dd HH:mm"));
    itemInfo->SetHeadPixmap(m_cell->iconPath);
    itemInfo->SetText(msg);
    itemInfo->SetOrientation(Left);

    ui->widgetBubble->addItem(itemInfo);
    DatabaseMsg::getInstance()->AddHistoryMsg(m_cell->id, itemInfo);
}

QVector<ItemInfo *> ChatWindow::getHistoryMsg()
{
    QVector<ItemInfo *> itemArr;
//    name, head, datetime, fizesize, content, type, direction
    QVector<QJsonObject> vJsonObj = DatabaseMsg::getInstance()->getHistoryMsg(m_cell->id, count++);
    foreach(QJsonObject obj, vJsonObj){
        ItemInfo* item = new ItemInfo(this);
        item->SetName(obj.value("name").toString());
        item->SetHeadPixmap(obj.value("head").toString());
        item->SetDatetime(obj.value("datetime").toString());
        item->SetFileSizeString(obj.value("filesize").toString());
        item->SetText(obj.value("content").toString());
        item->SetMsgType(static_cast<quint8>(obj.value("type").toInt()));
        item->SetOrientation(m_cell->name == obj.value("name").toString() ? Left : Right);
        itemArr.append(item);
    }
    return itemArr;
}

void ChatWindow::sltWheelUp()
{
   ui->widgetBubble->addItems(getHistoryMsg());
}

void ChatWindow::changeEvent(QEvent *event)
{
    QWidget::changeEvent(event);
    switch (event->type()) {
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
    if (watched == ui->textEditMsg && event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        bool isEnterKey = (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter);

        if (isEnterKey) {
            if (m_isEnterSend) {
                if (keyEvent->modifiers() == Qt::NoModifier || keyEvent->modifiers() == Qt::KeypadModifier) {
                    on_btnSendMsg_clicked();
                    return true;
                }

                else if (keyEvent->modifiers() == Qt::ShiftModifier) {
                    return false;
                }
            } else {

                if (keyEvent->modifiers() == Qt::ControlModifier) {
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

    if(msg.isEmpty()){
        QPoint point = ui->btnSendMsg->mapToGlobal(QPoint(0, -20));
        QToolTip::showText(point, tr("发送消息不能为空"));
        return;
    }
    QJsonObject jsonObj;
    jsonObj.insert("id", getUserId());
    jsonObj.insert("msg",msg);
    qDebug() << "发送信号";
    emit signalSendMessage(SendMsg, QJsonValue(jsonObj));

    ItemInfo *itemInfo = new ItemInfo(this);
    itemInfo->SetName(MyApp::m_strUserName);
    itemInfo->SetDatetime(QDateTime::currentDateTime().toString("MM-dd HH:mm"));
    itemInfo->SetHeadPixmap(MyApp::m_strHeadPath + MyApp::m_strHeadFile);
    itemInfo->SetText(msg);
    itemInfo->SetOrientation(Right);

    ui->widgetBubble->addItem(itemInfo);
    ui->textEditMsg->clear();
    DatabaseMsg::getInstance()->AddHistoryMsg(m_cell->id, itemInfo);

}

void ChatWindow::on_btnSendFile_clicked()
{
    if(ui->widgetFileBoard->isHidden()){
        ui->widgetFileBoard->show();
        return;
    }
    ui->widgetFileBoard->hide();
}

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
}
