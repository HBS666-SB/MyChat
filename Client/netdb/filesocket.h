#ifndef FILESOCKET_H
#define FILESOCKET_H

#include <QObject>
#include <QTcpSocket>
#include <QFile>
#include <QByteArray>
#include <QString>
class FileSocket : public QObject
{
    Q_OBJECT
public:
    explicit FileSocket(QObject *parent = nullptr);
    ~FileSocket() override;

    // 连接文件服务器
    void connectToFileServer(const QString &host, quint16 port);
    void setFilePath(const QString &filePath);
    void setLoadSize(quint64 loadSize = 4*1024); // 默认4KB分片
    void startSendFile();
    void stopSendFile();
    void setUserId(const QString &userId);

signals:
    void progressUpdated(quint64 sent, quint64 total);
    void sendFinished();
    void sendFailed(const QString &error);
    void fileServerConnected(); // 连接成功信号

private slots:
    void onBytesWritten(qint64 numBytes);
    void onSocketConnected();   // Socket连接成功槽函数
    void onSocketError(QAbstractSocket::SocketError err); // Socket错误处理

private:
    void resetSendState();

    QTcpSocket *m_tcpSocket = nullptr;
    QFile *m_fileToSend = nullptr;
    QString m_filePath;
    QByteArray m_outBlock;

    quint64 m_totalBytes = 0;
    quint64 m_bytesWritten = 0;
    quint64 m_bytesToWrite = 0;
    quint64 m_loadSize = 4*1024; // 分片大小（4KB）

    bool m_isSending = false;
    bool m_isStopped = false;
    QString m_userId;
};

#endif // FILESENDERWORKER_H
