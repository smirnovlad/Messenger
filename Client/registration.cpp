#include "registration.h"
#include "ui_registration.h"

#include <QDebug>
#include <QMessageBox>

Registration::Registration(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Registration)
{
    ui->setupUi(this);
    this->setWindowTitle("Registration");

    connect(ui->signUpButton, SIGNAL(clicked()), this, SLOT(sign_up()));
}

Registration::~Registration()
{
    delete ui;
}

void Registration::sign_up()
{
    QString login = ui->loginLabel->text();
    QString password_1 = ui->passwordLabel_1->text();
    QString password_2 = ui->passwordLabel_2->text();
    if (password_1 == password_2) {
        emit signUpRequest(login, password_1);
    } else {
        QMessageBox::warning(this, "Registration failed", "Password mismatch");
    }
}
