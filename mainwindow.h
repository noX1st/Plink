#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include <QButtonGroup>
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

    //void BackBtn_clicked();

private:
    QButtonGroup *friendsButtonGroup;

    sign_in sign_in;
    sign_up registration;
    User *user;
    Ui::MainWindow *ui;
    sql* userSql;
};
#endif // MAINWINDOW_H
