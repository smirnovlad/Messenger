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

ClientUI::ClientUI(QWidget *parent, Client *client)
    : QMainWindow(parent), client(client), ui(new Ui::ClientUI)
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
}

QLayoutItem *ClientUI::getActiveItem()
{
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
    connect(contactListWidget->ui->userList, SIGNAL(itemPressed(QListWidgetItem * )), this,
            SLOT(sendChatRequest(QListWidgetItem * )));

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
    connect(chatWidget, SIGNAL(editMessageRequest(QString, int32_t, QString, int32_t)),
            this->client->clientSocket, SLOT(sendEditMessageRequest(QString, int32_t, QString, int32_t)));
    ui->contentWidget->layout()->takeAt(0)->widget()->setVisible(false);
    ui->contentWidget->layout()->addWidget(chatWidget);
    this->client->clientSocket->sendChatRequest(name);
}

void ClientUI::handleRegistration(QStringList result)
{
    if (result[0] == "ILEN") {
        QMessageBox::warning(this, "Registration error", "The length of the username and password should be at least 4.");
    }
    else if (result[0] == "ALRD") {
        QMessageBox::warning(this, "Registration error", "A user with that name is already registered.");
    }
    else if (result[0] == "ICHR") {
        QMessageBox::warning(this, "Registration error", "You cannot use these characters in the username:\n" +
            result[1] + "\nYou cannot use these characters in the password:\n" +
            result[2] + "\nThe password must contain at least one non-whitespace character.");
    }
    else if (result[0] == "SCSS") {
        QMessageBox::information(this, "Registration success", "You registered a user with the username" + result[1]);
        this->setAuthorizationWidget();
    }
}

void ClientUI::handleAuthorization(QStringList result)
{
    if (result[0] == "ILEN") {
        QMessageBox::warning(this, "Authorization error", "The length of the username and password should be at least 4.");
    }
    else if (result[0] == "NFND" || result[0] == "IPSW") {
        QMessageBox::warning(this, "Authorization error", "Incorrect login or password.");
    }
    else if (result[0] == "ICHR") {
        QMessageBox::warning(this, "Authorization error", "You cannot use these characters in the username:\n" +
            result[1] + "\nYou cannot use these characters in the password:\n" +
            result[2] + "\nThe password must contain at least one non-whitespace character.");
    }
    else if (result[0] == "SCSS") {
        // qDebug() << "Generated token: " << result[1];
        client->userLogin = result[2];
        client->saveToken(result[1]);
        QMessageBox::information(this, "Authorization success", "You logged in as " + client->userLogin);
        this->setUserListWidget();
    }
}

void ClientUI::handleContactList(QString result, const QList<QPair<QString, QString> > &contactList)
{
    if (result == "SCSS") {
        QLayoutItem *activeItem = getActiveItem();
        static_cast<UserList *>(activeItem->widget())->setContactList(contactList);
    }
    else if (result == "ITKN") {
        this->handleIncorrectToken();
    }
}

void ClientUI::handleChat(QString result, QList<QString> &fromUser, QList<QString> &toUser,
                          QList<QString> &text, QList<QString> &timestamp,
                          QList<QString> &messageId)
{
    if (result == "SCSS") {
        QLayoutItem *activeItem = getActiveItem();
        static_cast<Chat *>(activeItem->widget())->setMessageList(fromUser,
                                                                  toUser,
                                                                  text,
                                                                  timestamp,
                                                                  messageId,
                                                                  client->userLogin);
    }
    else if (result == "ITKN") {
        this->handleIncorrectToken();
    }
}

void ClientUI::sendChatRequest(QListWidgetItem *contact)
{
    qDebug() << "Chat requested";
    QLayoutItem *activeItem = getActiveItem();
    QListWidget *userListWidget = static_cast<UserList *>(activeItem->widget())->ui->userList;
    User *userWidget = static_cast<User *>(userListWidget->itemWidget(contact));
    QString secondUser = userWidget->ui->loginLabel->text();
    userWidget->deleteLater();
    QCoreApplication::sendPostedEvents(userWidget, QEvent::DeferredDelete);
    emit getChatWidget(secondUser);
}

void ClientUI::handleSendMessage(QStringList result)
{
    QString sender = result[2];
    QString receiver = result[3];
    if (result[0] == "SCSS") {
        QString message = result[1];
        QString timestamp = result[4];
        int32_t messageId = result[5].toInt();
        QString clearEditLine = result[6];
        QLayoutItem *activeItem = getActiveItem();
        if (sender == client->userLogin) {
            Chat *chatWidget = dynamic_cast<Chat *>(activeItem->widget());
            if (chatWidget != nullptr &&
                chatWidget->ui->userNameLabel->text() == receiver) {
                chatWidget->sendMessage(message, timestamp, messageId, clearEditLine.toInt());
            }
            // QMessageBox::information(this, "Sending success", "The message was sent.");
        }
        else {
            Chat *chatWidget = dynamic_cast<Chat *>(activeItem->widget());
            if (chatWidget != nullptr &&
                chatWidget->ui->userNameLabel->text() == sender) {
                chatWidget->receiveMessage(message, timestamp, messageId);
                // QMessageBox::information(this, "Retrieving success", "The message was received.");
            }
        }
    }
    else if (result[0] == "ITKN") {
        handleIncorrectToken();
    }
    else {
        if (sender == client->userLogin) {
            QMessageBox::warning(this, "Error sending the message", "The message was not sent.");
        }
        else {
            QMessageBox::warning(this, "Error retrieving the message", "The message was not received.");
        }
    }
}

void ClientUI::handleIncorrectToken()
{
    QMessageBox::warning(this, "Request error", "Incorrect token. Please log in.");
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
        msgBox.setText("The connection to the server is not established. Would you like to try reconnecting?");
        QAbstractButton *pButtonExit = msgBox.addButton(tr("Exit"), QMessageBox::NoRole);
        QAbstractButton *pButtonTryAgain = msgBox.addButton(tr("Try reconnecting"), QMessageBox::YesRole);
        msgBox.exec();
        if (msgBox.clickedButton() == pButtonTryAgain) {
            client->clientSocket->connectSocketToHost();
        } else {
            QApplication::quit();
            return;
        }
    }
    if (request[0] == "CHAT") {
        client->clientSocket->sendChatRequest(request[1]);
    }
    else if (request[0] == "CTCS") {
        setUserListWidget();
    }
    else if (request[0] == "MSSG") {
        client->clientSocket->sendSendMessageRequest(request[1], request[2]);
    }
    else if (request[0] == "EMSG") {
        client->clientSocket->sendEditMessageRequest(request[1], request[2].toInt(),
                                                     request[3], request[4].toInt());
    }
}

void ClientUI::handleEditMessage(QStringList result)
{
    QString sender = result[1];
    QString receiver = result[2];
    if (result[0] == "SCSS") {
        QString editedMessage = result[3];
        int32_t messageChatIndex = result[4].toInt();
        QLayoutItem *activeItem = getActiveItem();
        Chat *chatWidget = static_cast<Chat *>(activeItem->widget());
        if (sender == client->userLogin &&
            chatWidget->ui->userNameLabel->text() == receiver ||
            receiver == client->userLogin &&
                chatWidget->ui->userNameLabel->text() == sender) {
            qDebug() << "Edit message";
            chatWidget->editMessage(messageChatIndex, editedMessage);
        }
    }
    else if (result[0] == "ITKN") {
        handleIncorrectToken();
    }
    else {
        if (sender == client->userLogin) {
            QMessageBox::warning(this, "Error during modification", "The message has not been changed.");
        }
    }
}