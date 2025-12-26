#ifndef DATABASEMAG_H
#define DATABASEMAG_H
#include <QMutex>
#include <QObject>
#include <QSqlDatabase>
#include "comapi/unit.h"

class DataBaseMag : public QObject
{
    Q_OBJECT
public:
    explicit DataBaseMag(QObject *parent = nullptr);
    ~DataBaseMag();

    static DataBaseMag* getInstance()
    {
        static QMutex mutex;
        if(self == nullptr){
            QMutexLocker locker(&mutex);
            if(!self){
                self = new DataBaseMag();
            }
        }
        return self;
    }

public:
    bool openDatabase(const QString &dataName);
    void closeDatabase();
    void createDatabase();
    void changeAllUserStatus(); //更新所有用户的状态

    // 注册用户
    E_STATUS userRegister(const QString &name, const QString &passwd);
    QJsonObject userLogin(const QString &name,const QString &passwd);
    bool isFriend(const int &userId,const QString &friendName);
    QString getUserHead(const int &userId);

    bool haveUser(const QString &name);   //查询用户是否存在

    void UpdateUserStatus(int id, E_STATUS status);
    QList<QVariantMap> getAllUser();
    int getIdFromUsername(const QString &userName); //通过好友用户名获取id
    QString getUsernameFromId(const QString &userId);   //通过好友Id获取用户名

    QString jsonValueToString(const QJsonValue &jsonVal);
    void insertMessageQueue(const int &senderId, const int &acceptId, quint8 type, const QJsonValue &dataVal);
    QList<QVariantMap> getUserMessageQueue(const int &userId);
    void addFriend(const int &userId, const int &friendId);
    void deleteFriend(const int &userId, const int &friendId);
    bool isOnline(const int &userId);
    QJsonValue getMyFriends(const int &userId);
    QJsonValue getMyGroup(const int &userId);
    int addGroup(const int &userId, const QString groupName);
    bool addGroupMember(const int &userId, const int &groupId, GroupIdentity identity = Member);
    int getGroupOwner(const int &groupId);

    QHash<int, QSet<int>> initGroupMembersCache();

signals:

public slots:

private:
    static DataBaseMag* self;
    QSqlDatabase userdb;

    void queryAll();
};
#endif // DATABASEMAG_H
