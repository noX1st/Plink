#include "sql.h"
#include <QDebug>
#include <QtSql/QSqlError>

const QString sqlConstans::hostname = "127.0.0.1";
const int sqlConstans::port = 3306;
const QString sqlConstans::username = "noX1st";
const QString sqlConstans::password = "password";
const QString sqlConstans::driver = "QSQLITE";
const QString sqlConstans::userTableName = "users";
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
