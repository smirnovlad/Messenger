#include "clientui.h"
#include "./ui_clientui.h"
#include "authorization.h"
#include "./ui_authorization.h"
#include "registration.h"
#include "./ui_registration.h"
#include "userlist.h"
#include "./ui_userlist.h"

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

void ClientUI::clearLayout()
{
    QLayoutItem *activeItem = ui->contentWidget->layout()->takeAt(0);
    if (activeItem != NULL) {
        delete activeItem->widget();
        delete activeItem;
    }
}

QLayoutItem* ClientUI::getActiveItem() {
    return ui->contentWidget->layout()->itemAt(0);
}

void ClientUI::setAuthorizationWidget()
{
    qDebug() << "Switched to log in";
    this->clearLayout();
    Authorization *authorizationWidget = new Authorization(this);

    connect(authorizationWidget->ui->signUpButton, SIGNAL(clicked()), this,
            SLOT(setRegistrationWidget()));
    connect(authorizationWidget, SIGNAL(logInRequest(QString, QString)), this->client->clientSocket,
            SLOT(sendLogInRequest(QString, QString)));

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

void ClientUI::setUserListWidget()
{
    qDebug() << "Switched to user list";
    this->clearLayout();
    UserList *contactListWidget = new UserList(this);

    connect(contactListWidget->ui->backButton, SIGNAL(clicked()), this,
            SLOT(setAuthorizationWidget()));

    ui->contentWidget->layout()->addWidget(contactListWidget);
}

void ClientUI::handleRegistration(QString result)
{
    if (result == "ILEN") {
        QMessageBox::warning(this, "Registration failed", "Login and password length must be at least 4");
    } else if (result == "ALRD") {
        QMessageBox::warning(this, "Registration failed", "You have been already registered");
    } else if (result == "SCSS") {
        QMessageBox::information(this, "Registration success", "You are registered");
        this->setAuthorizationWidget();
    }
}

void ClientUI::handleAuthorization(QString result)
{
    if (result == "ILEN") {
        QMessageBox::warning(this, "Authorization failed", "Login and password length must be at least 4");
    } else if (result == "NFND" || result == "IPSW") {
        QMessageBox::warning(this, "Authorization failed", "Incorrect login or password");
    } else if (result == "SCSS") {
        QMessageBox::information(this, "Authorization success", "You are loged in");
        this->setUserListWidget();
        this->client->clientSocket->sendContactListRequest("", "");
    }
}

void ClientUI::handleContactList(const QList< QPair<QString, QString> >& result)
{
    QLayoutItem *activeItem = getActiveItem();
    qDebug() << "count: " << ui->contentWidget->layout()->count();
    static_cast<UserList*>(activeItem->widget())->setContactList(result);
}
