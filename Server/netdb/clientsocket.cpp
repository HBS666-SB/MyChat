#include "clientsocket.h"
#include "databasemag.h"
#include "databasemag.h"
#include "comapi/unit.h"
#include <QJsonDocument>
#include <QJsonObject>

ClientSocket::ClientSocket(QObject *parent, QTcpSocket *tcpSocket)
{
    m_nId = -1;

    m_tcpSocket = tcpSocket; // 优先使用传入的 Socket（服务器端场景）
    if (!m_tcpSocket) { // 只有传入为空时，才新建（客户端主动连接场景）
        m_tcpSocket = new QTcpSocket(this);
    }

    connect(m_tcpSocket, SIGNAL(readyRead()), this, SLOT(SltReadyRead()));
    connect(m_tcpSocket, SIGNAL(connected()), this, SLOT(SltConnected()));
    connect(m_tcpSocket, SIGNAL(disconnected()), this, SLOT(SltDisconnected()));

}

ClientSocket::~ClientSocket()
{

}

int ClientSocket::GetUserId() const
{
    return  m_nId;
}

void ClientSocket::Close()
{

}

void ClientSocket::SltSendMessage(const quint8 &type, const QJsonValue &jsonVal)
{
    if (!m_tcpSocket->isOpen()) return;

    // 构建 Json 对象
    QJsonObject jsonObj;
    jsonObj.insert("type", type);
    jsonObj.insert("from", m_nId);
    jsonObj.insert("data", jsonVal);

    // 构建 Json 文档
    QJsonDocument document;
    document.setObject(jsonObj);

    qDebug() << "服务器端m_tcpSocket->write:" << document.toJson(QJsonDocument::Compact);

    m_tcpSocket->write(document.toJson(QJsonDocument::Compact));
}

void ClientSocket::SltConnected()
{
    qDebug() << "服务器端socket检测到新的连接";
}

void ClientSocket::SltDisconnected()
{
    qDebug() << "服务器端socket检测到有用户下线";
    DataBaseMag::getInstance()->UpdateUserStatus(m_nId, OffLine);
    Q_EMIT signalDisConnected();
}

void ClientSocket::SltReadyRead()
{
    QByteArray reply = m_tcpSocket->readAll();
//    qDebug() << reply;
    QJsonParseError jsonError;
    QJsonDocument document = QJsonDocument::fromJson(reply,&jsonError);

    int  nType;
    QJsonValue dataVal;
    if(!document.isNull() && jsonError.error == QJsonParseError::NoError){
        QJsonObject jsonObj = document.object();
        nType = jsonObj.value("type").toInt();
        dataVal = jsonObj.value("data");

//        qDebug() << "消息类型：" << nType;

    }

    switch (nType) {
    case Login:
    {
        ParseLogin(dataVal);
        break;
    }
    case Register:
        ParseReister(dataVal);
        break;
    }
}

void ClientSocket::ParseLogin(const QJsonValue &dataVal)
{
    if(dataVal.isObject()){
        QJsonObject dataObj = dataVal.toObject();
        QString strName = dataObj.value("name").toString();
        QString strPwd = dataObj.value("passwd").toString();
//        qDebug() << "登录" << strName << strPwd;
        QJsonObject jsonObj = DataBaseMag::getInstance()->userLogin(strName, strPwd);

        m_nId = jsonObj.value("id").toInt();

        if (m_nId > 0) emit signalConnected();
        // 发送查询结果至客户端
        SltSendMessage(Login, jsonObj);

    }
}

void ClientSocket::ParseUserOnline(const QJsonValue &dataVal)
{

}

void ClientSocket::ParseLogout(const QJsonValue &dataVal)
{

}

void ClientSocket::ParseUpdateUserHead(const QJsonValue &dataVal)
{

}

void ClientSocket::ParseReister(const QJsonValue &dataVal)
{
    if(dataVal.isObject()){
        QJsonObject jsonObj = dataVal.toObject();
        QString name = jsonObj.value("name").toString();
        QString passwd = jsonObj.value("passwd").toString();

        SltSendMessage(DataBaseMag::getInstance()->userRegister(name,passwd), jsonObj);
    }
}

void ClientSocket::ParseAddFriend(const QJsonValue &dataVal)
{

}

void ClientSocket::ParseAddGroup(const QJsonValue &dataVal)
{

}

void ClientSocket::ParseCreateGroup(const QJsonValue &dataVal)
{

}

void ClientSocket::ParseGetMyFriend(const QJsonValue &dataVal)
{

}

void ClientSocket::ParseGetMyGroups(const QJsonValue &dataVal)
{

}

void ClientSocket::ParseRefreshFriend(const QJsonValue &dataVal)
{

}

void ClientSocket::ParseRefreshGroups(const QJsonValue &dataVal)
{

}

void ClientSocket::ParseFriendMessages(const QByteArray &reply)
{

}

void ClientSocket::ParseGroupMessages(const QByteArray &reply)
{

}

void ClientSocket::ParseFaceMessages(const QByteArray &reply)
{

}
