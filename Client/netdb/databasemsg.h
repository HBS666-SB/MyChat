#ifndef DATABASEMSG_H
#define DATABASEMSG_H

#include <QMutex>
#include <QObject>
#include <QSqlDatabase>
#include <QVector>

class DatabaseMsg : public QObject
{
    Q_OBJECT
public:
    explicit DatabaseMsg(QObject *parent = nullptr);
    ~DatabaseMsg();

    // 单实例运行
    static DatabaseMsg *getInstance()
    {
        static QMutex mutex;
        if (nullptr == self) {
            QMutexLocker locker(&mutex);

            if (!self) {
                self = new DatabaseMsg();
            }
        }

        return self;
    }

    bool OpenUserDatabase(const QString &dataName);
    bool OpenMessageDatabase(const QString &dataName);
    // 好友操作
    void AddFriend(const int &id, const int &userId, const QString &name);
    QJsonArray GetMyFriend(const int &userId) const;

    // 消息操作
    void AddHistoryMsg(const int &userId, const QString &name,
                       const QString &text, const QString &time);
    QVector<QJsonObject> QueryHistory(const int &id, const int &count = 0);
    bool isMyFriend(int,QString);

private:
    static DatabaseMsg *self;
    QSqlDatabase userdb;
    QSqlDatabase msgdb;

signals:

public slots:
};



#endif // DATABASEMSG_H
