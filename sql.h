#ifndef SQL_H
#define SQL_H

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include "user.h"

struct SqlConstants {
    static const QString HOSTNAME;
    static const int PORT;
    static const QString USERNAME;
    static const QString PASSWORD;
    static const QString DRIVER;
    static const QString USER_TABLE_NAME;
    static const QString FRIEND_TABLE_NAME;
    static const QString MESSAGES_TABLE_NAME;
    static const QString MAIN_CONNECTION;
    static const QString DB_NAME;
};

class Database
{
protected:
    QSqlDatabase db;
    QSqlQuery query;
    QString request;
    QString connectionName;

public:
    Database(const QString& connectionName, const QString& dbName);
    virtual ~Database();
    virtual void createTable() = 0;
};

class UserDatabase : public Database
{
public:
    UserDatabase(const QString& connectionName, const QString& dbName);

    bool openDatabase();
    virtual void createTable() override;
    void createFriendsTable();
    void createMessagesTable();

    bool sendFriendRequest(const QString& sender, const QString& receiver);
    bool getUserByUsername(const QString& username);
    bool getUser(User& user, const QString& table, const QString& email, const QString& password);
    void insertUser(const User& user);

    QVector<QString> getDirectMessages(const QString& username);
    QPair<QVector<QString>, int> getAllFriends(const QString& username) const;
    QPair<QVector<QString>, int> getPendingRequests(const QString& username);
    QString getFriendshipStatus(const QString& user1, const QString& user2) const;
    bool acceptFriendRequest(const QString& user1, const QString& user2);
    void deleteFriend(const QString& user1, const QString& user2);

    void sendMessage(const QString& sender, const QString& receiver, const QString& message);
    QVector<QPair<QString, QString>> getMessages(const QString& user1, const QString& user2);
};

#endif // SQL_H
