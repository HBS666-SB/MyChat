#include "filesocket.h"
#include "myserver.h"
#include <QHostAddress>
#include <QHostInfo>
#include <QJsonValue>
#include <QJsonObject>
#include <QDir>
#include "databasemag.h"
#include "comapi/unit.h"
#include <comapi/myapp.h>

MyServer::MyServer(QObject *parent)
{
    m_tcpServer = new QTcpServer(this);

    connect(m_tcpServer, SIGNAL(newConnection()), this, SLOT(SltNewConnection()));
}

MyServer::~MyServer()
{
    if (m_tcpServer->isListening())
        m_tcpServer->close();
}

bool MyServer::StartListen(int port)
{
    m_tcpServer->listen(QHostAddress::Any, static_cast<quint16>(port));
    qDebug() << "开启监听" << port;
    return true;
}

void MyServer::CloseListen()
{
    m_tcpServer->close();
}

// 消息服务器
TcpMsgServer::TcpMsgServer(QObject *parent)
{
    m_groupMembersCache = DataBaseMag::getInstance()->initGroupMembersCache();
}

TcpMsgServer::~TcpMsgServer()
{
}

void TcpMsgServer::insertMessageQueue(const int &send, const int &getId, const QJsonValue &jsonVal, const quint8 &type)
{
    if (!jsonVal.isObject())
    {
        qDebug() << "server插入消息队列逻辑有误";
        return;
    }
    QJsonObject jsonObj = jsonVal.toObject();
    DataBaseMag::getInstance()->insertMessageQueue(send, getId, type, jsonObj);
}

void TcpMsgServer::sendUserMessageQueue(const QString &userId) // 上线的Id
{
    if (userId.isEmpty())
    {
        qDebug() << "转发上线消息：用户Id不能为空";
        return;
    }
    QList<QVariantMap> msgList = DataBaseMag::getInstance()->getUserMessageQueue(userId.toInt());
    foreach (QVariantMap msg, msgList)
    {
        quint8 type = static_cast<quint8>(msg["message_type"].toInt());
        int id = msg["request_userId"].toInt();
        int targetId = msg["accept_userId"].toInt();
        QString friendName = DataBaseMag::getInstance()->getUsernameFromId(QString::number(targetId));
        QJsonObject jsonObj = msg["data"].toJsonObject();
        if(type != 66) {
            jsonObj.insert("type", type);
        }
        jsonObj.insert("name", friendName);

        qDebug() << "myServer:80 转发离线队列消息给" << friendName;
        QJsonValue sendVal = jsonObj;
        SltPrivateMsgToClient(id, targetId, type, sendVal);
    }
}

void TcpMsgServer::SltAddGroupMembers(const int &groupId, const int &userId)
{
    if(groupId < 0 || userId < 0) return;
    m_groupMembersCache[groupId].insert(userId);
}

void TcpMsgServer::SltRemoveGroupMembers(const int &groupId, const int &userId)
{
    if(groupId < 0 || userId < 0) return;
    m_groupMembersCache[groupId].remove(userId);
}

void TcpMsgServer::SltTransFileToClient(const int &userId, const QJsonValue &jsonVal)
{
}

void TcpMsgServer::SltNewConnection()
{
    ClientSocket *client = new ClientSocket(this, m_tcpServer->nextPendingConnection());
    m_clients.append(client); // 加入客户端列表管理

    // 信号槽绑定
    connect(client, &ClientSocket::signalConnected, this, &TcpMsgServer::SltConnected);
    connect(client, &ClientSocket::signalDisConnected, this, &TcpMsgServer::SltDisConnected);
    connect(client, &ClientSocket::signalLoginSuccess, this, &TcpMsgServer::SltLoginSuccess);
    connect(client, &ClientSocket::signalPrivateMsgToClient, this, &TcpMsgServer::SltPrivateMsgToClient);
    connect(client, &ClientSocket::signalGroupMsgToClient, this, &TcpMsgServer::SltGroupMsgToClient);
    connect(client, &ClientSocket::signalAddGroupMembers, this, &TcpMsgServer::SltAddGroupMembers);
    connect(client, &ClientSocket::signalRemoveGroupMembers, this, &TcpMsgServer::SltRemoveGroupMembers);

    qDebug() << "[消息服务器] 新客户端连接！socketDescriptor：";
}

void TcpMsgServer::SltConnected()
{
}

void TcpMsgServer::SltDisConnected()
{
    ClientSocket *client = (ClientSocket *)this->sender();
    m_clients.removeOne(client);
    m_clientHash.remove(QString::number(client->GetUserId()));
    disconnect(client, &ClientSocket::signalConnected, this, &TcpMsgServer::SltConnected);
    disconnect(client, &ClientSocket::signalDisConnected, this, &TcpMsgServer::SltDisConnected);
    disconnect(client, &ClientSocket::signalLoginSuccess, this, &TcpMsgServer::SltLoginSuccess);
    disconnect(client, &ClientSocket::signalPrivateMsgToClient, this, &TcpMsgServer::SltPrivateMsgToClient);
    disconnect(client, &ClientSocket::signalGroupMsgToClient, this, &TcpMsgServer::SltGroupMsgToClient);
    disconnect(client, &ClientSocket::signalAddGroupMembers, this, &TcpMsgServer::SltAddGroupMembers);
    disconnect(client, &ClientSocket::signalRemoveGroupMembers, this, &TcpMsgServer::SltRemoveGroupMembers);
    client->deleteLater();
}

void TcpMsgServer::SltPublicMsgToClient(const int &id, const quint8 &type, const QJsonValue &json)
{

}

void TcpMsgServer::SltPrivateMsgToClient(const int &id, const int &targetId, const quint8 &type, const QJsonValue &jsonVal)
{
    if (targetId < 0)
    {
        qDebug() << "单播目标Id为空";
        return;
    }
    ClientSocket *targetClient = m_clientHash.value(QString::number(targetId), nullptr);
    if (!targetClient)
    {
        qDebug() << "[单播] 目标用户不存在或已下线：" << targetId;
        insertMessageQueue(id, targetId, jsonVal, type);
        return;
    }

    // 转发
    targetClient->SltSendMessage(type, QJsonValue(jsonVal));
    //    qDebug() << "server.cpp转发出去135" << jsonVal;
}

void TcpMsgServer::SltGroupMsgToClient(const int &id, const int &groupId, const quint8 &type, const QJsonValue &json)
{
    if(groupId < 0) {
        qDebug() << "组播目标出现错误";
        return;
    }
    QSet<int> groupSet = m_groupMembersCache[groupId];
    QList<int> memberList = groupSet.toList();
    for (int i = 0; i < memberList.size(); i += BATCH_SIZE) {
        // 截取当前批次的成员ID
        QList<int> batch = memberList.mid(i, BATCH_SIZE);
        // 异步发送，不阻塞主线程
        QMetaObject::invokeMethod(this, [=]() {
            int batchSendCount = 0;
            int batchOfflineCount = 0;
            for (int userId : batch) {
                if (userId == id) continue;
                ClientSocket *client = m_clientHash.value(QString::number(userId), nullptr);
                if (client) {
                    SltPrivateMsgToClient(id, userId, type, json);
                    batchSendCount++;
                } else {
                    insertMessageQueue(id, userId, json, type);
                    batchOfflineCount++;
                }
            }
            qDebug() << QString("[群聊分批] 群组ID=%1 批次%2 成功=%3 离线=%4")
                        .arg(groupId).arg(i/BATCH_SIZE + 1).arg(batchSendCount).arg(batchOfflineCount);
        }, Qt::QueuedConnection);
    }

}

void TcpMsgServer::SltLoginSuccess(ClientSocket *client, const QString &userId)
{
    if (!client || userId.isEmpty())
    {
        qDebug() << "客户端登录ID为空，拒绝绑定";
        client->deleteLater();
        return;
    }

    if (m_clientHash.contains(userId))
    {
        qDebug() << "不能重复登录";
        return;
    }
    m_clientHash.insert(userId, client);
    sendUserMessageQueue(userId);
    qDebug() << "客户端登录成功，ID：" << userId << "，当前在线数：" << m_clientHash.size();
}

TcpFileServer::TcpFileServer(QObject *parent)
{
    m_threadPool = new QThreadPool(this);
    m_threadPool->setMaxThreadCount(10);
    m_threadPool->setExpiryTimeout(30000); // 线程空闲30秒回收

    m_fileSaveDir = MyApp::m_strRecvPath;
    QDir saveDir(m_fileSaveDir);
    if (!saveDir.exists())
    {
        saveDir.mkpath(".");
        qDebug() << "[TcpFileServer] 创建文件保存目录：" << m_fileSaveDir;
    }
}

TcpFileServer::~TcpFileServer()
{
    stop();
    // 清理线程池
    m_threadPool->clear();
    m_threadPool->waitForDone(5000);
}

bool TcpFileServer::StartListen(quint16 port)
{
    if (m_isRunning)
    {
        qWarning() << "[TcpFileServer] 服务器已运行";
        return true;
    }
    if (!this->listen(QHostAddress::Any, port))
    {
        qCritical() << "[TcpFileServer] 启动失败：" << this->errorString();
        return false;
    }

    m_isRunning = true;
    qInfo() << "[TcpFileServer] 启动成功，监听端口：" << port
            << "线程池最大线程数：" << m_threadPool->maxThreadCount();
    return true;
}

void TcpFileServer::stop()
{
    if (!m_isRunning)
        return;

    // 停止监听新连接
    this->close();
    m_isRunning = false;

    // 停止所有客户端任务
    QMutexLocker locker(&m_clientMutex);
    for (FileSocket *socket : m_clientMap.values())
    {
        socket->stopRecvFile(); // 停止文件接收
    }
    m_clientMap.clear();
    locker.unlock();

    qInfo() << "[TcpFileServer] 已停止，等待线程池任务完成...";
    m_threadPool->waitForDone(10000); // 等待10秒
    qInfo() << "[TcpFileServer] 完全停止";
}

void TcpFileServer::setFileSaveDir(const QString &dir)
{
    if (!dir.isEmpty())
    {
        m_fileSaveDir = dir;
        QDir saveDir(m_fileSaveDir);
        if (!saveDir.exists())
            saveDir.mkpath(".");
        qInfo() << "[TcpFileServer] 文件保存目录已更新：" << m_fileSaveDir;
    }
}

void TcpFileServer::setMaxThreadCount(int count)
{
    if (count > 0)
    {
        m_threadPool->setMaxThreadCount(count);
        qInfo() << "[TcpFileServer] 线程池最大线程数已更新：" << count;
    }
}

void TcpFileServer::incomingConnection(qintptr socketDescriptor)
{
    if (!m_isRunning)
    {
        qWarning() << "[TcpFileServer] 服务器已停止，拒绝新连接";
        // 关闭未处理的socket
        QTcpSocket tempSocket;
        tempSocket.setSocketDescriptor(socketDescriptor);
        tempSocket.disconnectFromHost();
        return;
    }

    // 创建 FileSocket 任务，并将描述符交给它以便在工作线程中创建 QTcpSocket
    FileSocket *fileSocket = new FileSocket();
    fileSocket->setSocketDescriptor(socketDescriptor);
    // 设置文件保存目录
    fileSocket->setSaveDir(m_fileSaveDir);

    // 使用临时键（socketDescriptor）先加入管理表，FileSocket 在运行时会获取真实 clientKey
    QString clientKey = QString::number(socketDescriptor);
    qInfo() << "[TcpFileServer] 新客户端连接（描述符）：" << clientKey;

    // 绑定FileSocket信号（跨线程传递到主线程）
    connect(fileSocket, &FileSocket::recvFinished, this, &TcpFileServer::onFileRecvFinished, Qt::QueuedConnection);
    connect(fileSocket, &FileSocket::taskFailed, this, &TcpFileServer::onTaskFailed, Qt::QueuedConnection);
    connect(fileSocket, &FileSocket::destroyed, this, [=]()
    {
        // FileSocket销毁时，从客户端列表移除（按指针匹配）
        QMutexLocker locker(&m_clientMutex);
        QString foundKey;
        for (auto it = m_clientMap.begin(); it != m_clientMap.end(); ++it) {
            if (it.value() == fileSocket) {
                foundKey = it.key();
                break;
            }
        }
        if (!foundKey.isEmpty()) {
            m_clientMap.remove(foundKey);
            emit clientDisconnected(foundKey);
            qInfo() << "[TcpFileServer] 客户端任务销毁：" << foundKey;
        } });

    // 加入客户端列表
    QMutexLocker locker(&m_clientMutex);
    m_clientMap.insert(clientKey, fileSocket);
    locker.unlock();

    // 提交到线程池执行
    m_threadPool->start(fileSocket);

    // 触发新连接信号
    emit clientConnected(clientKey);
}

void TcpFileServer::onFileRecvFinished(const QString &clientKey, const QString &filePath)
{
    qInfo() << "[TcpFileServer] 文件接收完成：" << clientKey << "→" << filePath;
    // 转发全局信号
    emit fileRecvFinished(clientKey, filePath);

    // 接收完成后移除客户端
    QMutexLocker locker(&m_clientMutex);
    // 尝试按 key 直接移除，否则按 value->clientKey() 匹配移除
    if (m_clientMap.contains(clientKey))
    {
        m_clientMap.remove(clientKey);
    }
    else
    {
        QString foundKey;
        for (auto it = m_clientMap.begin(); it != m_clientMap.end(); ++it)
        {
            if (it.value() && it.value()->clientKey() == clientKey)
            {
                foundKey = it.key();
                break;
            }
        }
        if (!foundKey.isEmpty())
            m_clientMap.remove(foundKey);
    }
}

void TcpFileServer::onTaskFailed(const QString &clientKey, const QString &error)
{
    qWarning() << "[TcpFileServer] 任务失败：" << clientKey << "→" << error;
    // 转发全局信号
    emit taskFailed(clientKey, error);

    // 任务失败，移除客户端
    QMutexLocker locker(&m_clientMutex);
    if (m_clientMap.contains(clientKey))
    {
        m_clientMap.remove(clientKey);
    }
    else
    {
        QString foundKey;
        for (auto it = m_clientMap.begin(); it != m_clientMap.end(); ++it)
        {
            if (it.value() && it.value()->clientKey() == clientKey)
            {
                foundKey = it.key();
                break;
            }
        }
        if (!foundKey.isEmpty())
            m_clientMap.remove(foundKey);
    }
}

void TcpFileServer::onClientDisconnected(const QString &clientKey)
{
    qInfo() << "[TcpFileServer] 客户端断开连接：" << clientKey;
    // 转发全局信号
    emit clientDisconnected(clientKey);
}
