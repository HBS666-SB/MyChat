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

    query.exec("PRAGMA foreign_keys = ON;");    // 启用SQLite外键约束

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

    sql = QString("CREATE TABLE IF NOT EXISTS FRIEND (");
    sql.append(QString("id INTEGER PRIMARY KEY AUTOINCREMENT,"));          // 好友记录ID（自增主键）
    sql.append(QString("user_id INTEGER NOT NULL,"));                     // 发起添加的用户ID（我的ID）
    sql.append(QString("friend_id INTEGER NOT NULL,"));                   // 被添加的好友ID（对方ID）
    sql.append(QString("remark VARCHAR(30) DEFAULT '',"));                // 好友备注名（自定义）
    sql.append(QString("status INTEGER NOT NULL DEFAULT 0,"));            // 好友状态（0=待验证/1=已通过/2=已拉黑）
    sql.append(QString("add_time DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,")); // 添加时间
    sql.append(QString("update_time DATETIME DEFAULT CURRENT_TIMESTAMP,"));// 状态更新时间
    // 外键关联USERINFO，级联删除
    sql.append(QString("FOREIGN KEY (user_id) REFERENCES USERINFO(id) ON DELETE CASCADE,"));
    sql.append(QString("FOREIGN KEY (friend_id) REFERENCES USERINFO(id) ON DELETE CASCADE,"));
    sql.append(QString("UNIQUE (user_id, friend_id));"));
    query.exec(sql);

    sql = QString("INSERT INTO USERINFO VALUES(1, 'admin', '123456', '2.bmp', %1, '');").arg(OffLine);    //插入数据
    query.exec(sql);

    //消息队列数据库
    sql = QString ("CREATE TABLE IF NOT EXISTS MESSAGEQUEUE (");
    sql.append(QString("id INTEGER PRIMARY KEY AUTOINCREMENT,"));
    sql.append(QString("request_userId INTEGER NOT NULL,"));
    sql.append(QString("accept_userId INTEGER NOT NULL,"));
    sql.append(QString("status INTEGER NOT NULL DEFAULT 0,"));  //0=待处理/1=已处理
    sql.append(QString("data TEXT NOT NULL,"));                   //数据部分
    sql.append(QString("message_type INTEGER NOT NULL);"));

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
    if(nId == -1){  //用户名或密码错误
        jsonObj.insert("msg", "noneUser");
    }else if(nId == -2){ //已在线
        jsonObj.insert("msg", "OnLine");
    }else {
        jsonObj.insert("msg", "ok");
    }
    jsonObj.insert("head", strHead);
    jsonObj.insert("code", code);

    return jsonObj;
}

bool DataBaseMag::isFriend(const int &userId, const QString &friendName)
{
    QSqlQuery query;
    const QString sql = "SELECT 1 FROM FRIEND "
                        "WHERE user_id = :user_id "
                        "AND friend_id = (SELECT id FROM USERINFO WHERE name = :friendName) "
                        "LIMIT 1";
    query.prepare(sql);
    query.bindValue(":user_id", userId);
    query.bindValue(":friendName", friendName);

    if(!query.exec()){
        qDebug() << "判断是否为好友失败";
        return false;
    }
    return query.next();

}

QString DataBaseMag::getUserHead(const int &userId)
{
    QSqlQuery query;
    QString sql = "SELECT head FROM USERINFO WHERE id = :userId;";
    query.prepare(sql);
    query.bindValue(":userId", userId);
    if(!query.exec())
    {
        qDebug() << "获取用户头像路径错误" << query.lastError();
        return "";
    }
    if(!query.next()){
        qDebug() << "没有查询到该用户";
        return  "";
    }
    return query.value("head").toString();
}

bool DataBaseMag::haveUser(const QString &name)
{
    if(name.isEmpty()){
        return false;
    }
    QSqlQuery query;
    const QString sql("SELECT name FROM USERINFO where name = ?");
    query.prepare(sql);
    query.addBindValue(name);

    if(!query.exec()){
        return false;
    }
    return query.next();
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

int DataBaseMag::getIdFromUsername(const QString &userName)
{
    QSqlQuery query;
    QString sql("select id from userinfo where name = :name");
    query.prepare(sql);
    query.bindValue(":name", userName);
    if(!query.exec()){
        qDebug() << "添加好友：没有该用户";
        return -1;
    }
    if(!query.next()) return -1;
    return query.value("id").toInt();
}


QString DataBaseMag::getUsernameFromId(const QString &userId)
{
    if(userId.isEmpty()) {
        qDebug() << "userId不能为空";
        return "";
    }
    QSqlQuery query;
    QString sql("select name from userinfo where id = :userId");
    query.prepare(sql);
    query.bindValue(":userId", userId);
    if(!query.exec()){
        qDebug() << "通过id获取用户名失败";
        return "";
    }
    if(!query.next()){
        qDebug() << "userId:" << userId << "无匹配数据：userId=" << userId;
        return "";
    }
    return query.value("name").toString();
}

QString DataBaseMag::jsonValueToString(const QJsonValue &jsonVal)
{
    if (jsonVal.isNull() || jsonVal.isUndefined()) {
        qWarning() << "QJsonValue为空或未定义，转换失败";
        return "";
    }

    QJsonDocument jsonDoc;
    if (jsonVal.isObject()) {
        jsonDoc = QJsonDocument(jsonVal.toObject());
    } else if (jsonVal.isArray()) {
        jsonDoc = QJsonDocument(jsonVal.toArray());
    } else {
        jsonDoc = QJsonDocument(QJsonArray{jsonVal});
    }

    return jsonDoc.toJson(QJsonDocument::Compact);
}

void DataBaseMag::insertMessageQueue(const int &senderId, const int &acceptId, quint8 type, const QJsonValue &dataVal)
{
    qDebug() << dataVal;
    if(senderId < 0 || acceptId < 0){
        qDebug() << "无效的消息不能插入消息队列";
        return;
    }
    QString data = jsonValueToString(dataVal);
    QSqlQuery query;
    QString sql("INSERT INTO MESSAGEQUEUE (request_userId, accept_userId, data, message_type)"
                " VALUES (:request_userId, :accept_userId, :data, :message_type);");
    query.prepare(sql);
    query.bindValue(":request_userId",senderId);
    query.bindValue(":accept_userId",acceptId);
    query.bindValue(":data",data);
    query.bindValue(":message_type",static_cast<int>(type));

    if(!query.exec()){
        qDebug() << "消息队列插入数据失败：" << "senderId = " << senderId
                 << "accessId = " << acceptId
                 << "message_type = " << static_cast<int>(type)
                 << "data:" << data;
        return;
    }
}

QList<QVariantMap> DataBaseMag::getUserMessageQueue(const int &userId)
{
    QList<QVariantMap> msgList;
    QStringList  needUpdateIds;
    QSqlQuery query;
    QString sql("SELECT id, request_userId, accept_userId, data, message_type "
                "FROM MESSAGEQUEUE WHERE accept_userId = :userId AND status = 0;");
    query.prepare(sql);
    query.bindValue(":userId",userId);
    if(!query.exec()){
        qDebug() << "获取用户的消息队列失败";
        return msgList;
    }
    while(query.next()){
        //取出json
        QString jsonStr = query.value("data").toString();
        QByteArray jsonByte = jsonStr.toUtf8();
        QJsonParseError error;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonByte, &error);
        if (error.error != QJsonParseError::NoError) {
            qCritical() << "JSON解析失败：" << error.errorString();
            return msgList;
        }
        QVariant  jsonVar = jsonDoc.toVariant();
        QJsonValue jsonVal = QJsonValue::fromVariant(jsonVar);

        QString id = query.value("id").toString();
        QVariantMap msgMap;
        msgMap["request_userId"] = query.value("request_userId");
        msgMap["accept_userId"] = query.value("accept_userId");
        msgMap["data"] = jsonVal;
        msgMap["message_type"] = query.value("message_type");

        msgList.append(msgMap);
        needUpdateIds.append(id);
    }
    if(needUpdateIds.isEmpty()){
        qDebug() << "获取用户消息队列：没有要更新的状态";
        return msgList;
    }

    sql = QString("UPDATE MESSAGEQUEUE SET status = 1 where id in (%1);").arg(needUpdateIds.join(","));
    QSqlQuery updateQuery(sql);
    if(!updateQuery.exec()){
        qDebug() << "获取用户消息队列：SQL更新语句有误";
    }

    return msgList;
}

void DataBaseMag::addFriend(const int &userId, const int &friendId)
{
    QSqlQuery query;
    QString sql;
    sql = QString("INSERT INTO FRIEND (user_id, friend_id, remark, status, add_time, update_time)");
    sql.append("VALUES (:userId, :friendId, :remark, 1, CURRENT_TIMESTAMP, CURRENT_TIMESTAMP);");

    query.prepare(sql);
    query.bindValue(":userId",userId);
    query.bindValue(":friendId",friendId);
    query.bindValue(":remark",DataBaseMag::getInstance()->getUsernameFromId(QString::number(friendId)));

    if(!query.exec()){
        qDebug() << "服务器存储好友信息出错";
        return;
    }
    return;
}

void DataBaseMag::deleteFriend(const int &userId, const int &friendId)
{
    QSqlQuery query;
    QString sql;
    sql = QString("DELETE FROM FRIEND WHERE user_id = :userId AND friend_id = :friendId");
    query.prepare(sql);
    query.bindValue(":userId", userId);
    query.bindValue(":friendId", friendId);
    if(!query.exec()){
        qDebug() << "从数据库删除好友出错" << query.lastError();
        return;
    }
}

bool DataBaseMag::isOnline(const int &userId)
{
    QSqlQuery query;
    QString sql;
    sql = QString("SELECT status FROM USERINFO WHERE id = :userId");
    query.prepare(sql);
    query.bindValue(":userId",userId);

    if(!query.exec()){
        qDebug() << "获取用户在线状态失败";
        return true;    //失败就假设为在线
    }
    if(!query.next()){
        qDebug() << "用户不存在";    //不存在就假设为在线
        return true;
    }
    return query.value("status").toInt() == OnLine ? true : false;
}

// 发送Jsonarray  status  head    name    id
QJsonValue DataBaseMag::getMyFriends(const int &userId)
{
    QJsonArray jsonArr;
    QSqlQuery query;
    QString sql;
    sql = QString("SELECT status, head, name, id FROM USERINFO ");
    sql.append("WHERE id IN (SELECT friend_id FROM FRIEND WHERE user_id = :userId);");
    query.prepare(sql);
    query.bindValue(":userId", userId);

    if(!query.exec()){
        qDebug() << "获取用户好友失败 sql执行出错" << query.lastError();;
    }
    while(query.next()){
        QJsonObject jsonObj;
        jsonObj.insert("status",query.value("status").toInt());
        jsonObj.insert("head", query.value("head").toString());
        jsonObj.insert("name", query.value("name").toString());
        jsonObj.insert("id", query.value("id").toInt());
        jsonArr.append(jsonObj);
    }

    QJsonValue sendJsonVal(jsonArr);
    return sendJsonVal;
}

QJsonValue DataBaseMag::getMyGroup(const int &userId)
{
    QJsonArray jsonArr;
    QSqlQuery query;
//    QString sql = QString("SELECT group_id, identity FROM GROUP_MEMBER WHERE user_id = :userId;");
    QString sql = QString("SELECT gm.group_id, gm.identity, g.group_head, g.group_name, g.creator_id ");
    sql.append("FROM GROUP_MEMBER gm LEFT JOIN GROUPS g ON gm.group_id = g.group_id ");
    sql.append("WHERE gm.user_id = :userId");
    query.prepare(sql);
    query.bindValue(":userId", userId);
    if(!query.exec()) {
        qDebug() << "获取小组信息失败" << query.lastError();
    }
    while(query.next()){
        QJsonObject jsonObj;
        jsonObj.insert("id", query.value("group_id").toInt());
        jsonObj.insert("identity", query.value("identity").toInt());
        jsonObj.insert("head", query.value("group_head").toString());
        jsonObj.insert("name", query.value("group_name").toString());
        jsonObj.insert("owner", query.value("creator_id").toInt());
        jsonArr.append(jsonObj);
    }
    return QJsonValue(jsonArr);
}

int DataBaseMag::addGroup(const int &userId, const QString groupName)
{
    if(userId < 0 || groupName.isEmpty()){
        qDebug() << "数据库插入群组失败";
        return 0;
    }
    QSqlQuery query(userdb);
    QString sql;
    sql = QString("INSERT INTO GROUPS (group_name, creator_id) ");
    sql.append("VALUES (:groupName, :creatorId);");
    query.prepare(sql);
    query.bindValue(":groupName", groupName);
    query.bindValue(":creatorId", userId);
    if(!query.exec()){
        qDebug() << "添加群组出错" << query.lastError();
        return 0;
    }
    QVariant insertId = query.lastInsertId();
    if (!insertId.isValid()) {
        qDebug() << "获取group_id失败：ID无效";
        return 0;
    }
    qlonglong groupIdLL = insertId.toLongLong();
    int groupId = static_cast<int>(groupIdLL);

//    qDebug() << "插入成功，group_id=" << groupId; // 打印验证
    addGroupMember(userId, groupId, Owner);
    return groupId;
}

bool DataBaseMag::addGroupMember(const int &userId, const int &groupId, GroupIdentity identity)
{
    if(userId < 0 || groupId < 0) return false;
    QSqlQuery query;
    QString sql = "INSERT INTO GROUP_MEMBER (group_id, user_id, identity) VALUES (:groupId, :userId, :identity);";
    query.prepare(sql);
    query.bindValue(":groupId", groupId);
    query.bindValue(":userId", userId);
    query.bindValue(":identity", identity);
    if(!query.exec()){
        qDebug() << "添加群成员错误" << query.lastError();
        return false;
    }
    return true;
}

int DataBaseMag::getGroupOwner(const int &groupId)
{
    QSqlQuery query;
    QString sql = "SELECT creator_id from GROUPS WHERE group_id = :groupId";
    query.prepare(sql);
    query.bindValue(":groupId", groupId);
    if(!query.exec()){
        qDebug() << "获取群主Id错误" << query.lastError();
        return -1;
    }
    query.next();
    return query.value("creator_id").toInt();
}

QString DataBaseMag::getGroupName(const int &groupId)
{
    QSqlQuery query;
    QString sql = "SELECT group_name FROM GROUPS WHERE group_id = :groupId";
    query.prepare(sql);
    query.bindValue(":groupId", groupId);
    if(!query.exec()){
        qDebug() << "获取群组名错误" << query.lastError();
        return "";
    }
    query.next();
    return query.value("group_name").toString();
}

QString DataBaseMag::getGroupHead(const int &groupId)
{
    QSqlQuery query;
    QString sql = "SELECT group_head FROM GROUPS WHERE group_id = :groupId";
    query.prepare(sql);
    query.bindValue(":groupId", groupId);
    if(!query.exec()){
        qDebug() << "获取群组头像错误" << query.lastError();
        return "";
    }
    query.next();
    return query.value("group_head").toString();
}

QHash<int, QSet<int> > DataBaseMag::initGroupMembersCache()
{
    QHash<int, QSet<int> > hashSet;
    QString sql = "SELECT group_id, user_id FROM GROUP_MEMBER";
    QSqlQuery query(sql);
    if(!query.exec()) {
        qDebug() << "初始化群组成员错误";
        return hashSet;
    }
    while(query.next()) {
        int groupId = query.value(0).toInt();
        int userId = query.value(1).toInt();
        hashSet[groupId].insert(userId);
    }
    qDebug() << "服务器启动：加载" << hashSet.size() << "个群组的成员缓存";
    return hashSet;
}

QJsonValue DataBaseMag::getGroupMember(const int &groupId)
{
    QJsonArray jsonArr;
    QSqlQuery query;
    QString sql = "SELECT user_id, identity, join_time FROM GROUP_MEMBER WHERE group_id = :groupId";
    query.prepare(sql);
    query.bindValue(":groupId", groupId);
    if(!query.exec())
    {
        qDebug() << "获取群成员错误" << query.lastError();
        return QJsonValue(jsonArr);
    }
    while(query.next()) {
        QJsonObject jsonObj;
        jsonObj.insert("groupId", groupId);
        jsonObj.insert("userId", query.value("user_id").toInt());
        jsonObj.insert("identity", query.value("identity").toInt());
        jsonObj.insert("joinTime", query.value("join_time").toString());
        jsonArr.append(jsonObj);
    }
    return QJsonValue(jsonArr);
}

void DataBaseMag::queryAll()
{

}
