#include "clientui.h"
#include "./ui_clientui.h"
#include "authorization.h"
#include "./ui_authorization.h"
#include "registration.h"
#include "./ui_registration.h"

#include <QDebug>
#include <QMessageBox>

ClientUI::ClientUI(QWidget *parent, Client* client)
    : QMainWindow(parent)
    , client(client)
    , ui(new Ui::ClientUI)
{
    ui->setupUi(this);
    this->setAuthorizationWidget();
    this->show();
}

ClientUI::~ClientUI()
{
    delete ui;
}

void ClientUI::setAuthorizationWidget()
{
    qDebug() << "Switched to log in";
    this->clearLayout();
    Authorization *authorizationWidget = new Authorization(this);

    connect(authorizationWidget->ui->signUpButton, SIGNAL(clicked()), this,
            SLOT(setRegistrationWidget()));
    connect(authorizationWidget, SIGNAL(logInRequest(QString, QString)), this,
            SLOT(log_in(QString, QString)));

    ui->contentWidget->layout()->addWidget(authorizationWidget);
}

void ClientUI::setRegistrationWidget()
{
    qDebug() << "Switched to sign up";
    this->clearLayout();
    Registration *registrationWidget = new Registration(this);

    connect(registrationWidget->ui->backButton, SIGNAL(clicked()), this,
            SLOT(setAuthorizationWidget()));
    connect(registrationWidget, SIGNAL(signUpRequest(QString, QString)), this->client->clientSocket,
            SLOT(sendSignUpRequest(QString, QString)));

    ui->contentWidget->layout()->addWidget(registrationWidget);
}

void ClientUI::clearLayout()
{
    QLayoutItem *activeItem = ui->contentWidget->layout()->takeAt(0);
    if (activeItem != NULL) {
        delete activeItem->widget();
        delete activeItem;
    }
}

void ClientUI::log_in(QString login, QString password)
{
    int ret = this->client->logIn(login, password);
}

void ClientUI::handleRegistration(QString message)
{
    if (message == "ILEN") {
        QMessageBox::warning(this, "Registration failed", "Login and password length must be at least 4");
    } else if (message == "ALRD") {
        QMessageBox::warning(this, "Registration failed", "You have been already registered");
    } else if (message == "SCSS") {
        QMessageBox::information(this, "Registration success", "You are registered");
        this->setAuthorizationWidget();
    }
}
