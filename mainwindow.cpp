#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "sign_in.h"
#include "sql.h"
#include "QMessageBox"
#include <QMouseEvent>
#include <QSettings>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , signInWindow(std::make_unique<sign_in>(this))
    , signUpWindow(std::make_unique<sign_up>(this))
    , user(nullptr)
    , ui(new Ui::MainWindow)
    , userDatabase(std::make_unique<UserDatabase>(SqlConstants::MAIN_CONNECTION, SqlConstants::DB_NAME))

{
    ui->setupUi(this);

    setupEventFilter();

    userDatabase->createTable();
    userDatabase->createFriendsTable();

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

    connect(signInWindow.get(), &sign_in::signInBtn_clicked, this, &MainWindow::signIn);
    connect(signUpWindow.get(), &sign_up::signUpBtnClicked, this, &MainWindow::signUp);
    connect(signUpWindow.get(), &sign_up::finished, signInWindow.get(), &sign_in::show);
    connect(signInWindow.get(), &sign_in::signUpBtn_clicked, this, &MainWindow::changeWindow);

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

    connect(ui->friendsBtn, &QPushButton::clicked, this, [this]() {
        ui->dmAndFriends->setCurrentWidget(ui->friendsPage);
    });

    connect(ui->messageLineEdit, &QLineEdit::returnPressed, this, &MainWindow::sendMessage);

    setupAddFriendButton();
}

MainWindow::~MainWindow()
{
    delete ui;
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

void MainWindow::signIn()
{
    user = std::make_unique<User>();
    if (!userDatabase->getUser(*user, SqlConstants::USER_TABLE_NAME, signInWindow->get_email(), signInWindow->get_password()))
    {
        QMessageBox::warning(this, "warning", "The entered data is incorrect", QMessageBox::Ok);
        return;
    }
    QMessageBox::information(this, "information", "Sign in was successful", QMessageBox::Ok);
    this->show();
    signInWindow->hide();

    updateDirectMessages();
    updatePendingFriends();
}

void MainWindow::signUp()
{
    if (signUpWindow->get_username().isEmpty() || signUpWindow->get_email().isEmpty() || signUpWindow->get_password().isEmpty() || signUpWindow->get_confirm().isEmpty())
    {
        QMessageBox::warning(this, "Warning", "Please fill in all fields.", QMessageBox::Ok);
        return;
    }
    if (signUpWindow->get_email().contains('@'))
    {
        QMessageBox::warning(this, "Warning", "Please enter a valid email", QMessageBox::Ok);
        return;
    }
    if (signUpWindow->get_password() != signUpWindow->get_confirm())
    {
        QMessageBox MSG(
            QMessageBox::Warning, "warning", "passwords doesn't match", QMessageBox::Ok);
        MSG.exec();
        return;
    }
    user = std::make_unique<User>();
    if (userDatabase->getUser(*user, SqlConstants::USER_TABLE_NAME, signUpWindow->get_email(), signUpWindow->get_password()))
    {
        QMessageBox::warning(this, "Warning", "This user already exists", QMessageBox::Ok);
        return;
    }
    userDatabase->insertUser(*user);
    QMessageBox MSG(
        QMessageBox::Information, "information", "Sign up was succesfull", QMessageBox::Ok);

    MSG.exec();
    signInWindow->show();
    signUpWindow->hide();
}

void MainWindow::display()
{
    signInWindow->show();
}

void MainWindow::changeWindow()
{
    signUpWindow->show();
    signInWindow->hide();
}

void MainWindow::exitApp()
{
    signInWindow->close();
}

//ui
void MainWindow::setupButtonStyle(QPushButton *button) {
    button->setStyleSheet("QPushButton"
                          "{ background-color: #3498db; border: none; border-radius: 15px; padding: 3px; min-width: 30px; min-height: 30px; max-width: 30px; max-height: 30px; }"
                          "QPushButton:hover"
                          "{ background-color: #2980b9; }"
                          "QPushButton:pressed"
                          "{ background-color: #1c5985; }");
}

void MainWindow::setupAddFriendButton() {
    ui->addFriendBtn->setParent(ui->friendLineEdit);
    ui->addFriendBtn->move(ui->friendLineEdit->width() - ui->addFriendBtn->width() - 2,
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
}

//buttons
void MainWindow::on_addPushButtonBtn_clicked()
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

void MainWindow::on_addFriendBtn_clicked()
{
    QString friendUsername = ui->friendLineEdit->text();

    if (!userDatabase || !user) {
        QMessageBox::warning(this, "Error", "User data error", QMessageBox::Ok);
        return;
    }

    if (!userDatabase->getUserByUsername(friendUsername)) {
        QMessageBox::warning(this, "Information", "User wasn't found", QMessageBox::Ok);
        return;
    }

    QString status = userDatabase->getFriendshipStatus(user->get_username(), friendUsername);

    if (status == "pending") {
        QMessageBox::information(this, "Information", "Friend request already sent.", QMessageBox::Ok);
        return;
    }

    if (status == "accepted") {
        QMessageBox::information(this, "Information", "You are already friends!", QMessageBox::Ok);
        return;
    }


    if (userDatabase->sendFriendRequest(user->get_username(), friendUsername)) {
        QMessageBox::information(this, "Success", "Friend request sent!", QMessageBox::Ok);
    } else {
        QMessageBox::warning(this, "Error", "Friend request failed", QMessageBox::Ok);
    }
}

//messages
void MainWindow::sendMessage()
{
    QString message = ui->messageLineEdit->text();
    QString receiver = currentMessageReceiver;

    if (message.isEmpty() || receiver.isEmpty()) {
        qDebug() << "Message or receiver is empty!";
        return;
    }

    qDebug() << "Sending message to" << receiver << ": " << message;

    if (userDatabase) {
        userDatabase->sendMessage(user->get_username(), receiver, message);
    }

    ui->messageLineEdit->clear();
    loadMessages(user->get_username(), receiver);
}

void MainWindow::loadMessages(const QString& user1, const QString& user2) {
    QVector<QPair<QString, QString>> messages = userDatabase->getMessages(user1, user2);

    ui->messageList->clear();

    for (const auto& message : messages) {
        QListWidgetItem* item = new QListWidgetItem();
        item->setText(message.first + ": " + message.second);
        ui->messageList->addItem(item);
    }
}

void MainWindow::openChat(const QString& friendUsername)
{
    qDebug() << "Open chat with:" << friendUsername;
    ui->dmAndFriends->setCurrentWidget(ui->dmPage);
    currentMessageReceiver = friendUsername;

    loadMessages(user->get_username(), friendUsername);

    updateDirectMessages();
    ui->messageReceiverLbl->setText(currentMessageReceiver);
}

void MainWindow::updateDirectMessages()
{
    ui->directMessagesListWidget->clear();

    if (!userDatabase || !user) return;

    QVector<QString> directChats = userDatabase->getDirectMessages(user->get_username());
    for (const auto& chat : directChats) {
        qDebug() << chat;
    }

    ui->accountBtn->setText(user->get_username());

    for (const QString& friendUsername : directChats) {
        QWidget* itemWidget = new QWidget();
        QHBoxLayout* layout = new QHBoxLayout(itemWidget);

        QLabel* nameLabel = new QLabel(friendUsername);
        QPushButton* messageButton = new QPushButton("Message");

        messageButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

        layout->addWidget(nameLabel);
        layout->addStretch();
        layout->addWidget(messageButton);
        itemWidget->setLayout(layout);

        QListWidgetItem* item = new QListWidgetItem();
        item->setSizeHint(itemWidget->sizeHint());

        ui->directMessagesListWidget->addItem(item);
        ui->directMessagesListWidget->setItemWidget(item, itemWidget);

        connect(messageButton, &QPushButton::clicked, this, [=]() {
            openChat(friendUsername);
        });
    }
}

//Friends
void MainWindow::updateAllFriends()
{
    if (!userDatabase->openDatabase())
    {
        qDebug() << "Database failed to open!";
    }

    ui->allFriendsListWidget->clear();

    if (!userDatabase || !user) return;

    QPair<QVector<QString>, int> allFriends = userDatabase->getAllFriends(user->get_username());
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
            openChat(friendUsername);
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

    if (!userDatabase || !user) return;

    QPair<QVector<QString>, int> pendingFriends = userDatabase->getPendingRequests(user->get_username());

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
    if (!(pendingFriends.second < 1))
    {
        ui->friendsPendingBtn->setText("Pending " + (QString::number(pendingFriends.second)));
    }
}

void MainWindow::acceptFriendRequest(const QString &sender)
{
    if (!userDatabase || !user)
    {
        QMessageBox::warning(this, "Error", "User data error", QMessageBox::Ok);
        return;
    }

    if (userDatabase->acceptFriendRequest(sender, user->get_username()))
    {
        QMessageBox::information(this, "Success", "Friend request accepted!", QMessageBox::Ok);
        updatePendingFriends();
    } else {
        QMessageBox::warning(this, "Error", "Failed to accept friend request", QMessageBox::Ok);
    }
    ui->friendsPendingBtn->setText("Pending");
    updateDirectMessages();
}

void MainWindow::removeFriend(const QString& friendUsername)
{
    if (!userDatabase || !user) {
        QMessageBox::warning(this, "Error", "User data error", QMessageBox::Ok);
        return;
    }

    if (QMessageBox::question(this, "Remove Friend", "Are you sure you want to remove " + friendUsername + " from your friends?",
                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
    {
        userDatabase->deleteFriend(user->get_username(), friendUsername);
        updateAllFriends();
        updateDirectMessages();
    }
}
