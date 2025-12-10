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

int ClientSocket::GetUserId()
{
    return  m_nId;
}

void ClientSocket::Close()
{

}

void ClientSocket::sendMsgType(const quint8 &nType, const QJsonValue &dataVal)
{
    switch (nType) {
    case Login:
    {
        ParseLogin(dataVal);
        break;
    }
    case Register:
    {
        ParseReister(dataVal);
        break;
    }
    case AddFriend:
    {
        ParseAddFriend(dataVal);
        break;
    }
    case UserOnLine:    //用户上线
    {
        ParseUserOnline(dataVal);
        break;
    }
    case AddFriendReply:
    {
        ParseAddFriendReply(dataVal);
        break;
    }
    case RefreshFriends:
    {
        ParseRefreshFriend(dataVal);
        break;
    }
    case SendMsg:
    {
        ParseSendMsg(dataVal);
        break;
    }
    }
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
    QByteArray jsonData = document.toJson(QJsonDocument::Compact);
    qDebug() << "服务器发送消息：" << jsonData;

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

void ClientSocket::SltConnected()
{
    qDebug() << "服务器端socket检测到新的连接";
}

void ClientSocket::SltDisconnected()
{
    qDebug() << "服务器端socket检测到有用户下线";
    DataBaseMag::getInstance()->UpdateUserStatus(m_nId, OffLine);
    Q_EMIT signalDisConnected(this);
}

void ClientSocket::SltReadyRead()
{
    QByteArray newData = m_tcpSocket->readAll();
    m_recvBuffer.append(newData);
    qDebug() << "服务器收到消息：" << newData;
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
            quint8 nType = static_cast<quint8>(jsonObj["type"].toInt());            QString from = jsonObj["from"].toString();
            QJsonValue dataVal = jsonObj["data"];
            m_nId = jsonObj["from"].toInt();

            sendMsgType(nType,dataVal);
        }
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

        QString msg = jsonObj.value("msg").toString();
        if(msg == "ok"){  //不在线
            SltSendMessage(LoginSuccess, jsonObj);
            emit signalLoginSuccess(this,QString::number(m_nId));
            qDebug() << "登陆成功" << dataVal;
            SltSendMessage(GetMyFriends,DataBaseMag::getInstance()->getMyFriends(m_nId));
            return;
        }else if(msg == "OnLine"){
            SltSendMessage(LoginRepeat, jsonObj);
        }else{
            SltSendMessage(LoginPasswdError, jsonObj);
        }

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

//dataVal :senderId accessName
void ClientSocket::ParseAddFriend(const QJsonValue &dataVal)
{
    if(!dataVal.isObject()){
        qDebug() << "添加好友传入参数有误";
        return;
    }
    QJsonObject jsonObj = dataVal.toObject();
    QString friendName = jsonObj.value("name").toString();
    int senderId = jsonObj.value("id").toInt();
    //传输的obj先不管
    int friendId = DataBaseMag::getInstance()->getIdFromUsername(friendName);
    if(!DataBaseMag::getInstance()->haveUser(friendName)){
        SltSendMessage(AddFriendFailed_NoneUser,jsonObj);
        return;
    }
//    if(DataBaseMag::getInstance()->isFriend(GetUserId(),QString::number(friendId))){
//        SltSendMessage(AddFriendOk,jsonObj);
//        return;
//    }
    QString senderName = DataBaseMag::getInstance()->getUsernameFromId(QString::number(senderId));

//    qDebug() << jsonObj << "senderName" << senderName << "senderId" << senderId;
    QJsonObject resObj;

    resObj.insert("name",senderName);   //requestName
    resObj.insert("id",senderId);
    resObj.insert("targetId",friendId);
    QJsonValue resVal = resObj;

    emit signalPrivateMsgToClient(m_nId ,friendId, AddFriendRequist,resVal);
}

void ClientSocket::ParseAddFriendReply(const QJsonValue &dataVal)
{
//    resObj.insert("id",MyApp::m_nId); //
//    resObj.insert("name",name);       //这个名字是要转发给的对象的名字
//    resObj.insert("msg","refuse");
    if(!dataVal.isObject()){
        qDebug() << "添加好友回复转发失败";
        return;
    }
    QJsonObject jsonObj = dataVal.toObject();
    int id = jsonObj.value("id").toInt();
    QString name = jsonObj.value("name").toString();
    QString msg = jsonObj.value("msg").toString();
    QJsonObject resObj;
    resObj.insert("id", id);
    resObj.insert("name",DataBaseMag::getInstance()->getUsernameFromId(QString::number(id)));
    resObj.insert("msg",msg);
    if(msg.compare("accept") == 0){
        DataBaseMag::getInstance()->addFriend(id, DataBaseMag::getInstance()->getIdFromUsername(name));
        DataBaseMag::getInstance()->addFriend(DataBaseMag::getInstance()->getIdFromUsername(name), id);
    }


    int sendId = DataBaseMag::getInstance()->getIdFromUsername(name);
    if(!DataBaseMag::getInstance()->isOnline(sendId)){
        DataBaseMag::getInstance()->insertMessageQueue(static_cast<int>(id),static_cast<int>(sendId),AddFriendReply, dataVal);
        return;
    }
    emit signalPrivateMsgToClient(m_nId, sendId,AddFriendReply, resObj);

    qDebug() << "套接字发送AddFriendReply信号clientsocket.cpp" << resObj;

}

void ClientSocket::ParseAddGroup(const QJsonValue &dataVal)
{

}

void ClientSocket::ParseCreateGroup(const QJsonValue &dataVal)
{

}

void ClientSocket::ParseGetMyFriend(const QJsonValue &dataVal)
{
// 发送Jsonarray  status  head    name    id
}

void ClientSocket::ParseGetMyGroups(const QJsonValue &dataVal)
{

}

void ClientSocket::ParseRefreshFriend(const QJsonValue &dataVal)
{
    if(!dataVal.isObject()){
        qDebug() << "客户端发送的刷新请求出错 dataVal = " << dataVal;
        return;

    }
    QJsonObject jsonObj = dataVal.toObject();
    int id = jsonObj.value("id").toInt();

    QJsonValue sendVal = DataBaseMag::getInstance()->getMyFriends(id);


    SltSendMessage(RefreshFriends, sendVal);
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

void ClientSocket::ParseSendMsg(const QJsonValue &dataVal)
{
    if(!dataVal.isObject()){
        qDebug() << "发送消息功能出错，收到的信息不是JsonObject";
        return;
    }
    QJsonObject jsonObj = dataVal.toObject();
    QJsonObject resObj;
    resObj.insert("id", m_nId);
    resObj.insert("msg",jsonObj.value("msg"));

    emit signalPrivateMsgToClient(m_nId, jsonObj.value("id").toInt(),SendMsg, QJsonValue(resObj));

}
