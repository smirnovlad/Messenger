#include "inc/clientui.h"
#include "./ui_clientui.h"
#include "inc/authorization.h"
#include "./ui_authorization.h"
#include "inc/registration.h"
#include "./ui_registration.h"
#include "inc/userlist.h"
#include "./ui_userlist.h"
#include "inc/user.h"
#include "./ui_user.h"
#include "inc/chat.h"
#include "./ui_chat.h"

#include <QDebug>
#include <QMessageBox>
#include <QDir>
#include <QTextStream>

ClientUI::ClientUI(QWidget *parent, Client* client)
    : QMainWindow(parent)
    , client(client)
    , ui(new Ui::ClientUI)
{
    ui->setupUi(this);
    this->setAuthorizationWidget();
    this->show();

    connect(this, SIGNAL(getChatWidget(QString)), this, SLOT(setChatWidget(QString)),
            Qt::QueuedConnection);

}

ClientUI::~ClientUI()
{
    delete ui;
}

void ClientUI::clearLayout()
{
    QLayoutItem *activeItem = ui->contentWidget->layout()->itemAt(0);
    if (activeItem != NULL) {
        delete activeItem->widget();
        // delete activeItem;
    }
//    qDebug() << "Layout items count: " << ui->contentWidget->layout()->count();
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

    connect(contactListWidget->ui->logoutButton, SIGNAL(clicked()), this->client->clientSocket,
            SLOT(sendLogOutRequest()));
    connect(contactListWidget->ui->userList, SIGNAL(itemPressed(QListWidgetItem*)), this,
            SLOT(sendChatRequest(QListWidgetItem*)));

    ui->contentWidget->layout()->addWidget(contactListWidget);
    this->client->clientSocket->sendContactListRequest();
}

void ClientUI::setChatWidget(QString name)
{
    qDebug() << "Switched to chat";
    //    this->clearLayout();
    //    ui->setupUi(this);
    Chat *chatWidget = new Chat(this);
    chatWidget->ui->userNameLabel->setText(name);
    connect(chatWidget->ui->backButton, SIGNAL(clicked()), this,
              SLOT(setUserListWidget()));
    connect(chatWidget, SIGNAL(sendMessageRequest(QString, QString)), this->client->clientSocket,
              SLOT(sendSendMessageRequest(QString, QString)));
    ui->contentWidget->layout()->takeAt(0)->widget()->setVisible(false);
    ui->contentWidget->layout()->addWidget(chatWidget);
    this->client->clientSocket->sendChatRequest(name);
}

void ClientUI::handleRegistration(QStringList result)
{
    if (result[0] == "ILEN") {
        QMessageBox::warning(this, "Registration failed", "Login and password length must be at least 4.");
    } else if (result[0] == "ALRD") {
        QMessageBox::warning(this, "Registration failed", "You have been already registered.");
    } else if (result[0] == "ISYM") {
        QMessageBox::warning(this, "Registration failed", "You can not use these symbols for login:\n" +
                             result[1] + "\nYou can not youse these symbols for password:\n" +
                            result[2] + "\nPassword must contain at least one non-space symbol.");
    } else if (result[0] == "SCSS") {
        QMessageBox::information(this, "Registration success", "You are registered. Your login: " + result[1]);
        this->setAuthorizationWidget();
    }
}

void ClientUI::handleAuthorization(QStringList result)
{
    if (result[0] == "ILEN") {
        QMessageBox::warning(this, "Authorization failed", "Login and password length must be at least 4.");
    } else if (result[0] == "NFND" || result[0] == "IPSW") {
        QMessageBox::warning(this, "Authorization failed", "Incorrect login or password.");
    } else if (result[0] == "ISYM") {
        QMessageBox::warning(this, "Authorization failed", "You can not use these symbols for login:\n" +
                             result[1] + "\nYou can not youse these symbols for password:\n" +
                            result[2] + "\nPassword must contain at least one non-space symbol.");
    } else if (result[0] == "SCSS") {
        // qDebug() << "Generated token: " << result[1];
        client->userLogin = result[2];
        client->saveToken(result[1]);
        QMessageBox::information(this, "Authorization success", "You are logged in as " + client->userLogin);
        this->setUserListWidget();
    }
}

void ClientUI::handleContactList(QString result, const QList< QPair<QString, QString> >& contactList)
{
    if (result == "SCSS") {
        QLayoutItem *activeItem = getActiveItem();
        static_cast<UserList*>(activeItem->widget())->setContactList(contactList);
    } else if (result == "ITKN") {
        this->handleIncorrectToken();
    }
}

void ClientUI::handleChat(QString result, QList<QString> &fromUser, QList<QString> &toUser,
                          QList<QString> &text, QList<QString> &timestamp)
{
    if (result == "SCSS") {
        QLayoutItem *activeItem = getActiveItem();
        static_cast<Chat*>(activeItem->widget())->setMessageList(fromUser,
                                                                 toUser,
                                                                 text,
                                                                 timestamp,
                                                                 client->userLogin);
    } else if (result == "ITKN") {
        this->handleIncorrectToken();
    }
}

void ClientUI::sendChatRequest(QListWidgetItem* contact)
{
    qDebug() << "Chat requested";
    QLayoutItem *activeItem = getActiveItem();
    QListWidget *userListWidget = static_cast<UserList*>(activeItem->widget())->ui->userList;
    User *userWidget = static_cast<User*>(userListWidget->itemWidget(contact));
    QString secondUser = userWidget->ui->loginLabel->text();
    userWidget->deleteLater();
    QCoreApplication::sendPostedEvents(userWidget, QEvent::DeferredDelete);
    this->setChatWidget(secondUser);
}

void ClientUI::handleSendMessage(QStringList result)
{
    QString sender = result[2];
    QString receiver = result[3];
    if (result[0] == "SCSS") {
        QString message = result[1];
        QString timestamp = result[4];
        QString clearEditLine = result[5];
        QLayoutItem *activeItem = getActiveItem();
        if (sender == client->userLogin) {
            // maybe use dynamic_cast and check if it is nullptr in case
            // of user closed chat "very fast"
            Chat* chatWidget = static_cast<Chat*>(activeItem->widget());
            if (chatWidget->ui->userNameLabel->text() == receiver) {
              chatWidget->sendMessage(message, timestamp, clearEditLine.toInt());
            }
            // one user can be logged in from different sockets simultaneously
            // so we need to pass message
            // QMessageBox::information(this, "Sending success", "Message was sent.");
        } else {
            Chat* chatWidget = dynamic_cast<Chat*>(activeItem->widget());
            if (chatWidget != nullptr && chatWidget->ui->userNameLabel->text() == sender) {
                chatWidget->receiveMessage(message, timestamp);
                // QMessageBox::information(this, "Receiving success", "Message was received.");
            }
        }
    } else if (result[0] == "ITKN") {
        handleIncorrectToken();
    } else {
        if (sender == client->userLogin) {
            QMessageBox::warning(this, "Sending failed", "Message was not sent.");
        } else {
            QMessageBox::warning(this, "Receiving failed", "Message was not received.");
        }
    }
}

void ClientUI::handleIncorrectToken()
{
    QMessageBox::warning(this, "Request failed", "Incorrect token. Please log in.");
    this->client->clientSocket->sendLogOutRequest();
}

void ClientUI::handleLogOut(QString result)
{
    if (result == "SCSS") {
        client->userLogin = "";
        setAuthorizationWidget();
    }
}

void ClientUI::handleConnectionError(QStringList request)
{
    while (client->clientSocket->tcpSocket->state() != QAbstractSocket::ConnectedState) {
        QMessageBox msgBox;
        msgBox.setText("You are not connected to server. Would you like to try to connect?");
        QAbstractButton *pButtonTryAgain = msgBox.addButton(tr("Try to connect"), QMessageBox::YesRole);
        msgBox.exec();
        if (msgBox.clickedButton() == pButtonTryAgain) {
            client->clientSocket->connectSocketToHost();
        }
    }
    if (request[0] == "CHAT") {
        client->clientSocket->sendChatRequest(request[1]);
    } else if (request[0] == "CTCS") {
        setUserListWidget();
    } else if (request[0] == "MSSG") {
        client->clientSocket->sendSendMessageRequest(request[1], request[2]);
    }
}
