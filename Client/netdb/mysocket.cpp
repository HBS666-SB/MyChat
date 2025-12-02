#include "mysocket.h"

#include <QJsonObject>
#include <qjsondocument.h>

MySocket::MySocket(QObject *parent)
{
    m_tcpSocket = new QTcpSocket(this);

    connect(m_tcpSocket,&QTcpSocket::connected,this,&MySocket::sltConnect);
    connect(m_tcpSocket,&QTcpSocket::readyRead,this,&MySocket::sltReadyRead);
    connect(m_tcpSocket,&QTcpSocket::disconnected,this,&MySocket::sltDisconnect);

}

MySocket::~MySocket()
{

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
    QByteArray reply = m_tcpSocket->readAll();
    qDebug() << "客户端收到消息：" << reply;
    QJsonParseError jsonError;
    QJsonDocument document = QJsonDocument::fromJson(reply,&jsonError);

    if(!document.isNull() && jsonError.error == QJsonParseError::NoError){
        QJsonObject jsonObj = document.object();
        int nType = jsonObj.value("type").toInt();
        QJsonValue dataVal = jsonObj.value("data");

        qDebug() << "消息类型：" << nType;
        signalStatus(static_cast<quint8>(nType));
    }
}

void MySocket::sltConnect()
{

}

void MySocket::sltDisconnect()
{

}

void MySocket::sendMessage(const qint8 &type, const QJsonValue &dataVal)
{
    QJsonObject json;
    json.insert("type",type);
    json.insert("data",dataVal);

    QJsonDocument document;
    document.setObject(json);
    QByteArray byteArray = document.toJson(QJsonDocument::Compact);
//    qDebug() << byteArray;
    m_tcpSocket->write(byteArray);
}
