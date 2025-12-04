#include "databasemag.h"
#include <QDebug>
#include <QJsonObject>
#include <QSqlQuery>
#include "comapi/myapp.h"
#include <QSqlError>

DataBaseMag *DataBaseMag::self = nullptr;

DataBaseMag::DataBaseMag(QObject *parent) : QObject(parent)
{
}

DataBaseMag::~DataBaseMag()
{
    if (userdb.isOpen()) {
        userdb.close();
    }
}

bool DataBaseMag::openDatabase(const QString &dataName)
{
    if (userdb.isOpen()) {
        return true;
    }
    if (QSqlDatabase::contains("qt_sql_default_connection")) {
        userdb = QSqlDatabase::database("qt_sql_default_connection");
    } else {
        userdb = QSqlDatabase::addDatabase("QSQLITE");
    }
    userdb.setDatabaseName(dataName);
    if (!userdb.open()) {
        // 打印完整错误信息
        qDebug() << "数据库打开失败："
                 << "路径=" << dataName
                 << "错误信息=" << userdb.lastError().text();
        return false;
    }
    QSqlQuery query(userdb);
    query.exec("PRAGMA foreign_keys = ON;");

    createDatabase();

    qDebug() << "数据库打开成功：" << dataName;
    return true;
}

void DataBaseMag::closeDatabase()
{
    qDebug() << userdb;
    userdb.close();
}

void DataBaseMag::createDatabase()
{
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
    sql.append(QString("creator_id INTEGER NOT NULL REFERENCES USERINFO(id) ON DELETE CASCADE);"));
    query.exec(sql);

    sql = QString("CREATE TABLE IF NOT EXISTS GROUP_MEMBER (");
    sql.append(QString("id INTEGER PRIMARY KEY AUTOINCREMENT,"));
    sql.append(QString("group_id INTEGER NOT NULL REFERENCES GROUPS(group_id) ON DELETE CASCADE,"));
    sql.append(QString("user_id INTEGER NOT NULL REFERENCES USERINFO(id) ON DELETE CASCADE,"));
    sql.append(QString("identity INTEGER NOT NULL DEFAULT 0,"));
    sql.append(QString("join_time DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,"));
    sql.append(QString("UNIQUE (group_id, user_id));"));
    query.exec(sql);

    sql = QString("INSERT INTO USERINFO VALUES(1, 'admin', '123456', '2.bmp', %1, '');").arg(OffLine);    //插入数据
    query.exec(sql);
}

void DataBaseMag::changeAllUserStatus()
{
    QSqlQuery query;
    const QString sql = "UPDATE USERINFO SET status = ?";
    query.prepare(sql);
    query.addBindValue(OffLine);
    query.exec();
}

E_STATUS DataBaseMag::userRegister(const QString &name, const QString &passwd)
{
    QSqlQuery query;
    QString sql("INSERT INTO USERINFO (name, passwd) VALUES (?, ?)");
    query.prepare(sql);
    query.addBindValue(name);
    query.addBindValue(passwd);
    return query.exec() ? RegisterOk : RegisterFailed;
}

QJsonObject DataBaseMag::userLogin(const QString &name, const QString &passwd)
{
    QJsonObject jsonObj;
    int nId = -1;
    int code = -1;
    QString strHead = "0.bmp";

    QSqlQuery query;
    const QString sql = "SELECT id, head, status FROM USERINFO WHERE name = ? AND passwd = ?";

    query.prepare(sql);
    query.addBindValue(name);
    query.addBindValue(passwd);

    if (query.exec() && query.next()) {
        nId = query.value("id").toInt();
        int nStatus = query.value("status").toInt();
        if (OnLine == nStatus)
        {
            nId = -2;
            code = -2;
        }
        else
        {
            UpdateUserStatus(nId, OnLine);
            code = 0;
        }
        strHead = query.value("head").toString();
    }

    // 组织返回
    jsonObj.insert("id", nId);
    jsonObj.insert("msg", nId < 0 ? "error" : "ok");
    jsonObj.insert("head", strHead);
    jsonObj.insert("code", code);

    return jsonObj;
}

void DataBaseMag::UpdateUserStatus(int id, E_STATUS status)
{
    if (id <= 0) return;

    QSqlQuery query;
    const QString sql = "UPDATE USERINFO SET status = ?, lasttime = datetime('now') WHERE id = ?";
    query.prepare(sql);
    query.addBindValue(static_cast<int>(status));
    query.addBindValue(id);
    query.exec();
}

QList<QVariantMap> DataBaseMag::getAllUser()
{
    QList<QVariantMap> userList;
    if (!userdb.isOpen()) {
        qDebug() << "查询失败：数据库未打开";
        return userList;
    }

    QSqlQuery query(userdb);
    QString sql = "SELECT * FROM USERINFO;";

    if (query.exec(sql)) {
        while (query.next()) {
            QVariantMap userMap;
            // 按字段名取值，键名和数据库字段一致，便于后续使用
            userMap["id"] = query.value("id").toInt();
            userMap["name"] = query.value("name").toString();
            userMap["passwd"] = query.value("passwd").toString();
            userMap["head"] = query.value("head").toString();
            userMap["status"] = query.value("status").toInt();
            userMap["lasttime"] = query.value("lasttime").toString();

            userList.append(userMap); // 添加到列表
        }
    } else {
        qDebug() << "查询失败：" << query.lastError().text();
    }

    return userList;
}


void DataBaseMag::queryAll()
{

}
