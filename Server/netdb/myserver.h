#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QList>
#include <QVector>

#include "clientsocket.h"

// 服务器管理类
class MyServer : public QObject {
    Q_OBJECT
public:
    explicit MyServer(QObject *parent = nullptr);
    ~MyServer();

    bool StartListen(int port = 6666);
    void CloseListen();
signals:
    void signalUserStatus(const QString &text);
protected:
    QTcpServer *m_tcpServer;


public slots:

protected slots:
    // 继承虚函数
    virtual void SltNewConnection() = 0;
    virtual void SltConnected() = 0;
    virtual void SltDisConnected(ClientSocket *client) = 0;
    virtual void SltLoginSuccess(ClientSocket *client, const QString &userId) = 0;
};

// 消息服务器
class TcpMsgServer : public MyServer
{
    Q_OBJECT
public:
    explicit TcpMsgServer(QObject *parent = 0);
    ~TcpMsgServer();

    void insertMessageQueue(const int &send,const int &getId, const QJsonValue &jsonVal, const quint8 &type);
    void sendUserMessageQueue(const QString &userId);   //用户登录成功后服务器检索消息队列转发属于他的消息

signals:
    void signalDownloadFile(const QJsonValue &json);

private:
    // 客户端管理
    QVector < ClientSocket * > m_clients;   //广播快
    QHash<QString, ClientSocket*> m_clientHash;  //单播快
public slots:
    void SltTransFileToClient(const int &userId, const QJsonValue &jsonVal);

private slots:
    void SltNewConnection();
    void SltConnected();
    void SltDisConnected(ClientSocket *client);
//    void SltPublicMsgToClient(const quint8 &type, const int &id, const QJsonValue &json); //广播
    void SltPrivateMsgToClient(const int &id, const int &targetId, const quint8 &type, const QJsonValue &json);    //单播

    void SltLoginSuccess(ClientSocket *client, const QString &userId);
};

/*
// 文件服务器
class TcpFileServer : public MyServer {
    Q_OBJECT
public :
    explicit TcpFileServer(QObject *parent = 0);
    ~TcpFileServer();
signals:
    void signalRecvFinished(int id, const QJsonValue &json);
private:
    // 客户端管理
    QVector < ClientFileSocket * > m_clients;

private slots:
    void SltNewConnection();
    void SltConnected();
    void SltDisConnected();
    void SltClientDownloadFile(const QJsonValue &json);
};
*/
#endif // TCPSERVER_H
