#include "databasemsg.h"
#include "comapi/unit.h"
#include <QJsonArray>
#include <QVector>
#include <QDebug>
#include <QSqlQuery>
#include <QSqlError>

DatabaseMsg *DatabaseMsg::self = nullptr;
DatabaseMsg::DatabaseMsg(QObject *parent) : QObject(parent)
{
}

DatabaseMsg::~DatabaseMsg()
{
    if (userdb.isOpen())
    {
        userdb.close();
    }

    if (msgdb.isOpen())
    {
        msgdb.close();
    }
}

bool DatabaseMsg::OpenUserDatabase(const QString &dataName)
{
    userdb = QSqlDatabase::addDatabase("QSQLITE", "connectionUser");
    userdb.setDatabaseName(dataName);
    if (!userdb.open())
    {
        qDebug() << "打开用户数据库失败";
        return false;
    }
    QSqlQuery query(userdb);
    QString sql;

    sql = QString("CREATE TABLE IF NOT EXISTS USERINFO (");
    sql.append(QString("id INTEGER PRIMARY KEY AUTOINCREMENT,"));
    sql.append(QString("name VARCHAR(30) NOT NULL UNIQUE,"));
    sql.append(QString("passwd VARCHAR(30) NOT NULL,"));
    sql.append(QString("head VARCHAR(100) DEFAULT 'default.png',"));
    sql.append(QString("status INTEGER DEFAULT %1,").arg(OffLine));
    sql.append(QString("lasttime DATETIME DEFAULT NULL);"));
    query.exec(sql);

    sql = QString("CREATE TABLE IF NOT EXISTS GROUPS (");
    sql.append(QString("group_id INTEGER PRIMARY KEY AUTOINCREMENT,"));
    sql.append(QString("group_name VARCHAR(50) NOT NULL,"));
    sql.append(QString("group_head VARCHAR(100) DEFAULT 'default_group.png',"));
    sql.append(QString("create_time DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,"));
    sql.append(QString("creator_id INTEGER NOT NULL);"));
    query.exec(sql);

    sql = QString("CREATE TABLE IF NOT EXISTS GROUP_MEMBER (");
    sql.append(QString("id INTEGER PRIMARY KEY AUTOINCREMENT,"));
    sql.append(QString("group_id INTEGER NOT NULL,"));
    sql.append(QString("user_id INTEGER NOT NULL,"));
    sql.append(QString("identity INTEGER NOT NULL DEFAULT 0,"));
    sql.append(QString("join_time DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,"));
    sql.append(QString("UNIQUE (group_id, user_id));"));
    query.exec(sql);

    sql = QString("CREATE TABLE IF NOT EXISTS FRIEND (");
    sql.append(QString("id INTEGER PRIMARY KEY AUTOINCREMENT,"));
    sql.append(QString("user_id INTEGER NOT NULL,"));          // 好友的用户ID
    sql.append(QString("friend_name VARCHAR(30) NOT NULL,"));  // 好友用户名
    sql.append(QString("remark VARCHAR(30) DEFAULT '',"));     // 好友备注
    sql.append(QString("status INTEGER NOT NULL DEFAULT 0,")); // 0=待验证/1=已通过/2=已拉黑
    sql.append(QString("add_time DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,"));
    sql.append(QString("update_time DATETIME DEFAULT CURRENT_TIMESTAMP,"));
    sql.append(QString("UNIQUE (user_id, friend_name));")); // 避免重复添加好友
    query.exec(sql);
    return true;
}

bool DatabaseMsg::OpenMessageDatabase(const QString &dataName)
{
    // 避免重复连接
    if (QSqlDatabase::contains("connectionMsg"))
    {
        msgdb = QSqlDatabase::database("connectionMsg");
        if (msgdb.isOpen())
        {
            return true;
        }
    }

    msgdb = QSqlDatabase::addDatabase("QSQLITE", "connectionMsg");
    msgdb.setDatabaseName(dataName);

    if (!msgdb.open())
    {
        qDebug() << "打开消息数据库失败";
        return false;
    }

    QSqlQuery query(msgdb);
    QString sql;
    sql = QString("CREATE TABLE IF NOT EXISTS MSGINFO (");
    sql.append(QString("id INTEGER PRIMARY KEY AUTOINCREMENT,"));
    sql.append(QString("user_id INTEGER NOT NULL,")); // 好友的Id
    sql.append(QString("name VARCHAR(20) NOT NULL,"));
    sql.append(QString("head VARCHAR(100) DEFAULT 'default.png',"));
    sql.append(QString("datetime DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,"));
    sql.append(QString("filesize VARCHAR(30) DEFAULT '',"));
    sql.append(QString("content VARCHAR(1000) NOT NULL,"));
    sql.append(QString("type INTEGER NOT NULL DEFAULT 0,"));
    sql.append(QString("direction INTEGER NOT NULL DEFAULT 0);"));

    if (!query.exec(sql))
    {
        qDebug() << "创建消息表MSGINFO失败" << query.lastError();
        msgdb.close();
        return false;
    }

    return true;
}

QString DatabaseMsg::getFriendName(const int &id)
{
    QString userName = "";
    if (id < 0)
    {
        qDebug() << "获取好友名失败，id < 0";
        return userName;
    }
    QSqlQuery query(userdb);
    QString sql("SELECT friend_name FROM FRIEND where user_id = :id;");
    query.prepare(sql);
    query.bindValue(":id", id);
    if (!query.exec())
    {
        qDebug() << "获取用户名失败" << query.lastError();
        return userName;
    }
    query.next();
    userName = query.value("friend_name").toString();
    return userName;
}

void DatabaseMsg::AddFriend(const int &userId, const QString &friendName, int status)
{
    bool isFriend = isMyFriend(friendName);
    if (!isFriend)
    { // 不是好友
        QSqlQuery query(userdb);
        QString sql;
        sql = QString("INSERT INTO FRIEND (");
        sql.append("user_id, friend_name, remark, status, add_time, update_time) ");
        sql.append("VALUES (:userId, :friendName, :remark, :status, CURRENT_TIMESTAMP, CURRENT_TIMESTAMP)");

        query.prepare(sql);
        query.bindValue(":userId", userId);
        query.bindValue(":friendName", friendName);
        query.bindValue(":remark", friendName);
        query.bindValue(":status", status);
        if (!query.exec())
        {
            qDebug() << "数据库插入好友失败" << query.lastError();
        }
    }
    // 好友表已经有这条记录，收到同意消息修改status
    QSqlQuery query(userdb);
    QString sql;
    sql = QString("UPDATE FRIEND SET status = :status, update_time = CURRENT_TIMESTAMP");
    sql.append("WHERE user_id = :userId AND friend_name = :friendName;");
    query.prepare(sql);
    query.bindValue(":status", status);
    query.bindValue(":userId", userId);
    query.bindValue(":friendName", friendName);
}

QJsonValue DatabaseMsg::GetMyFriend()
{
    QJsonArray jsonArr;
    QSqlQuery query;
    QString sql;
    sql = QString("SELECT user_id FROM FRIEND;");

    if (!query.exec(sql))
    {
        qDebug() << "查询我的好友Id失败" << query.lastError();
        return QJsonValue();
    }
    while (query.next())
    {
        QJsonObject jsonObj;
        jsonObj.insert("id", query.value("user_id").toInt());
        jsonArr.append(jsonObj);
    }
    QJsonValue jsonVal(jsonArr);
    return jsonVal;
}

bool DatabaseMsg::deleteFriend(const int &userId)
{
    if(userId < 0){
        qDebug() << "删除好友失败，好友Id < 0";
        return false;
    }
    QSqlQuery query(userdb);
    QString sql = QString("DELETE FROM FRIEND WHERE user_id = :userId;");
    query.prepare(sql);
    query.bindValue(":userId", userId);
    if(!query.exec()){
        qDebug() << "删除好友操作错误" << query.lastError();
        return false;
    }
    return true;
}

void DatabaseMsg::AddHistoryMsg(const int &userId, ItemInfo *itemInfo)
{
    if (userId < 0)
    {
        qDebug() << "添加历史消息出错userId < 0";
        return;
    }
    QSqlQuery query(msgdb);
    QString sql;
    sql = QString("INSERT INTO MSGINFO (user_id, name, head, datetime, filesize, content, type, direction) ");
    sql.append("VALUES (:user_id, :name, :head, :datetime, :filesize, :content, :type, :direction);");
    query.prepare(sql);
    query.bindValue(":user_id", userId);
    query.bindValue(":name", itemInfo->GetName());
    query.bindValue(":head", itemInfo->GetStrPixmap());
    query.bindValue(":datetime", itemInfo->GetDatetime());
    query.bindValue(":filesize", itemInfo->GetFileSizeString());
    if (itemInfo->GetMsgType() == Text || itemInfo->GetMsgType() == Files)
    {
        query.bindValue(":content", itemInfo->GetText());
    }
    else if (itemInfo->GetMsgType() == Face)
    {
        query.bindValue(":content", QString::number(itemInfo->getFace()));
    }
    else
    {
        // 保证 content 字段不会为 NULL
        query.bindValue(":content", QString(""));
    }
    query.bindValue(":type", static_cast<int>(itemInfo->GetMsgType()));
    query.bindValue(":direction", itemInfo->GetOrientation());

    if (!query.exec())
    {
        qDebug() << "插入历史消息出错" << query.lastError();
    }
    //    qDebug() << "插入到数据库消息：" << "content" << itemInfo->getFace() << "type" << itemInfo->GetMsgType();
}

QVector<QJsonObject> DatabaseMsg::getHistoryMsg(const int &id, const int &count)
{
    const int maxCount = 10;
    int n = 0;
    qDebug() << "count = " << count;
    QVector<QJsonObject> vJsonObj;
    QSqlQuery query(msgdb);
    QString sql;
    sql = QString("SELECT name, head, datetime, filesize, content, type, direction FROM MSGINFO WHERE user_id = :id");
    sql.append(" ORDER BY id DESC LIMIT 10 OFFSET :count");
    query.prepare(sql);
    query.bindValue(":id", id);
    query.bindValue(":count", count * maxCount);
    if (!query.exec())
    {
        qDebug() << "查询用户历史信息失败" << query.lastError();
        return vJsonObj;
    }
    while (query.next() && n++ < maxCount)
    {
        QJsonObject Obj;
        Obj.insert("name", query.value("name").toString());
        Obj.insert("head", query.value("head").toString());
        Obj.insert("datetime", query.value("datetime").toString());
        Obj.insert("filesize", query.value("filesize").toString());
        Obj.insert("content", query.value("content").toString());
        Obj.insert("type", query.value("type").toInt());
        Obj.insert("direction", query.value("direction").toInt());
        vJsonObj.append(Obj);
        //        qDebug() << Obj.value("content");
    }
    return vJsonObj;
}

bool DatabaseMsg::isMyFriend(QString friendName)
{

    QSqlQuery query(userdb);
    QString sql = "SELECT friend_name FROM FRIEND WHERE friend_name = :friendName LIMIT 1";

    query.prepare(sql);
    query.bindValue(":friendName", friendName);
    query.exec();

    if (!query.next())
    {
        qDebug() << "查询是不是好友失败" << query.lastError();
        return false; // 不是好友
    }
    return true; // 是好友

    //    if (query.next()){
    //        status = query.value("status").toInt();
    //        if(status == 0){
    //            return AddFriendFailed_Readd;   //添加申请还未通过
    //        }else if(status == 1 || status == 2){
    //            return  AddFriendFailed_IsHad;  //已经是好友了
    //        }
    //    }
    //    return AddFriendOk; //不是好友
    //    qDebug() << "status = " << status;
}

void DatabaseMsg::removeFriend(const QString &friendName)
{
    if (friendName.isEmpty())
    {
        qDebug() << "删除好友失败，用户名不能为空";
    }
    QSqlQuery query;
    QString sql;
    sql = QString("DELETE FROM FRIEND WHERE friend_name = :friendId");
    query.prepare(sql);
    query.bindValue(":friendId", friendName);
    if (!query.exec())
    {
        qDebug() << "删除好友操作Sql执行失败";
        return;
    }
}

void DatabaseMsg::AddGroup(const int &userId, const QString &groupName, const int &groupId)
{
    if(userId < 0 || groupName.isEmpty()){
        qDebug() << "数据库插入群组失败";
        return;
    }
    QSqlQuery query(userdb);
    QString sql;
    sql = QString("INSERT INTO GROUPS (group_id, group_name, creator_id) ");
    sql.append("VALUES (:groupId, :groupName, :creatorId);");
    query.prepare(sql);
    query.bindValue(":groupId", groupId);
    query.bindValue(":groupName", groupName);
    query.bindValue(":creatorId", userId);
    if(!query.exec()){
        qDebug() << "添加群组出错" << query.lastError();
        return ;
    }

}
