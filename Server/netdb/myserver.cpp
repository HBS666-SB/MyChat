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

    qDebug() << "[消息服务器] 新客户端连接！socketDescriptor：";
}

void TcpMsgServer::SltConnected()
{

}

void TcpMsgServer::SltDisConnected()
{

}

void TcpMsgServer::SltMsgToClient(const quint8 &type, const int &id, const QJsonValue &json)
{

}
