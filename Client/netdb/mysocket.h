#ifndef MYSOCKET_H
#define MYSOCKET_H

#include <QTcpSocket>

class MySocket : public QObject
{
    Q_OBJECT
public:
    explicit MySocket(QObject *parent = nullptr);
    ~MySocket();
    void CheckConnected();
    void ColseConnected();

    void connectToHost(const QString &ip, quint16 port);
    void connectToHost(const QHostAddress &host, quint16 port); // 与服务器建立Tcp连接
    void sendMessage(const qint8 &type, const QJsonValue &dataVal);

    void ParseLogin(const QJsonValue &dataVal);

    QJsonValue GetUserId();

signals:
    void signalStatus(const quint8 &status);
    void signalConnectSuccess();

private slots:
    void sltReadyRead();
    void sltConnect();
    void sltDisconnect();


private:
    QTcpSocket *m_tcpSocket;
    int m_nId;
};


#endif // MYSOCKET_H
