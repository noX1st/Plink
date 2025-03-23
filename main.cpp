#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    if (!QSqlDatabase::isDriverAvailable("QSQLITE")) {
        qDebug() << "SQLite driver is not available!";
        return -1;
    }
    MainWindow w;
    w.display();
    return a.exec();
}
