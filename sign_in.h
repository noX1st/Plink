#ifndef SIGN_IN_H
#define SIGN_IN_H

#include <QDialog>

namespace Ui {
class sign_in;
}

class sign_in : public QDialog
{
    Q_OBJECT
protected:
    void closeEvent(QCloseEvent *event) override;

public:
    explicit sign_in(QWidget *parent = nullptr);
    ~sign_in();
    QString get_email() const;
    QString get_password() const;


private slots:
    void on_signInBtn_clicked();

    void on_signUpBtn_clicked();

signals:
    void signInBtn_clicked();

    void signUpBtn_clicked();

private:
    Ui::sign_in *ui;
};

#endif // SIGN_IN_H
