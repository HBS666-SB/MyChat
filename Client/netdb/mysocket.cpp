#include "mysocket.h"
#include <QJsonObject>
#include <qjsondocument.h>
#include "comapi/unit.h"
#include <comapi/myapp.h>

MySocket::MySocket(QObject *parent)
{
    m_tcpSocket = new QTcpSocket(this);
    m_nId = MyApp::m_nId;
    connect(m_tcpSocket,&QTcpSocket::connected,this,&MySocket::sltConnect);
    connect(m_tcpSocket,&QTcpSocket::readyRead,this,&MySocket::sltReadyRead);
    connect(m_tcpSocket,&QTcpSocket::disconnected,this,&MySocket::sltDisconnect);

}

MySocket::~MySocket()
{

}

void MySocket::CheckConnected()
{
    if (m_tcpSocket->state() != QTcpSocket::ConnectedState)
    {
        m_tcpSocket->connectToHost(MyApp::m_strHostAddr, MyApp::m_nMsgPort);
    }
}

void MySocket::ColseConnected()
{
    if (m_tcpSocket->isOpen()) m_tcpSocket->abort();
}


void MySocket::connectToHost(const QString &ip, quint16 port)
{
    // 先断开已有连接,避免重复连接
    if (m_tcpSocket->state() == QAbstractSocket::ConnectedState) {
        m_tcpSocket->disconnectFromHost();
        m_tcpSocket->waitForDisconnected(1000); // 等待1秒断开
    }
    // 转发给内部的 m_tcpSocket，建立连接
    m_tcpSocket->connectToHost(ip, port);

    qDebug() << "尝试连接服务器";
}

void MySocket::connectToHost(const QHostAddress &host, quint16 port)
{
    if (m_tcpSocket->state() == QAbstractSocket::ConnectedState) {
        m_tcpSocket->disconnectFromHost();
        m_tcpSocket->waitForDisconnected(1000);
    }
    m_tcpSocket->connectToHost(host, port);

}

void MySocket::sltReadyRead()
{
    QByteArray newData = m_tcpSocket->readAll();
    m_recvBuffer.append(newData);
    qDebug() << "客户端收到消息：" << newData;
    while (true) {
        // 先检查是否够4字节长度头
        if (m_recvBuffer.size() < 4) {
            break; // 数据不足，等待下次
        }

        quint32 dataLen = 0;
        dataLen |= (static_cast<quint8>(m_recvBuffer[0]) << 24);
        dataLen |= (static_cast<quint8>(m_recvBuffer[1]) << 16);
        dataLen |= (static_cast<quint8>(m_recvBuffer[2]) << 8);
        dataLen |= static_cast<quint8>(m_recvBuffer[3]);

        if (m_recvBuffer.size() < 4 + dataLen) {
            break; // 数据未收全，等待下次
        }
        QByteArray jsonData = m_recvBuffer.mid(4, dataLen);
        m_recvBuffer.remove(0, 4 + dataLen);

        QJsonDocument doc = QJsonDocument::fromJson(jsonData);
        if (doc.isObject()) {
            QJsonObject jsonObj = doc.object();
            quint8 nType = jsonObj["type"].toInt();
            QString from = jsonObj["from"].toString();
            QJsonValue dataVal = jsonObj["data"];

            sendMsgType(nType,dataVal);
        }
    }
}

void MySocket::sltConnect()
{
    emit signalConnectSuccess();
}

void MySocket::sltDisconnect()
{
    qDebug() << "客户端断开连接";
}


void MySocket::sendMessage(const quint8 &type, const QJsonValue &dataVal)
{
    // 连接服务器
    if (!m_tcpSocket->isOpen()) {
        m_tcpSocket->connectToHost(MyApp::m_strHostAddr, MyApp::m_nMsgPort);
        m_tcpSocket->waitForConnected(1000);
    }
    // 超时1s后还是连接不上，直接返回
    if (!m_tcpSocket->isOpen()) return;
    QJsonObject json;
    json.insert("type",type);
    json.insert("from", MyApp::m_nId);
    json.insert("data",dataVal);

    QJsonDocument document;
    document.setObject(json);
    QByteArray jsonData = document.toJson(QJsonDocument::Compact);
    qDebug() << "客户端发送消息：" << jsonData;

    //防止粘包
    quint32 dataLen = static_cast<quint32>(jsonData.size());
    QByteArray lenBytes;
    lenBytes.resize(4);
    lenBytes[0] = (dataLen >> 24) & 0xFF;
    lenBytes[1] = (dataLen >> 16) & 0xFF;
    lenBytes[2] = (dataLen >> 8) & 0xFF;
    lenBytes[3] = dataLen & 0xFF;
    QByteArray sendData = lenBytes + jsonData;

    m_tcpSocket->write(sendData);
}

void MySocket::sendMsgType(const quint8 &nType, const QJsonValue &dataVal)
{
    switch (nType) {
    case LoginSuccess:
    {
        emit signalStatus(LoginSuccess,dataVal);
        break;
    }
    case LoginRepeat:
    {
        emit signalStatus(LoginRepeat,dataVal);
        break;
    }
    case LoginPasswdError:
    {
        emit signalStatus(LoginPasswdError,dataVal);
        break;
    }
    case RegisterOk:
    {
        emit signalStatus(RegisterOk,dataVal);
        break;
    }
    case RegisterFailed:
    {
        emit signalStatus(RegisterFailed,dataVal);
        break;
    }
    case AddFriendFailed_NoneUser:
    {
        emit signalStatus(AddFriendFailed_NoneUser,dataVal);
        break;
    }
    case AddFriendRequist:
    {
        emit signalStatus(AddFriendRequist, dataVal);
        break;
    }
    case AddFriendReply:
    {
        emit signalStatus(AddFriendReply,dataVal);
        break;
    }
    case GetMyFriends:
    {
        emit signalStatus(GetMyFriends, dataVal);
        break;
    }
    case RefreshFriends:
    {
        emit signalStatus(RefreshFriends, dataVal);
        break;
    }

    case SendMsg:
    {
        emit signalStatus(SendMsg, dataVal);
        break;
    }
    case SendFace:
    {
        emit signalStatus(SendFace, dataVal);
        break;
    }
    case SendFile:
    {
        emit signalStatus(SendFile, dataVal);
        break;
    }
    case DeleteFriend:
    {
        emit signalStatus(DeleteFriend, dataVal);
        break;
    }
    case CreateGroup:
    {
        emit signalStatus(CreateGroup, dataVal);
        break;
    }
    case GetMyGroups:
    {
        emit signalStatus(GetMyGroups, dataVal);
        break;
    }
    case RefreshGroups:
    {
        emit signalStatus(RefreshGroups, dataVal);
        break;
    }
    case AddGroupRequist:
    {
        emit signalStatus(AddGroupRequist, dataVal);
        break;
    }
    case AddGroupReply:
    {
        emit signalStatus(AddGroupReply, dataVal);
        break;
    }
    case AddGroupAccept:
    {
        emit signalStatus(AddGroupAccept, dataVal);
        break;
    }
    }
}


QJsonValue MySocket::GetUserId()
{
    return MyApp::m_nId;
}
