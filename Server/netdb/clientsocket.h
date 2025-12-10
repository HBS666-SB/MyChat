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
    void signalPrivateMsgToClient(const int &id,const int &targetId, const quint8 &type, const QJsonValue &json);
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
    void ParseAddGroup(const QJsonValue &dataVal);
    void ParseCreateGroup(const QJsonValue &dataVal);

    void ParseGetMyFriend(const QJsonValue &dataVal);
    void ParseGetMyGroups(const QJsonValue &dataVal);

    void ParseRefreshFriend(const QJsonValue &dataVal);
    void ParseRefreshGroups(const QJsonValue &dataVal);

    void ParseFriendMessages(const QByteArray &reply);
    void ParseGroupMessages(const QByteArray &reply);
    void ParseFaceMessages(const QByteArray &reply);

    void ParseSendMsg(const QJsonValue &dataVal);   //发送信息
};

/*
// 文件的tcp线程
class ClientFileSocket : public QObject
{
    Q_OBJECT
public:
    explicit ClientFileSocket(QObject *parent = 0, QTcpSocket *tcpSocket = NULL);
    ~ClientFileSocket();

    void Close();
    bool CheckUserId(const qint32 nId, const qint32 &winId);

    // 文件传输完成
    void FileTransFinished();

    void StartTransferFile(QString fileName);
signals:
    void signalConnected();
    void signalDisConnected();

    void signalRecvFinished(int id, const QJsonValue &json);

private:
    quint64 loadSize;
    quint64 bytesReceived;  //已收到数据的大小
    quint64 fileNameSize;  //文件名的大小信息
    QString fileReadName;   //存放文件名
    QByteArray inBlock;   //数据缓冲区
    quint64 ullRecvTotalBytes;  //数据总大小
    QFile *fileToRecv;  //要发送的文件

    QTcpSocket *m_tcpSocket;

    //接收文件
    quint16 blockSize;  //存放接收到的信息大小
    QFile *fileToSend;  //要发送的文件
    quint64 ullSendTotalBytes;  //数据总大小
    quint64 bytesWritten;  //已经发送数据大小
    quint64 bytesToWrite;   //剩余数据大小
    QByteArray outBlock;  //数据缓冲区，即存放每次要发送的数据


    bool m_bBusy;

    // 需要转发的用户id
    qint32 m_nUserId;
    // 当前用户的窗口好友的id
    qint32 m_nWindowId;
private:
    void InitSocket();

public slots:

private slots:
    void displayError(QAbstractSocket::SocketError); // 显示错误
    // 文件接收
    void SltReadyRead();
    // 发送
    void SltUpdateClientProgress(qint64 numBytes);
};
*/
#endif // CLIENTSOCKET_H
