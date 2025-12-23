#ifndef DATABASEMSG_H
#define DATABASEMSG_H

#include <QMutex>
#include <QObject>
#include <QSqlDatabase>
#include <QVector>
#include <basewidget/iteminfo.h>
#include "comapi/unit.h"

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
    QString getFriendName(const int &id);

    // 好友操作
    void AddFriend(const int &userId, const QString &friendName ,int status);
    QJsonValue GetMyFriend();
    bool deleteFriend(const int &userId);

    // 消息操作
    void AddHistoryMsg(const int &userId, ItemInfo *itemInfo);
    QVector<QJsonObject> getHistoryMsg(const int &id, const int &count = 0);
    bool isMyFriend(QString friendName);
    void removeFriend(const QString &friendName);

    //群组操作
    void AddGroup(const int &userId, const QString &groupName);


private:
    static DatabaseMsg *self;
    QSqlDatabase userdb;
    QSqlDatabase msgdb;

signals:

public slots:
};



#endif // DATABASEMSG_H
