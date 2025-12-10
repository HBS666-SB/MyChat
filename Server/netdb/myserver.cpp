#include "myserver.h"
#include <QHostAddress>
#include <QHostInfo>
#include <QJsonValue>
#include <QJsonObject>
#include "databasemag.h"
#include "comapi/unit.h"

MyServer::MyServer(QObject *parent)
{
    m_tcpServer = new QTcpServer(this);

    connect(m_tcpServer, SIGNAL(newConnection()), this, SLOT(SltNewConnection()));
}

MyServer::~MyServer()
{
    if (m_tcpServer->isListening()) m_tcpServer->close();
}


bool MyServer::StartListen(int port)
{
    m_tcpServer->listen(QHostAddress::Any,static_cast<quint16>(port));
    return true;
}

void MyServer::CloseListen()
{
    m_tcpServer->close();
}



//消息服务器
TcpMsgServer::TcpMsgServer(QObject *parent)
{

}

TcpMsgServer::~TcpMsgServer()
{

}

void TcpMsgServer::insertMessageQueue(const int &send, const int &getId, const QJsonValue &jsonVal, const quint8 &type)
{
    //    resObj.insert("requestName",senderName);
    //    resObj.insert("requestId",senderId);
    //    resObj.insert("acceptId",friendId);
    if(!jsonVal.isObject()){
        qDebug() << "server插入消息队列逻辑有误";
        return ;
    }
    QJsonObject jsonObj = jsonVal.toObject();
    DataBaseMag::getInstance()->insertMessageQueue(send,getId,type, jsonObj);
}

void TcpMsgServer::sendUserMessageQueue(const QString &userId)  //上线的Id
{
    if(userId.isEmpty()){
        qDebug() <<"转发上线消息：用户Id不能为空";
        return;
    }
    QList<QVariantMap> msgList = DataBaseMag::getInstance()->getUserMessageQueue(userId.toInt());
//    qDebug() << msgList;
//    msgMap["request_userId"] = query.value("request_userId");
//    msgMap["accept_userId"] = query.value("accept_userId");
//    msgMap["message_type"] = query.value("message_type");
    foreach(QVariantMap msg , msgList){
        quint8 type = static_cast<quint8>(msg["message_type"].toInt());
        int id = msg["request_userId"].toInt();
        int targetId = msg["accept_userId"].toInt();
        QString friendName = DataBaseMag::getInstance()->getUsernameFromId(QString::number(targetId));
        QJsonObject jsonObj = msg["data"].toJsonObject();
        jsonObj.insert("type",type);
        jsonObj.insert("name",friendName);

        qDebug() << "myServer:80 转发离线队列消息给" << friendName;
        QJsonValue sendVal = jsonObj;
        SltPrivateMsgToClient(id, targetId, type,sendVal);
    }


}

void TcpMsgServer::SltTransFileToClient(const int &userId, const QJsonValue &jsonVal)
{

}

void TcpMsgServer::SltNewConnection()
{
    ClientSocket *client = new ClientSocket(this, m_tcpServer->nextPendingConnection());
    m_clients.append(client); // 加入客户端列表管理


    // 信号槽绑定
    connect(client, &ClientSocket::signalConnected, this, &TcpMsgServer::SltConnected);
    connect(client, &ClientSocket::signalDisConnected, this, &TcpMsgServer::SltDisConnected);
    connect(client, &ClientSocket::signalLoginSuccess, this, &TcpMsgServer::SltLoginSuccess);
    connect(client, &ClientSocket::signalPrivateMsgToClient, this, &TcpMsgServer::SltPrivateMsgToClient);

    qDebug() << "[消息服务器] 新客户端连接！socketDescriptor：";
}

void TcpMsgServer::SltConnected()
{

}

void TcpMsgServer::SltDisConnected(ClientSocket *client)
{
    m_clients.removeOne(client);
    m_clientHash.remove(QString::number(client->GetUserId()));
    client->deleteLater();
}

void TcpMsgServer::SltPrivateMsgToClient(const int &id, const int &targetId, const quint8 &type, const QJsonValue &jsonVal)
{
    if(targetId < 0){
        qDebug() << "单播目标Id为空";
        return;
    }
    ClientSocket *targetClient = m_clientHash.value(QString::number(targetId), nullptr);
    if (!targetClient) {
        qDebug() << "[单播] 目标用户不存在或已下线：" << targetId;
        insertMessageQueue(id, targetId, jsonVal, type);
        return;
    }

    //转发
    targetClient->SltSendMessage(type, QJsonValue(jsonVal));
//    qDebug() << "server.cpp转发出去135" << jsonVal;
}

void TcpMsgServer::SltLoginSuccess(ClientSocket *client, const QString &userId)
{
    if(!client || userId.isEmpty()){
        qDebug() << "客户端登录ID为空，拒绝绑定";
        client->deleteLater();
        return;
    }

    if(m_clientHash.contains(userId)){
        qDebug() << "不能重复登录";
        return;
    }
    m_clientHash.insert(userId, client);
    sendUserMessageQueue(userId);
    qDebug() << "客户端登录成功，ID：" << userId << "，当前在线数：" << m_clientHash.size();

}

