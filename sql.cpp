#include "sql.h"
#include <QDebug>
#include <QtSql/QSqlError>

const QString sqlConstans::hostname = "127.0.0.1";
const int sqlConstans::port = 3306;
const QString sqlConstans::username = "noX1st";
const QString sqlConstans::password = "password";
const QString sqlConstans::driver = "QSQLITE";
const QString sqlConstans::userTableName = "users";
const QString sqlConstans::friendTableName = "friends";
const QString sqlConstans::mainConnection = "main_connections";
const QString sqlConstans::DBname = "Harmoniq.db";

sql::sql(const QString& connectionN, const QString& dbN)
{
    connectionName = connectionN;
    DB = QSqlDatabase::addDatabase(sqlConstans::driver, connectionName);
    DB.setHostName(sqlConstans::hostname);
    DB.setPort(sqlConstans::port);
    DB.setUserName(sqlConstans::username);
    DB.setPassword(sqlConstans::password);
    DB.setDatabaseName(dbN);
    if (!DB.open())
    {
        throw std::runtime_error(DB.lastError().text().toStdString());
    }
    else
    {
        qDebug()<<"connect is successfully created";
    }
    query = QSqlQuery(DB);
    DB.close();
}

sql::~sql()
{
    query.clear();
    query.finish();
    DB.close();
    QSqlDatabase::removeDatabase(connectionName);
}

sqlUser::sqlUser(const QString& connectionN, const QString& dbN) : sql(connectionN, dbN){}

bool sqlUser::openDatabase() {
    if (!DB.isOpen()) {
        if (!DB.open()) {
            qDebug() << "Database failed to open:" << DB.lastError().text();
            return false;
        }
    }
    return true;
}


void sqlUser::createTable()
{
    DB.open();
    request = "CREATE TABLE IF NOT EXISTS " + sqlConstans::userTableName + " ("
                                                                           "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                                                                           "username TEXT NOT NULL UNIQUE, "
                                                                           "email TEXT NOT NULL UNIQUE, "
                                                                           "password TEXT NOT NULL, "
                                                                           "birthday DATE NOT NULL);";

    if (!query.exec(request))
    {
        DB.close();
        throw std::runtime_error(DB.lastError().text().toStdString());
    }
    else
    {
        qDebug()<<"TABLE " + sqlConstans::userTableName + " CREATED SUCCESFULLY";
        DB.close();
    }
}

void sqlUser::createFriendsTable() {
    if (!DB.isOpen()) {
        DB.open();
    }
    request = "CREATE TABLE IF NOT EXISTS " + sqlConstans::friendTableName + " ("
                                                                             "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                                                                             "user1 TEXT NOT NULL, "
                                                                             "user2 TEXT NOT NULL, "
                                                                             "status TEXT NOT NULL CHECK(status IN ('pending', 'accepted', 'rejected'))"
                                                                             ");";
    QSqlQuery query(DB);
    if (!query.exec(request)) {
        qDebug() << "Error creating friends table:" << query.lastError().text();
    } else {
        qDebug() << "TABLE " + sqlConstans::friendTableName + " CREATED SUCCESSFULLY";
    }

    DB.close();
}

bool sqlUser::sendFriendRequest(const QString& sender, const QString& receiver) {
    if (sender.isEmpty() || receiver.isEmpty()) {
        qDebug() << "Error: Sender or receiver is empty!";
        return false;
    }

    if (sender == receiver) {
        qDebug() << "Error: User cannot send a friend request to themselves!";
        return false;
    }

    if (!DB.isOpen()) {
        DB.open();
    }

    QSqlQuery query(DB);
    query.prepare("INSERT INTO " + sqlConstans::friendTableName + " (user1, user2, status) "
                                                                  "VALUES (:sender, :receiver, 'pending');");

    query.bindValue(":sender", sender);
    query.bindValue(":receiver", receiver);

    bool success = query.exec();

    if (!success) {
        qDebug() << "Error inserting friend request:" << query.lastError().text();
        qDebug() << "Executed query:" << query.lastQuery();
    } else {
        qDebug() << "Friend request sent successfully from" << sender << "to" << receiver;
    }

    DB.close();
    return success;
}

bool sqlUser::getUserByUsername(const QString& username) {
    DB.open();
    request = "SELECT * FROM " + sqlConstans::userTableName + " WHERE username = :username;";
    query.prepare(request);
    query.bindValue(":username", username);

    bool exists = query.exec() && query.next();
    DB.close();
    return exists;
}

bool sqlUser::getUser(User& user, const QString& table, const QString& email, const QString& password)
{
    DB.open();
    request = "select * from " + table + " where email=:emailID and password=:passwordID";
    query.prepare(request);
    query.bindValue(":emailID", email);
    query.bindValue(":passwordID", password);
    if (!query.exec())
    {
        qDebug()<<query.lastError().text().toStdString();
        DB.close();
        return false;
    }
    else
    {
        qDebug()<<"data insert succesfully";
        DB.close();
    }
    if (query.next())
    {
        user.set_username(query.value("username").toString());
        user.set_password(query.value("password").toString());
        user.set_birthday(query.value("birthday").toDate());
        return true;
    }
    return false;
}

void sqlUser::insertUser(const User& user)
{
    DB.open();
    request = "INSERT INTO " + sqlConstans::userTableName + " ("
                                                            "username, email, password, birthday) VALUES ("
                                                            ":usernameID, :emailID, :passwordID, :birthdayID);";
    query.prepare(request);
    query.bindValue(":usernameID", user.get_username());
    query.bindValue(":emailID", user.get_email());
    query.bindValue(":passwordID", user.get_password());
    query.bindValue(":birthdayID", user.get_birthday().toString());

    if (!query.exec())
    {
        qDebug()<<query.lastError().text().toStdString();
        throw std::runtime_error(query.lastError().text().toStdString());
    }
    else
    {
        qDebug()<<"data insert succesfully";
    }

    DB.close();
}

QPair<QVector<QString>, int> sqlUser::getAllFriends(const QString &username) {
    QVector<QString> friendsList;
    if (!DB.isOpen()) DB.open();
    QSqlQuery query(DB);
    query.prepare("SELECT user2 FROM " + sqlConstans::friendTableName + " WHERE user1 = :username AND status = 'accepted' "
                  "UNION "
                  "SELECT user1 FROM " + sqlConstans::friendTableName + " WHERE user2 = :username AND status = 'accepted'");
    query.bindValue(":username", username);

    if (query.exec()) {
        while (query.next()) {
            friendsList.append(query.value(0).toString());
        }
        //qDebug() << "User" << username << "has" << friendsList.size() << "friends.";
    } else {
        qDebug() << "Error fetching friends list:" << query.lastError().text();
    }

    return QPair<QVector<QString>, int>(friendsList, friendsList.size());
}

QPair<QVector<QString>, int> sqlUser::getPendingRequests(const QString& username) {
    QVector<QString> pendingRequests;
    if (!DB.isOpen()) DB.open();
    request = "SELECT user1 FROM " + sqlConstans::friendTableName + " WHERE user2 = :username AND status = 'pending';";
    query.prepare(request);
    query.bindValue(":username", username);

    if (query.exec()) {
        while (query.next()) {//
            pendingRequests.append(query.value(0).toString());
        }
    } else {
        qDebug() << "Error fetching pending requests:" << query.lastError().text();
    }

    DB.close();
    return QPair<QVector<QString>, int>(pendingRequests, pendingRequests.size());
}

QString sqlUser::getFriendshipStatus(const QString& user1, const QString& user2) {
    if (!DB.isOpen()) DB.open();
    request = "SELECT status FROM " + sqlConstans::friendTableName + " WHERE "
                                                                     "(user1 = :user1 AND user2 = :user2) OR (user1 = :user2 AND user2 = :user1);";
    query.prepare(request);
    query.bindValue(":user1", user1);
    query.bindValue(":user2", user2);

    if (!query.exec() || !query.next()) {
        DB.close();
        return "";
    }

    QString status = query.value(0).toString();
    DB.close();
    return status;
}

bool sqlUser::acceptFriendRequest(const QString& user1, const QString& user2) {
    if (!DB.isOpen()) DB.open();
    request = "UPDATE " + sqlConstans::friendTableName + " SET status = 'accepted' "
                                                         "WHERE user1 = :user1 AND user2 = :user2 AND status = 'pending';";

    query.prepare(request);
    query.bindValue(":user1", user1);
    query.bindValue(":user2", user2);

    bool success = query.exec();
    DB.close();
    return success;
}

void sqlUser::deleteFriend(const QString& user1, const QString& user2)
{
    if (!DB.isOpen()) DB.open();

    QSqlQuery query(DB);

    query.prepare("DELETE FROM " + sqlConstans::friendTableName + " WHERE (user1 = :user1 AND user2 = :user2) OR (user1 = :user2 AND user2 = :user1)");
    query.bindValue(":user1", user1);
    query.bindValue(":user2", user2);

    if (!query.exec()) {
        qDebug() << "Failed to delete friend:" << query.lastError().text();
    } else {
        qDebug() << "Friend deleted successfully!";
    }
}
