#include "sql.h"
#include <QDebug>
#include <QtSql/QSqlError>

const QString SqlConstants::HOSTNAME = "127.0.0.1";
const int SqlConstants::PORT = 3306;
const QString SqlConstants::USERNAME = "username";
const QString SqlConstants::PASSWORD = "password";
const QString SqlConstants::DRIVER = "QSQLITE";
const QString SqlConstants::USER_TABLE_NAME = "users";
const QString SqlConstants::FRIEND_TABLE_NAME = "friends";
const QString SqlConstants::MESSAGES_TABLE_NAME = "messages";
const QString SqlConstants::MAIN_CONNECTION = "main_connections";
const QString SqlConstants::DB_NAME = "Harmoniq.db";

Database::Database(const QString& connectionName, const QString& dbName)
{
    this->connectionName = connectionName;
    db = QSqlDatabase::addDatabase(SqlConstants::DRIVER, connectionName);
    db.setHostName(SqlConstants::HOSTNAME);
    db.setPort(SqlConstants::PORT);
    db.setUserName(SqlConstants::USERNAME);
    db.setPassword(SqlConstants::PASSWORD);
    db.setDatabaseName(dbName);

    if (!db.open()) {
        throw std::runtime_error(db.lastError().text().toStdString());
    }

    query = QSqlQuery(db);
    db.close();
}

Database::~Database()
{
    query.clear();
    query.finish();
    db.close();
    QSqlDatabase::removeDatabase(connectionName);
}

bool UserDatabase::openDatabase()
{
    if (!db.isOpen())
    {
        if (!db.open()) {
            qDebug() << "Database failed to open:" << db.lastError().text();
            return false;
        }
    }
    return true;
}

UserDatabase::UserDatabase(const QString& connectionName, const QString& dbName)
    : Database(connectionName, dbName)
{
    //createTable();
    //createFriendsTable();
    //createMessagesTable();
}

void UserDatabase::createTable()
{
    if (!openDatabase()) {
        throw std::runtime_error("Database connection failed");
    }

    QString request = "CREATE TABLE IF NOT EXISTS " + SqlConstants::USER_TABLE_NAME + " ("
                                                                                      "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                                                                                      "username TEXT NOT NULL UNIQUE, "
                                                                                      "email TEXT NOT NULL UNIQUE, "
                                                                                      "password TEXT NOT NULL, "
                                                                                      "birthday DATE NOT NULL);";

    QSqlQuery query(db);
    if (!query.exec(request)) {
        db.close();
        throw std::runtime_error(db.lastError().text().toStdString());
    }

    db.close();
}

void UserDatabase::createFriendsTable()
{
    if (!openDatabase()) {
        throw std::runtime_error("Database connection failed");
    }

    QString request = "CREATE TABLE IF NOT EXISTS " + SqlConstants::FRIEND_TABLE_NAME + " ("
                                                                                        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                                                                                        "user1 TEXT NOT NULL, "
                                                                                        "user2 TEXT NOT NULL, "
                                                                                        "status TEXT NOT NULL CHECK(status IN ('pending', 'accepted', 'rejected'))";

    QSqlQuery query(db);
    if (!query.exec(request)) {
        qDebug() << "Error creating friends table:" << query.lastError().text();
    }

    db.close();
}

void UserDatabase::createMessagesTable()
{
    if (!openDatabase()) {
        throw std::runtime_error("Database connection failed");
    }

    QString request = "CREATE TABLE IF NOT EXISTS " + SqlConstants::MESSAGES_TABLE_NAME + " ("
                                                                                          "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                                                                                          "sender TEXT NOT NULL, "
                                                                                          "receiver TEXT NOT NULL, "
                                                                                          "message TEXT NOT NULL, "
                                                                                          "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP)";

    QSqlQuery query(db);
    if (!query.exec(request)) {
        qDebug() << "Error creating messages table:" << query.lastError().text();
        throw std::runtime_error(query.lastError().text().toStdString());
    }

    db.close();
}

bool UserDatabase::getUserByUsername(const QString& username)
{
    if (!openDatabase()) {
        throw std::runtime_error("Database connection failed");
    }

    QSqlQuery query(db);
    query.prepare("SELECT 1 FROM " + SqlConstants::USER_TABLE_NAME + " WHERE username = :username;");
    query.bindValue(":username", username);

    if (!query.exec()) {
        qDebug() << "Error checking user existence:" << query.lastError().text();
        return false;
    }

    return query.next();
}

bool UserDatabase::getUser(User& user, const QString& table, const QString& email, const QString& password)
{
    if (!openDatabase()) {
        throw std::runtime_error("Database connection failed");
    }

    QString request = "SELECT * FROM " + table + " WHERE email = :email AND password = :password";
    QSqlQuery query(db);
    query.prepare(request);
    query.bindValue(":email", email);
    query.bindValue(":password", password);

    if (!query.exec()) {
        qDebug() << query.lastError().text();
        return false;
    }

    if (query.next()) {
        user.set_username(query.value("username").toString());
        user.set_password(query.value("password").toString());
        user.set_birthday(query.value("birthday").toDate());
        return true;
    }

    return false;
}

void UserDatabase::insertUser(const User& user)
{
    if (!openDatabase()) {
        throw std::runtime_error("Database connection failed");
    }

    QString request = "INSERT INTO " + SqlConstants::USER_TABLE_NAME + " ("
                                                                       "username, email, password, birthday) VALUES ("
                                                                       ":username, :email, :password, :birthday);";

    QSqlQuery query(db);
    query.prepare(request);
    query.bindValue(":username", user.get_username());
    query.bindValue(":email", user.get_email());
    query.bindValue(":password", user.get_password());
    query.bindValue(":birthday", user.get_birthday().toString());

    if (!query.exec()) {
        qDebug() << query.lastError().text();
        throw std::runtime_error(query.lastError().text().toStdString());
    }

    db.close();
}

QVector<QString> UserDatabase::getDirectMessages(const QString& username)
{
    if (!openDatabase()) {
        throw std::runtime_error("Database connection failed");
    }

    QVector<QString> directChats;
    QString request = "SELECT user2 FROM " + SqlConstants::FRIEND_TABLE_NAME + " WHERE user1 = :username AND status = 'accepted' "
                                                                               "UNION "
                                                                               "SELECT user1 FROM " + SqlConstants::FRIEND_TABLE_NAME + " WHERE user2 = :username AND status = 'accepted'";

    QSqlQuery query(db);
    query.prepare(request);
    query.bindValue(":username", username);

    if (!query.exec()) {
        qDebug() << "Error in getDirectMessages:" << query.lastError().text();
        return {};
    }

    while (query.next()) {
        directChats.append(query.value(0).toString());
    }

    return directChats;
}

QPair<QVector<QString>, int> UserDatabase::getAllFriends(const QString& username) const
{
    if (!db.isOpen()) {
        throw std::runtime_error("Database connection failed");
    }

    QVector<QString> friendsList;
    QString request = "SELECT user2 FROM " + SqlConstants::FRIEND_TABLE_NAME + " WHERE user1 = :username AND status = 'accepted' "
                                                                               "UNION "
                                                                               "SELECT user1 FROM " + SqlConstants::FRIEND_TABLE_NAME + " WHERE user2 = :username AND status = 'accepted'";

    QSqlQuery query(db);
    query.prepare(request);
    query.bindValue(":username", username);

    if (query.exec()) {
        while (query.next()) {
            friendsList.append(query.value(0).toString());
        }
    } else {
        qDebug() << "Error fetching friends list:" << query.lastError().text();
    }

    return qMakePair(friendsList, friendsList.size());
}

bool UserDatabase::sendFriendRequest(const QString& sender, const QString& receiver)
{
    if (!openDatabase()) {
        throw std::runtime_error("Database connection failed");
    }

    if (sender.isEmpty() || receiver.isEmpty()) {
        qDebug() << "Error: Sender or receiver is empty!";
        return false;
    }

    if (sender == receiver) {
        qDebug() << "Error: User cannot send a friend request to themselves!";
        return false;
    }

    QString request = "INSERT INTO " + SqlConstants::FRIEND_TABLE_NAME + " (user1, user2, status) "
                                                                         "VALUES (:sender, :receiver, 'pending');";

    QSqlQuery query(db);
    query.prepare(request);
    query.bindValue(":sender", sender);
    query.bindValue(":receiver", receiver);

    bool success = query.exec();

    if (!success) {
        qDebug() << "Error inserting friend request:" << query.lastError().text();
    }

    db.close();
    return success;
}

QPair<QVector<QString>, int> UserDatabase::getPendingRequests(const QString& username)
{
    if (!openDatabase()) {
        throw std::runtime_error("Database connection failed");
    }

    QVector<QString> pendingRequests;
    QString request = "SELECT user1 FROM " + SqlConstants::FRIEND_TABLE_NAME + " WHERE user2 = :username AND status = 'pending';";

    QSqlQuery query(db);
    query.prepare(request);
    query.bindValue(":username", username);

    if (query.exec()) {
        while (query.next()) {
            pendingRequests.append(query.value(0).toString());
        }
    } else {
        qDebug() << "Error fetching pending requests:" << query.lastError().text();
    }

    return qMakePair(pendingRequests, pendingRequests.size());
}

QString UserDatabase::getFriendshipStatus(const QString& user1, const QString& user2) const
{
    if (!db.isOpen()) {
        throw std::runtime_error("Database connection failed");
    }

    QString request = "SELECT status FROM " + SqlConstants::FRIEND_TABLE_NAME + " WHERE "
                                                                                "(user1 = :user1 AND user2 = :user2) OR (user1 = :user2 AND user2 = :user1);";

    QSqlQuery query(db);
    query.prepare(request);
    query.bindValue(":user1", user1);
    query.bindValue(":user2", user2);

    if (!query.exec() || !query.next()) {
        return "";
    }

    return query.value(0).toString();
}

bool UserDatabase::acceptFriendRequest(const QString& user1, const QString& user2)
{
    if (!openDatabase()) {
        throw std::runtime_error("Database connection failed");
    }

    QString request = "UPDATE " + SqlConstants::FRIEND_TABLE_NAME + " SET status = 'accepted' "
                                                                    "WHERE user1 = :user1 AND user2 = :user2 AND status = 'pending';";

    QSqlQuery query(db);
    query.prepare(request);
    query.bindValue(":user1", user1);
    query.bindValue(":user2", user2);

    bool success = query.exec();

    db.close();
    return success;
}

void UserDatabase::deleteFriend(const QString& user1, const QString& user2)
{
    if (!openDatabase()) {
        throw std::runtime_error("Database connection failed");
    }

    QString request = "DELETE FROM " + SqlConstants::FRIEND_TABLE_NAME + " WHERE "
                                                                         "(user1 = :user1 AND user2 = :user2) OR (user1 = :user2 AND user2 = :user1);";

    QSqlQuery query(db);
    query.prepare(request);
    query.bindValue(":user1", user1);
    query.bindValue(":user2", user2);

    if (!query.exec()) {
        qDebug() << "Failed to delete friend:" << query.lastError().text();
    }

    db.close();
}

void UserDatabase::sendMessage(const QString& sender, const QString& receiver, const QString& message)
{
    if (!openDatabase()) {
        throw std::runtime_error("Database connection failed");
    }

    if (!getUserByUsername(sender) || !getUserByUsername(receiver)) {
        qDebug() << "One of the users does not exist";
        throw std::runtime_error("User not found");
    }

    QString request = "INSERT INTO " + SqlConstants::MESSAGES_TABLE_NAME + " (sender, receiver, message) "
                                                                           "VALUES (:sender, :receiver, :message);";

    QSqlQuery query(db);
    query.prepare(request);
    query.bindValue(":sender", sender);
    query.bindValue(":receiver", receiver);
    query.bindValue(":message", message);

    if (!query.exec()) {
        qDebug() << "Error sending message:" << query.lastError().text();
        throw std::runtime_error(query.lastError().text().toStdString());
    }

    db.close();
}

QVector<QPair<QString, QString>> UserDatabase::getMessages(const QString& user1, const QString& user2)
{
    if (!openDatabase()) {
        throw std::runtime_error("Database connection failed");
    }

    QVector<QPair<QString, QString>> messages;
    QString request = "SELECT sender, message FROM " + SqlConstants::MESSAGES_TABLE_NAME + " WHERE "
                                                                                           "(sender = :user1 AND receiver = :user2) OR (sender = :user2 AND receiver = :user1) "
                                                                                           "ORDER BY timestamp ASC";

    QSqlQuery query(db);
    query.prepare(request);
    query.bindValue(":user1", user1);
    query.bindValue(":user2", user2);

    if (!query.exec()) {
        qDebug() << "Error fetching messages:" << query.lastError().text();
        throw std::runtime_error(query.lastError().text().toStdString());
    }

    while (query.next()) {
        messages.append(qMakePair(query.value(0).toString(), query.value(1).toString()));
    }

    return messages;
}
