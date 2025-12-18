#include "filesocket.h"
#include <QDataStream>
#include <QFileInfo>
#include <QIODevice>
#include <QThread>
#include <comapi/myapp.h>

FileSocket::FileSocket(QObject *parent) : QObject(parent)
{
    // 延迟在工作线程中创建 Socket，避免在主线程创建后 moveToThread 导致线程亲和性不一致。
    m_tcpSocket = nullptr;
    // 创建文件对象，文件对象可以有 this 作为父对象（与 QObject 一起移动）
    m_fileToSend = new QFile(this);
}

FileSocket::~FileSocket()
{
    stopSendFile();
    if (m_tcpSocket)
    {
        m_tcpSocket->disconnectFromHost();
        m_tcpSocket->deleteLater();
    }
}

// 连接文件服务器
void FileSocket::connectToFileServer(const QString &host, quint16 port)
{
    if (m_isSending)
    {
        emit sendFailed("正在发送文件，无法连接服务器");
        return;
    }

    // 仅在未连接/断开状态下发起连接
    if (!m_tcpSocket)
    {
        // 确保该槽在工作线程中被调用（由 ChatWindow 发出的信号触发），在此线程创建 socket
        m_tcpSocket = new QTcpSocket(this);
        connect(m_tcpSocket, &QTcpSocket::bytesWritten, this, &FileSocket::onBytesWritten);
        connect(m_tcpSocket, &QTcpSocket::connected, this, &FileSocket::onSocketConnected);
        connect(m_tcpSocket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::error),
                this, &FileSocket::onSocketError);
    }

    if (m_tcpSocket->state() == QAbstractSocket::UnconnectedState)
    {
        m_tcpSocket->connectToHost(host, port);
        qDebug() << "子线程发起连接：" << QThread::currentThreadId() << host << port;
    }
    else if (m_tcpSocket->state() == QAbstractSocket::ConnectingState)
    {
        emit sendFailed("正在连接文件服务器，请等待");
    }
    else
    {
        emit fileServerConnected(); // 已连接，直接触发信号
    }
}

void FileSocket::setFilePath(const QString &filePath)
{
    m_filePath = filePath;
}

void FileSocket::setLoadSize(quint64 loadSize)
{
    m_loadSize = loadSize;
}

// Socket连接成功槽函数
void FileSocket::onSocketConnected()
{
    qDebug() << "文件服务器连接成功（子线程）：" << QThread::currentThreadId();
    emit fileServerConnected();
}

// Socket错误处理
void FileSocket::onSocketError(QAbstractSocket::SocketError err)
{
    QString errMsg = QString("Socket错误：%1").arg(m_tcpSocket->errorString());
    qDebug() << errMsg;
    emit sendFailed(errMsg);
}

void FileSocket::startSendFile()
{
    // 状态检查
    if (m_isSending)
    {
        emit sendFailed("正在发送文件，拒绝重复启动");
        return;
    }
    if (!m_tcpSocket || m_tcpSocket->state() != QAbstractSocket::ConnectedState)
    {
        emit sendFailed("未连接文件服务器，无法发送");
        return;
    }
    if (m_filePath.isEmpty())
    {
        emit sendFailed("文件路径为空");
        return;
    }

    // 重置发送状态
    resetSendState();
    m_isSending = true;
    m_isStopped = false;

    // 3. 打开文件
    m_fileToSend->setFileName(m_filePath);
    if (!m_fileToSend->open(QFile::ReadOnly))
    {
        emit sendFailed(QString("文件打开失败：%1").arg(m_fileToSend->errorString()));
        m_isSending = false;
        return;
    }

    // 构造包头
    m_totalBytes = m_fileToSend->size();
    m_outBlock.clear();
    QDataStream sendOut(&m_outBlock, QIODevice::WriteOnly);
    sendOut.setVersion(QDataStream::Qt_5_12);
    QFileInfo fileInfo(m_filePath);
    QString curFileName = fileInfo.fileName();

    // 占位符：总大小 + 文件名长度 + 文件名
    sendOut << qint64(0) << qint64(0) << curFileName;
    m_totalBytes += m_outBlock.size();
    sendOut.device()->seek(0);
    sendOut << m_totalBytes << qint64(m_outBlock.size() - sizeof(qint64) * 2);

    // 初始化发送变量
    m_bytesWritten = 0;
    m_bytesToWrite = m_totalBytes - m_bytesWritten; // 初始化待发送字节数

    // 发送包头
    qint64 sendBytes = m_tcpSocket->write(m_outBlock);
    if (sendBytes == -1)
    {
        emit sendFailed("包头发送失败");
        m_fileToSend->close();
        m_isSending = false;
        return;
    }
    m_bytesWritten += sendBytes;
    m_bytesToWrite -= sendBytes; // 更新待发送字节数
    m_outBlock.clear();

    emit progressUpdated(m_bytesWritten, m_totalBytes);
    qDebug() << "开始发送文件，总字节数：" << m_totalBytes << "已发包头：" << sendBytes;
}

void FileSocket::stopSendFile()
{
    m_isStopped = true;
    m_isSending = false;
    if (m_fileToSend && m_fileToSend->isOpen())
    {
        m_fileToSend->close();
    }
    m_outBlock.clear();
}

void FileSocket::setUserId(const QString &userId)
{
    m_userId = userId;
}

void FileSocket::onBytesWritten(qint64 numBytes)
{
    if (m_isStopped || !m_isSending)
        return;

    //  更新已发送字节数
    m_bytesWritten += numBytes;
    emit progressUpdated(m_bytesWritten, m_totalBytes);

    // 发送剩余文件数据
    if (m_bytesToWrite > 0)
    {
        // 读取分片数据
        m_outBlock = m_fileToSend->read(qMin(m_bytesToWrite, m_loadSize));
        qint64 writeBytes = m_tcpSocket->write(m_outBlock);
        if (writeBytes < 0)
        {
            emit sendFailed("文件数据发送失败");
            stopSendFile();
            return;
        }
        m_bytesToWrite -= writeBytes;
        m_outBlock.clear();
    }
    // 发送完成判断
    else if (m_bytesWritten >= m_totalBytes)
    {
        m_fileToSend->close();
        emit sendFinished();
        m_isSending = false;
        resetSendState();
    }
}

void FileSocket::resetSendState()
{
    m_totalBytes = 0;
    m_bytesWritten = 0;
    m_bytesToWrite = 0;
    m_outBlock.clear();
    m_isSending = false;
    m_isStopped = false;
}
