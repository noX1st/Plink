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

    virtual void createTable() override;

    bool getUser(User& user, const QString& table, const QString& email, const QString& password);

    void insertUser(const User& user);
};

#endif // SQL_H
