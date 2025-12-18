#ifndef FILESOCKET_H
#define FILESOCKET_H

#include <QObject>
#include <QRunnable>
#include <QTcpSocket>
#include <QFile>
#include <QDataStream>
#include <QMutex>
#include <QEventLoop>

class FileSocket : public QObject, public QRunnable
{
    Q_OBJECT
public:
    explicit FileSocket(QTcpSocket *socket = nullptr, QObject *parent = nullptr);
    ~FileSocket() override;

    void run() override; // 线程池执行入口

    // 设置客户端Socket
    void setTcpSocket(QTcpSocket *socket);
    // 设置文件保存目录
    void setSaveDir(const QString &dir = "");
    // 获取客户端标识
    QString clientKey() const { return m_clientKey; }

signals:
    // 接收文件完成   客户端标识、文件保存路径
    void recvFinished(const QString &clientKey, const QString &filePath);
    // 任务失败             客户端标识、错误信息
    void taskFailed(const QString &clientKey, const QString &error);

private slots:
    void onReadyRead();                                   // 读取客户端数据
    void onSocketDisconnected();                          // Socket断开连接
    void onSocketError(QAbstractSocket::SocketError err); // Socket错误

public:
    void startRecvFile();
    void stopRecvFile();

    QTcpSocket *m_tcpSocket = nullptr; // 客户端通信Socket
    QFile *m_fileHandler = nullptr;    // 接收文件操作对象
    QString m_clientKey;               // 客户端标识
    QString m_saveDir;                 // 文件保存目录
    QString m_finalRecvPath;           // 接收文件的最终保存路径

    QByteArray m_dataBuffer;      // 数据缓冲区
    quint64 m_totalBytes = 0;     // 文件总字节数
    quint64 m_processedBytes = 0; // 已接收字节数

    bool m_isRunning = false;          // 任务是否运行中
    QMutex m_mutex;                    // 线程安全锁
    QEventLoop *m_eventLoop = nullptr; // 线程内事件循环
    qintptr m_socketDescriptor = -1;   // 延迟创建 socket 时保存的描述符

public:
    // 延迟创建 socket：在主线程接收到描述符后调用，实际在 run() 中使用
    void setSocketDescriptor(qintptr sd) { m_socketDescriptor = sd; }
};

#endif // FILESOCKET_H
