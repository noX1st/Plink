#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include <QButtonGroup>
#include <qboxlayout.h>
#include <qlineedit.h>
#include "sign_in.h"
#include "sign_up.h"
#include "sql.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public slots:
    void changeWindow();

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void signIn();
    void signUp();
    void display();
    void exitApp();

private slots:
    void on_addPushButtonBtn_clicked();
    void on_addFriendBtn_clicked();
    void updateDirectMessages();
    void updateAllFriends();
    void updatePendingFriends();
    void acceptFriendRequest(const QString& senderUsername);
    void removeFriend(const QString& friendUsername);

private:
    void setupEventFilter();
    bool eventFilter(QObject *obj, QEvent *event);

    void setupButtonStyle(QPushButton *button);
    void setupAddFriendButton();
    void sendMessage();
    void loadMessages(const QString& user1, const QString& user2);
    void openChat(const QString& friendUsername);

    QButtonGroup *serversAndDmButtonGroup;
    QButtonGroup *friendsButtonGroup;

    std::unique_ptr<sign_in> signInWindow;
    std::unique_ptr<sign_up> signUpWindow;
    std::unique_ptr<User> user;
    Ui::MainWindow *ui;
    std::unique_ptr<UserDatabase> userDatabase;
    QWidget *dmPage;

    QLineEdit *friendLineEdit;
    QPushButton *addFriendBtn;
    QHBoxLayout *friendLayout;

    QString currentMessageReceiver;
};
#endif // MAINWINDOW_H
