#include "chatwindow.h"
#include "ui_chatwindow.h"
#include <QPalette>
#include <QDebug>
#include <QJsonObject>
#include "comapi/unit.h"

ChatWindow::ChatWindow(CustomMoveWidget *parent) :
    CustomMoveWidget(parent),
    ui(new Ui::ChatWindow)
{
    ui->setupUi(this);
    setQss(":/qss/resource/qss/default.css");
    setWindowFlags(Qt::FramelessWindowHint);


}

ChatWindow::~ChatWindow()
{
    delete ui;
}

void ChatWindow::setCell(QQCell *cell, const quint8 &type)
{
    m_cell = cell;
        ui->labelWinTitle->setText(QString("与 %1 聊天中").arg(cell->name));
        this->setWindowTitle(QString("与 %1 聊天中").arg(cell->name));

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

QString ChatWindow::getAddr() const
{

}

int ChatWindow::getUserId()
{
    if(!m_cell){
        qCritical() << "[ChatWindow] getUserId: m_cell是空指针！";
        return -1; // 返回无效ID，避免崩溃
    }
    return m_cell->id;
}


void ChatWindow::on_btnWinClose_clicked()
{
    emit signalClose();
    close();
}

void ChatWindow::on_btnSendMsg_clicked()
{
    QString msg = ui->textEditMsg->toPlainText();

    QJsonObject jsonObj;
    jsonObj.insert("id", getUserId());
    jsonObj.insert("msg",msg);
    qDebug() << "发送信号";
    emit signalSendMessage(SendMsg, QJsonValue(jsonObj));

}

