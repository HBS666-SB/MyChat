#include "mysocket.h"
#include <QJsonObject>
#include <qjsondocument.h>
#include "comapi/unit.h"
#include <comapi/myapp.h>

MySocket::MySocket(QObject *parent)
{
    m_tcpSocket = new QTcpSocket(this);
    m_nId = -1;
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
    QByteArray reply = m_tcpSocket->readAll();
    qDebug() << "客户端收到消息：" << reply;
    QJsonParseError jsonError;
    QJsonDocument document = QJsonDocument::fromJson(reply,&jsonError);

    if(!document.isNull() && jsonError.error == QJsonParseError::NoError){
        QJsonObject jsonObj = document.object();
        int nType = jsonObj.value("type").toInt();
        MyApp::m_nId = jsonObj.value("from").toInt();
        QJsonValue dataVal = jsonObj.value("data");

        switch (nType) {
        case Login:
        {
            ParseLogin(dataVal);
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
        case AddFriendOk:
        {

            emit signalStatus(AddFriendOk,dataVal);
            break;
        }
        case AddFriendFailed:
        {
            emit signalStatus(AddFriendFailed,dataVal);
            break;
        }
        case AddFriendFailed_NoneUser:
        {
            emit signalStatus(AddFriendFailed_NoneUser,dataVal);
            break;
        }

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

void MySocket::sendMessage(const qint8 &type, const QJsonValue &dataVal)
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
    json.insert("from", m_nId);
    json.insert("data",dataVal);

    QJsonDocument document;
    document.setObject(json);
    QByteArray byteArray = document.toJson(QJsonDocument::Compact);
    qDebug() << "客户端发送消息：" << byteArray;
    m_tcpSocket->write(byteArray);
}

void MySocket::ParseLogin(const QJsonValue &dataVal)
{
    if(!dataVal.isObject()){
        qDebug() << "登录功能出错";
    }
    QJsonObject jsonObj = dataVal.toObject();
    qDebug() << dataVal;
    MyApp::m_strHeadFile = jsonObj.value("head").toString();
    m_nId = jsonObj.value("id").toInt();
    QString msg = jsonObj.value("msg").toString();
    if(msg == "ok"){
        emit signalStatus(LoginSuccess,dataVal);
    }else if(m_nId == -2){
        emit signalStatus(LoginRepeat,dataVal);
    }
}

QJsonValue MySocket::GetUserId()
{
    return MyApp::m_nId;
}
