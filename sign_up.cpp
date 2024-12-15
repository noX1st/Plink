#include "sign_up.h"
#include "ui_sign_up.h"

sign_up::sign_up(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::sign_up)
{
    ui->setupUi(this);
}

sign_up::~sign_up()
{
    delete ui;
}

void sign_up::on_signUpBtn_clicked()
{
    emit signUpBtnClicked();
}

QString sign_up::get_username() const
{
    return ui->usernameLine->text();
}

QString sign_up::get_email() const
{
    return ui->emailLine->text();
}
QString sign_up::get_password() const
{
    return ui->passwordLine->text();
}
QDate sign_up::get_birthday() const
{
    return ui->birthdayLine->date();
}
QString sign_up::get_confirm() const
{
    return ui->confirmLine->text();
}
