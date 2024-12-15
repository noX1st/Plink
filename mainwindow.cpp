#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "sign_in.h"
#include "sql.h"
#include "QMessageBox"
#include <QMouseEvent>
#include <QSettings>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , sign_in(new class sign_in(this))
    , registration(new sign_up(this))
    , user(nullptr)
    , ui(new Ui::MainWindow)
    , userSql(new sqlUser(sqlConstans::mainConnection, sqlConstans::DBname))
{
    ui->setupUi(this);

    setupEventFilter();

    friendsButtonGroup = new QButtonGroup(this);
    friendsButtonGroup->addButton(ui->friendsOnlineBtn, 0);
    friendsButtonGroup->addButton(ui->friendsAllBtn, 3);
    friendsButtonGroup->addButton(ui->friendsPendingBtn, 1);
    friendsButtonGroup->addButton(ui->friendsAddBtn, 2);
    friendsButtonGroup->setExclusive(true);

    connect(friendsButtonGroup, QOverload<QAbstractButton *>::of(&QButtonGroup::buttonClicked), this, [this](QAbstractButton *button){
        int id = friendsButtonGroup->id(button);
        ui->stackedWidgetFriends->setCurrentIndex(id);
    });

    connect(&sign_in, &sign_in::signInBtn_clicked, this, &MainWindow::signIn);
    connect(&registration, &sign_up::signUpBtnClicked, this, &MainWindow::signUp);
    connect(&registration, SIGNAL(finished(int)), &sign_in, SLOT(show()));
    connect(&sign_in, SIGNAL(signUpBtn_clicked()), this, SLOT(changeWindow()));

    userSql->createTable();
}

void MainWindow::changeWindow()
{
    registration.show();
    sign_in.hide();
}

/*void MainWindow::exit()
{
    mainMenu.close();
}*/

void MainWindow::exitApp()
{
    sign_in.close();
}

void MainWindow::signIn()
{
    delete user;
    user = new User;
    sqlUser* sql = dynamic_cast<sqlUser*> (userSql);
    if (sql == nullptr)
    {
        throw (std::runtime_error("dynamic cast error"));
    }
    /*if (!sql->getUser(*user, sqlConstans::userTableName, sign_in.get_email(), sign_in.get_password()))
    {
        QMessageBox MSG(
            QMessageBox::Warning, "warning", "you don't have an account", QMessageBox::Ok);
        MSG.exec();
        return;
    }
    QMessageBox MSG(
        QMessageBox::Information, "information", "sign in was successful", QMessageBox::Ok);
    MSG.exec();

    sign_in.hide();

    this->show();*/

    if (!sql->getUser(*user, sqlConstans::userTableName, sign_in.get_email(), sign_in.get_password()))
    {
        QMessageBox::warning(this, "warning", "The entered data is incorrect", QMessageBox::Ok);
        return;
    }
    QMessageBox::information(this, "information", "Sign in was successful", QMessageBox::Ok);
    this->show();
    sign_in.hide();
}

void MainWindow::signUp()
{
    if (registration.get_username().isEmpty() || registration.get_email().isEmpty() || registration.get_password().isEmpty() || registration.get_confirm().isEmpty())
    {
        QMessageBox::warning(this, "Warning", "Please fill in all fields.", QMessageBox::Ok);
        return;
    }
    if (!registration.get_email().contains('@'))
    {
        QMessageBox::warning(this, "Warning", "Please enter a valid email", QMessageBox::Ok);
        return;
    }
    if (registration.get_password() != registration.get_confirm())
    {
        QMessageBox MSG(
            QMessageBox::Warning, "warning", "passwords doesn't match", QMessageBox::Ok);
        MSG.exec();
        return;
    }
    delete user;
    user = new User(registration.get_username(), registration.get_email(), registration.get_password(), registration.get_birthday());
    sqlUser* sql = dynamic_cast<sqlUser*> (userSql);
    if (sql == nullptr)
    {
        throw (std::runtime_error("dynamic cast error"));
    }
    if (sql->getUser(*user, sqlConstans::userTableName, registration.get_email(), registration.get_password()))
    {
        QMessageBox::warning(this, "Warning", "This user already exists", QMessageBox::Ok);
        return;
    }
    sql->insertUser(*user);
    QMessageBox MSG(
        QMessageBox::Information, "information", "Sign up was succesfull", QMessageBox::Ok);

    MSG.exec();
    sign_in.show();
    registration.hide();
}

void setupButtonStyle(QPushButton *button) {
    button->setStyleSheet("QPushButton"
                          "{ background-color: #3498db; border: none; border-radius: 15px; padding: 3px; min-width: 30px; min-height: 30px; max-width: 30px; max-height: 30px; }"
                          "QPushButton:hover"
                          "{ background-color: #2980b9; }"
                          "QPushButton:pressed"
                          "{ background-color: #1c5985; }");
}

void MainWindow::on_addPushButton_clicked()
{
    QVBoxLayout *layout = qobject_cast<QVBoxLayout*>(ui->scrollAreaWidgetContent->layout());

    if (layout) {
        QPushButton *newButton = new QPushButton("", this);
        setupButtonStyle(newButton);

        int plusButtonIndex = -1;
        for (int i = 0; i < layout->count(); ++i)
        {
            QWidget *widget = layout->itemAt(i)->widget();
            if (widget && widget->objectName() == "addPushButton")
            {
                plusButtonIndex = i;
                break;
            }
        }

        if (plusButtonIndex != -1) {
            layout->insertWidget(plusButtonIndex, newButton);
        }
        else
        {
            layout->addWidget(newButton);
        }
    }
}

void MainWindow::setupEventFilter()
{
    ui->friendsPage->installEventFilter(this);
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->friendsPage && event->type() == QEvent::MouseButtonPress) {
        QWidget *focusWidget = QApplication::focusWidget();
        if (focusWidget) {
            focusWidget->clearFocus();
        }
    }
    return QMainWindow::eventFilter(obj, event);
}


void MainWindow::display()
{
    sign_in.show();
}


MainWindow::~MainWindow()
{
    delete userSql;
    delete user;
    delete ui;
}
