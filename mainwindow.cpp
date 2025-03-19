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

    serversAndDmButtonGroup = new QButtonGroup(this);
    serversAndDmButtonGroup->addButton(ui->directMessagesBtn, 0);
    serversAndDmButtonGroup->addButton(ui->serverBtn, 1);
    serversAndDmButtonGroup->setExclusive(true);

    connect(serversAndDmButtonGroup, QOverload<QAbstractButton *>::of(&QButtonGroup::buttonClicked), this, [this](QAbstractButton *button){
        int id = serversAndDmButtonGroup->id(button);
        ui->serversAndDmStackedWidget->setCurrentIndex(id);
    });


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
    userSql->createFriendsTable();


    ui->addFriendBtn->setParent(ui->friendLineEdit);
    ui->addFriendBtn->move(ui->friendLineEdit->width() - ui->addFriendBtn->width() - 5,
                           (ui->friendLineEdit->height() - ui->addFriendBtn->height()) / 2);
    ui->addFriendBtn->raise();

    ui->addFriendBtn->setEnabled(false);

    ui->addFriendBtn->setStyleSheet(
        "QPushButton { "
        "   background-color: rgb(52, 152, 219); color: rgb(255, 255, 255); border-radius: 5px; padding: 1px;"
        "} "
        "QPushButton:disabled { "
        "   background-color: rgb(32, 94, 135); color: #333;"
        "} "
        "QPushButton:hover { "
        "   background-color: rgb(46, 134, 193);"
        "} "
        "QPushButton:pressed { "
        "   background-color: rgb(40, 116, 167);"
        "}"
        );

    connect(ui->friendLineEdit, &QLineEdit::textChanged, this, [this]() {
        ui->addFriendBtn->setEnabled(!ui->friendLineEdit->text().isEmpty());
    });

    connect(ui->serversAndDmStackedWidget, &QStackedWidget::currentChanged, this, [this](int index) {
        if (ui->serversAndDmStackedWidget->widget(index) == ui->directMessagesPage) {
            updateDirectMessages();
        }
    });

    connect(ui->friendsPendingBtn, &QPushButton::clicked, this, &MainWindow::updatePendingFriends);

    connect(ui->friendsAllBtn, &QPushButton::clicked, this, &MainWindow::updateAllFriends);
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

void MainWindow::on_addFriendBtn_clicked()
{
    QString friendUsername = ui->friendLineEdit->text();
    sqlUser* sql = dynamic_cast<sqlUser*>(userSql);

    if (!sql || !user) {
        QMessageBox::warning(this, "Error", "User data error", QMessageBox::Ok);
        return;
    }

    if (!sql->getUserByUsername(friendUsername)) {
        QMessageBox::warning(this, "Information", "User wasn't found", QMessageBox::Ok);
        return;
    }

    QString status = sql->getFriendshipStatus(user->get_username(), friendUsername);

    if (status == "pending") {
        QMessageBox::information(this, "Information", "Friend request already sent.", QMessageBox::Ok);
        return;
    }

    if (status == "accepted") {
        QMessageBox::information(this, "Information", "You are already friends!", QMessageBox::Ok);
        return;
    }


    if (sql->sendFriendRequest(user->get_username(), friendUsername)) {
        QMessageBox::information(this, "Success", "Friend request sent!", QMessageBox::Ok);
    } else {
        QMessageBox::warning(this, "Error", "Friend request failed", QMessageBox::Ok);
    }
}

void MainWindow::updateDirectMessages()
{
    ui->directMessagesListWidget->clear();

    sqlUser* sql = dynamic_cast<sqlUser*>(userSql);
    if (!sql || !user) return;

    QVector<QString> directChats = sql->getDirectMessages(user->get_username());

    qDebug() << "Direct messages list for user:" << user->get_username();
    for (const auto& chat : directChats) {
        qDebug() << chat;
    }

    for (const QString& friendUsername : directChats) {
        QListWidgetItem* dmItem = new QListWidgetItem(friendUsername);
        ui->directMessagesListWidget->addItem(dmItem);

        connect(ui->directMessagesListWidget, &QListWidget::itemClicked, this, [this](QListWidgetItem* item) {
            //openChatWithFriend(item->text());
        });
    }
}

void MainWindow::updateAllFriends()
{
    if (!userSql->openDatabase())
    {
        qDebug() << "Database failed to open!";
    }

    ui->allFriendsListWidget->clear();
    sqlUser* sql = dynamic_cast<sqlUser*>(userSql);

    if (!sql || !user) return;

    QPair<QVector<QString>, int> allFriends = sql->getAllFriends(user->get_username());
    //allFriendsCountLbl
    for (const QString& friendUsername : allFriends.first) {
        QWidget* itemWidget = new QWidget();
        QHBoxLayout* layout = new QHBoxLayout(itemWidget);

        QLabel* nameLabel = new QLabel(friendUsername);
        QPushButton* messageButton = new QPushButton("Message");
        QPushButton* deleteButton = new QPushButton("Remove");

        layout->addWidget(nameLabel);
        layout->addWidget(messageButton);
        layout->addWidget(deleteButton);
        itemWidget->setLayout(layout);

        QListWidgetItem* item = new QListWidgetItem();
        item->setSizeHint(itemWidget->sizeHint());

        ui->allFriendsListWidget->addItem(item);
        ui->allFriendsListWidget->setItemWidget(item, itemWidget);

        connect(messageButton, &QPushButton::clicked, this, [this, friendUsername]() {
            //openChatWithFriend(friendUsername);
        });

        connect(deleteButton, &QPushButton::clicked, this, [this, friendUsername]() {
            removeFriend(friendUsername);
        });
    }
    ui->allFriendsCountLbl->setText(QString::number(allFriends.second));
}

void MainWindow::updatePendingFriends()
{

    ui->pendingListWidget->clear();
    sqlUser* sql = dynamic_cast<sqlUser*>(userSql);

    if (!sql || !user) return;

    QPair<QVector<QString>, int> pendingFriends = sql->getPendingRequests(user->get_username());

    for (const QString& sender : pendingFriends.first)
    {
        QWidget* itemWidget = new QWidget();
        QHBoxLayout* layout = new QHBoxLayout(itemWidget);

        QLabel* nameLabel = new QLabel(sender);
        QPushButton* acceptButton = new QPushButton("Accept");

        layout->addWidget(nameLabel);
        layout->addWidget(acceptButton);
        itemWidget->setLayout(layout);

        QListWidgetItem* item = new QListWidgetItem();
        item->setSizeHint(itemWidget->sizeHint());

        ui->pendingListWidget->addItem(item);
        ui->pendingListWidget->setItemWidget(item, itemWidget);


        connect(acceptButton, &QPushButton::clicked, this, [this, sender]() {
            acceptFriendRequest(sender);
        });
    }
    ui->pendingFriendsCountLbl->setText(QString::number(pendingFriends.second));
}

void MainWindow::acceptFriendRequest(const QString &sender)
{
    sqlUser* sql = dynamic_cast<sqlUser*>(userSql);
    if (!sql || !user)
    {
        QMessageBox::warning(this, "Error", "User data error", QMessageBox::Ok);
        return;
    }

    if (sql->acceptFriendRequest(sender, user->get_username()))
    {
        QMessageBox::information(this, "Success", "Friend request accepted!", QMessageBox::Ok);
        updatePendingFriends();
    } else {
        QMessageBox::warning(this, "Error", "Failed to accept friend request", QMessageBox::Ok);
    }
}

void MainWindow::removeFriend(const QString& friendUsername)
{
    sqlUser* sql = dynamic_cast<sqlUser*>(userSql);
    if (!sql || !user) {
        QMessageBox::warning(this, "Error", "User data error", QMessageBox::Ok);
        return;
    }

    if (QMessageBox::question(this, "Remove Friend", "Are you sure you want to remove " + friendUsername + " from your friends?",
                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
    {
        sql->deleteFriend(user->get_username(), friendUsername);
        updateAllFriends();
    }
}

