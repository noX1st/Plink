#ifndef SIGN_UP_H
#define SIGN_UP_H

#include <QDialog>

namespace Ui {
class sign_up;
}

class sign_up : public QDialog
{
    Q_OBJECT

public:
    explicit sign_up(QWidget *parent = nullptr);
    ~sign_up();
    QString get_username() const;
    QString get_email() const;
    QString get_password() const;
    QDate get_birthday() const;
    QString get_confirm() const;

private slots:
    void on_signUpBtn_clicked();

signals:
    void signUpBtnClicked();

private:
    Ui::sign_up *ui;
};

#endif // SIGN_UP_H
