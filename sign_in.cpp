#include "sign_in.h"
#include "ui_sign_in.h"
#include <QApplication>
#include <QCloseEvent>

sign_in::sign_in(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::sign_in)
{
    ui->setupUi(this);
}

sign_in::~sign_in()
{
    delete ui;
}

void sign_in::on_signInBtn_clicked()
{
    emit signInBtn_clicked();
}

void sign_in::on_signUpBtn_clicked()
{
    emit signUpBtn_clicked();
}

QString sign_in::get_email() const
{
    return ui->emailLine->text();
}

QString sign_in::get_password() const
{
    return ui->passwordLine->text();
}

void sign_in::closeEvent(QCloseEvent *event)
{
    QApplication::quit();
    event->accept();
}
