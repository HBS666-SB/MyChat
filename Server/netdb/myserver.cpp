#include "myserver.h"
#include <QHostAddress>
#include <QHostInfo>

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

void TcpMsgServer::SltTransFileToClient(const int &userId, const QJsonValue &json)
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

void TcpMsgServer::SltPrivateMsgToClient(const quint8 &type, const QString &targetId, const QJsonValue &jsonVal)
{
    if(targetId.isEmpty()){
        qDebug() << "单播目标Id为空";
        return;
    }
    ClientSocket *targetClient = m_clientHash.value(targetId, nullptr);
    if (!targetClient) {
        qDebug() << "[单播] 目标用户不存在或已下线：" << targetId;
        return;
    }

    //转发
    targetClient->SltSendMessage(type, jsonVal);
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
    qDebug() << "客户端登录成功，ID：" << userId << "，当前在线数：" << m_clientHash.size();
}
