#ifndef SQL_H
#define SQL_H
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include "user.h"

struct sqlConstans{
    static const QString hostname;
    static const int port;
    static const QString username;
    static const QString password;
    static const QString driver;
    static const QString userTableName;
    static const QString friendTableName;
    static const QString mainConnection;
    static const QString DBname;
};

class sql
{
protected:
    QSqlDatabase DB;
    QSqlQuery query;
    QString request;
    QString connectionName;

public:
    sql(const QString& connectionN, const QString& dbN);

    virtual ~sql();

    virtual void createTable() = 0;
};

class sqlUser : public sql
{
public:
    sqlUser(const QString& connectionN, const QString& dbN);

    bool openDatabase();

    virtual void createTable() override;

    void createFriendsTable();

    void createMessagesTable();

    bool sendFriendRequest(const QString& sender, const QString& receiver);

    bool getUserByUsername(const QString& username);

    bool getUser(User& user, const QString& table, const QString& email, const QString& password);

    void insertUser(const User& user);

    QVector<QString> getDirectMessages(const QString& username);

    QPair<QVector<QString>, int> getAllFriends(const QString& username);

    QPair<QVector<QString>, int> getPendingRequests(const QString& username);

    QString getFriendshipStatus(const QString& user1, const QString& user2);

    bool acceptFriendRequest(const QString& user1, const QString& user2);

    void deleteFriend(const QString& user1, const QString& user2);
};

#endif // SQL_H
