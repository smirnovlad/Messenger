#ifndef AUTHORIZATION_H
#define AUTHORIZATION_H

#include <QDialog>
#include <QKeyEvent>
#include <QTcpSocket>
#include <QTime>

#include "registration.h"

namespace Ui {
class Authorization;
}

class Authorization : public QWidget
{
    Q_OBJECT

public:
    explicit Authorization(QWidget *parent = nullptr);
    ~Authorization();

public:
    Ui::Authorization *ui;

private slots:
    void log_in();

signals:
    void logInRequest(QString login, QString password);
};

#endif // AUTHORIZATION_H
