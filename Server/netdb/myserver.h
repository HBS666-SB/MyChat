#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QList>
#include <QVector>
#include <QThreadPool>
#include <QMutex>

#include "clientsocket.h"
class FileSocket;

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
    virtual void SltDisConnected() = 0;
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
    QHash<int, QSet<int>> m_groupMembersCache;  //群号 -> 群成员
public slots:
    void SltTransFileToClient(const int &userId, const QJsonValue &jsonVal);

private slots:
    void SltNewConnection();
    void SltConnected();
    void SltDisConnected();
    void SltPublicMsgToClient(const int &id, const quint8 &type, const QJsonValue &json); //广播
    //id发给targetId
    void SltPrivateMsgToClient(const int &id, const int &targetId, const quint8 &type, const QJsonValue &json);    //单播
    void SltGroupMsgToClient(const int &id, const int &groupId, const quint8 &type, const QJsonValue &json);    //组播

    void SltLoginSuccess(ClientSocket *client, const QString &userId);
    void SltAddGroupMembers(const int &groupId, const int &userId);
    void SltRemoveGroupMembers(const int &groupId, const int &userId);
};


// 文件服务器
class TcpFileServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit TcpFileServer(QObject *parent = nullptr);
    ~TcpFileServer() override;

    // 启动服务器
    bool StartListen(quint16 port);
    // 停止服务器
    void stop();
    // 设置文件保存根目录
    void setFileSaveDir(const QString &dir);
    // 设置线程池最大线程数
    void setMaxThreadCount(int count);

signals:
    // 新客户端连接
    void clientConnected(const QString &clientKey);
    // 文件接收完成
    void fileRecvFinished(const QString &clientKey, const QString &filePath);
    // 任务失败
    void taskFailed(const QString &clientKey, const QString &error);
    // 客户端断开连接
    void clientDisconnected(const QString &clientKey);

protected:
    // 重写QTcpServer的新连接处理
    void incomingConnection(qintptr socketDescriptor) override;

private slots:
    // 接收FileSocket的信号
    void onFileRecvFinished(const QString &clientKey, const QString &filePath);
    void onTaskFailed(const QString &clientKey, const QString &error);
    void onClientDisconnected(const QString &clientKey);

private:
    QThreadPool *m_threadPool;              // 自定义线程池（替代全局池，更易管理）
    QMutex m_clientMutex;                   // 客户端列表锁
    QHash<QString, FileSocket*> m_clientMap;// 客户端标识
    QString m_fileSaveDir;                  // 文件保存根目录
    bool m_isRunning = false;               // 服务器是否运行中
};

#endif // TCPSERVER_H
