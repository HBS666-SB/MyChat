#include "databasemsg.h"

#include <QJsonArray>
#include <QVector>
#include <QDebug>
#include <QSqlQuery>

DatabaseMsg *DatabaseMsg::self = nullptr;
DatabaseMsg::DatabaseMsg(QObject *parent) : QObject(parent)
{

}

DatabaseMsg::~DatabaseMsg()
{
    if (userdb.isOpen()) {
        userdb.close();
    }

    if (msgdb.isOpen()) {
        msgdb.close();
    }
}

bool DatabaseMsg::OpenUserDatabase(const QString &dataName)
{
    userdb = QSqlDatabase::addDatabase("QSQLITE", "connectionUser");
    userdb.setDatabaseName(dataName);
    if (!userdb.open()) {
        qDebug() << "打开用户数据库失败";
        return false;
    }
    QSqlQuery query(userdb);

    query.exec("CREATE TABLE FRIEND (id INT, userId INT, name varchar(50))");   // 好友表 id为好友id，userid为当前用户id
    query.exec("CREATE TABLE MYGROUP (id INT, userId INT, name varchar(50))");// 群组表 id为群组id，userid为当前用户id
    query.exec("CREATE TABLE USERINFO (id INT, name varchar(50), passwd varchar(50))"); // 用户数据保存
    return true;
}

bool DatabaseMsg::OpenMessageDatabase(const QString &dataName)
{
    msgdb = QSqlDatabase::addDatabase("QSQLITE","connectionMsg");
    msgdb.setDatabaseName(dataName);
    if(!msgdb.open()){
        qDebug() << "打开消息数据库失败";
        return false;
    }
    QSqlQuery query(msgdb);
    query.exec("CREATE TABLE MSGINFO (id INT PRIMARY KEY, userId INT, name varchar(20),"
               "head varchar(50), datetime varchar(20), filesize varchar(30),"
               "content varchar(500), type INT, direction INT)");

    return true;
}

void DatabaseMsg::AddFriend(const int &id, const int &userId, const QString &name)
{

}

QJsonArray DatabaseMsg::GetMyFriend(const int &userId) const
{

}

void DatabaseMsg::AddHistoryMsg(const int &userId, const QString &name, const QString &text, const QString &time)
{

}

QVector<QJsonObject> DatabaseMsg::QueryHistory(const int &id, const int &count)
{

}

bool DatabaseMsg::isMyFriend(int, QString)
{

}
