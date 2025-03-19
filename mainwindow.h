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

protected:

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void display();
    void signIn();
    void signUp();

    void startGame();
    void singleGame();
    void multiplayerGame();
    void settings();
    void statistic();

    void exit();
    void exitApp();

    void on_addPushButton_clicked();

    void setupEventFilter();
    bool eventFilter(QObject *obj, QEvent *event);

    void updateDirectMessages();

    void updateAllFriends();

    void updatePendingFriends();

private slots:

    void on_addFriendBtn_clicked();

    void acceptFriendRequest(const QString& senderUsername);

    void removeFriend(const QString& friendUsername);

private:
    QButtonGroup *serversAndDmButtonGroup;
    QButtonGroup *friendsButtonGroup;

    sign_in sign_in;
    sign_up registration;
    User *user;
    Ui::MainWindow *ui;
    sqlUser* userSql;

    QLineEdit *friendLineEdit;
    QPushButton *addFriendBtn;
    QHBoxLayout *friendLayout;
};
#endif // MAINWINDOW_H
