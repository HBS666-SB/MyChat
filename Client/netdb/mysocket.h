#ifndef MYSOCKET_H
#define MYSOCKET_H

#include <QTcpSocket>

class MySocket : public QObject
{
    Q_OBJECT
public:
    explicit MySocket(QObject *parent = nullptr);
    ~MySocket();
    void connectToHost(const QString &ip, quint16 port);
    void connectToHost(const QHostAddress &host, quint16 port); // 与服务器建立Tcp连接
    void sendMessage(const qint8 &type, const QJsonValue &dataVal);

signals:
    void signalStatus(const quint8 &status);

private slots:
    void sltReadyRead();
    void sltConnect();
    void sltDisconnect();

private:
    QTcpSocket *m_tcpSocket;
};


#endif // MYSOCKET_H
