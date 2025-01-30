#ifndef USER_H
#define USER_H
#include <QString>
#include <QDate>

class User
{
private:
    QString username;
    QString userTag;
    QString email;
    QString password;
    QDate birthday;
public:
    User();
    User(const QString& username, const QString& email, const QString& password, const QDate& birthday);

    User(const User& other);

    const QString get_username() const;

    const QString get_email() const;

    const QString get_password() const;

    const QDate get_birthday() const;

    void set_username(const QString& username_);

    void set_email(const QString& email_);

    void set_password(const QString& password_);

    void set_birthday(const QDate& birthday_);

    /*User& operator++();

    User operator++(int);

    User& operator--();

    User operator--(int);
    User& operator=(const User& other);*/
};

#endif // USER_H
