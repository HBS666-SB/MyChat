#ifndef CLIENTSOCKET_H
#define CLIENTSOCKET_H

#include <QObject>
#include <QTcpSocket>
#include <QFile>
#include <QApplication>


// 服务端socket管理类
class ClientSocket : public QObject
{
    Q_OBJECT
public:
    explicit ClientSocket(QObject *parent = 0, QTcpSocket *tcpSocket = NULL);
    ~ClientSocket();

    int GetUserId();
    void Close();
    void sendMsgType(const quint8 &nType, const QJsonValue &dataVal);
signals:
    void signalConnected();
    void signalDisConnected(ClientSocket *client);
    void signalDownloadFile(const QJsonValue &json);
    void signalLoginSuccess(ClientSocket *client, const QString &userId);
    //id发给targetId
    void signalPrivateMsgToClient(const int &id,const int &targetId, const quint8 &type, const QJsonValue &json);
    void signalGroupMsgToClient(const int &id, const int &groupId, const quint8 &type, const QJsonValue &json);
    void signalAddGroupMembers(const int &groupId, const int &userId);
    void signalRemoveGroupMembers(const int &groupId, const int &userId);

public slots:

private:
    QTcpSocket *m_tcpSocket;
    int m_nId;  //储存用户的id（唯一字段）识别每个套接字对应的用户
    QByteArray m_recvBuffer;

public slots:
    // 消息回发
    void SltSendMessage(const quint8 &type, const QJsonValue &json);

private slots:
    void SltConnected();
    void SltDisconnected();
    void SltReadyRead();

private:
    // 消息解析和抓转发处理
    void ParseLogin(const QJsonValue &dataVal);     //同步回复
    void ParseUserOnline(const QJsonValue &dataVal);
    void ParseLogout(const QJsonValue &dataVal);
    void ParseUpdateUserHead(const QJsonValue &dataVal);

    void ParseReister(const QJsonValue &dataVal);
    void ParseAddFriend(const QJsonValue &dataVal);
    void ParseAddFriendReply(const QJsonValue &dataVal);
    void ParseAddGroupRequist(const QJsonValue &dataVal);
    void ParseAddGroupReply(const QJsonValue &dataVal);
    void ParseCreateGroup(const QJsonValue &dataVal);

    void ParseGetMyFriend(const QJsonValue &dataVal);
    void ParseGetMyGroups(const QJsonValue &dataVal);

    void ParseDeleteMyFriend(const QJsonValue &dataVal);

    void ParseRefreshFriend(const QJsonValue &dataVal);
    void ParseRefreshGroups(const QJsonValue &dataVal);

    void ParseFriendMessages(const QByteArray &reply);
    void ParseGroupMessages(const QByteArray &reply);
    void ParseFaceMessages(const QByteArray &reply);

    void ParseSendMsg(const QJsonValue &dataVal);   //发送信息
    void ParseSendFace(const QJsonValue &dataVal);  //发送表情
    void ParseSendFile(const QJsonValue &dataVal);
};


#endif // CLIENTSOCKET_H
