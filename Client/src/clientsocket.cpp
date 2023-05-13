#include "inc/clientsocket.h"

ClientSocket::ClientSocket(QObject *parent, Client* client)
    : QObject(parent)
    , client(client)
    , tcpSocket(new QTcpSocket(this))
{
    connectSocketToHost();

    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(getResponse()));
}

void ClientSocket::connectSocketToHost()
{
    QString hostname = "127.0.0.1";
    quint32 port = 55155;
//    tcpSocket->abort();
    tcpSocket->connectToHost(hostname, port);
}

QStringList ClientSocket::requestSeparation(QString text, QString sep)
{
    QString outStr =  text;
    QStringList list = outStr.split(sep);

    return list;
}

void ClientSocket::getResponse()
{
    QString packetType;
    QString message;

    packetType = tcpSocket->read(4);
    message = tcpSocket->readAll();

    QStringList commandList;
    QString fromname;

    enum class COMMAND { NONE,
                         REGISTRATION_RESPONSE,
                         AUTHORIZATION_RESPONSE,
                         CONTACT_LIST_RESPONSE,
                         MESSAGE_LIST_RESPONSE,
                         SEND_MESSAGE_RESPONSE,
                         LOG_OUT_RESPONSE
                       };
    COMMAND command = COMMAND::NONE;

    if (packetType == "REGI") {
        command = COMMAND::REGISTRATION_RESPONSE;
    } else if (packetType == "AUTH") {
        command = COMMAND::AUTHORIZATION_RESPONSE;
    } else if (packetType == "CTCS") {
        command = COMMAND::CONTACT_LIST_RESPONSE;
    } else if (packetType == "CHAT") {
        command = COMMAND::MESSAGE_LIST_RESPONSE;
    } else if (packetType == "MSSG") {
        command = COMMAND::SEND_MESSAGE_RESPONSE;
    } else if (packetType == "LOGO") {
        command = COMMAND::LOG_OUT_RESPONSE;
    }

    switch (command)
    {
        case COMMAND::REGISTRATION_RESPONSE:
        {
            QStringList result = requestSeparation(message, " /s ");
            client->clientUI->handleRegistration(result);
            break;
        }

        case COMMAND::AUTHORIZATION_RESPONSE:
        {
            QStringList result = requestSeparation(message, " /s ");
            client->clientUI->handleAuthorization(result);
            break;
        }

        case COMMAND::CONTACT_LIST_RESPONSE:
        {
            QList< QPair<QString, QString> > contactList;
            QStringList splitWords = requestSeparation(message, " /n ");
            QString result = splitWords[0];
            // if result is not "SCSS", splitWords[1] = ""
            // is result is "SCSS", splitWords[1] also can be empty
            if (splitWords[1] != "") {
              for (uint32_t i = 1; i < splitWords.size(); ++i) {
                QStringList pair = requestSeparation(splitWords[i], " /s ");
                contactList.push_back({pair[0], pair[1]});
              }
            }
            client->clientUI->handleContactList(result, contactList);
            break;
        }

        case COMMAND::MESSAGE_LIST_RESPONSE:
        {
            QList<QString> fromUser, toUser, text, timestamp;
            qDebug() << "Message list response: " << message;
            QStringList splitWords = requestSeparation(message, " /n ");
            QString result = splitWords[0];
            if (splitWords[1] != "") {
                for (uint32_t i = 1; i < splitWords.size(); ++i) {
                    QStringList tuple = requestSeparation(splitWords[i], " /s ");
                    fromUser.push_back(tuple[0]);
                    toUser.push_back(tuple[1]);
                    text.push_back(tuple[2]);
                    timestamp.push_back(tuple[3]);
                }
            }
            client->clientUI->handleChat(result, fromUser, toUser, text, timestamp);
            break;
        }

        case COMMAND::SEND_MESSAGE_RESPONSE:
        {
            qDebug() << "Send message response: " << message;
            QStringList splitWords = requestSeparation(message, " /s ");
            client->clientUI->handleSendMessage(splitWords);
            break;
        }

        case COMMAND::LOG_OUT_RESPONSE:
        {
            qDebug() << "Log out response: " << message;
            client->clientUI->handleLogOut(message);
            break;
        }
    }
}

void ClientSocket::sendSignUpRequest(QString login, QString password)
{
    qDebug() << "Sign Up requested. Login: " << login << ", password: " << password;
    QString request = "REGI";

    if (tcpSocket->state() != QAbstractSocket::ConnectedState) {
        QStringList userRequest;
        userRequest.append(request);
        userRequest.append(login);
        userRequest.append(password);
        client->clientUI->handleConnectionError(userRequest);
    } else {
        request.append(QString("%1 /s %2").arg(login).arg(password));
        tcpSocket->write(request.toUtf8());
    }
}

void ClientSocket::sendLogInRequest(QString login, QString password)
{
    qDebug() << "Log in requested. Login: " << login << ", password: " << password;
    QString request = "AUTH";

    if (tcpSocket->state() != QAbstractSocket::ConnectedState) {
        QStringList userRequest;
        userRequest.append(request);
        userRequest.append(login);
        userRequest.append(password);
        client->clientUI->handleConnectionError(userRequest);
    } else {
        request.append(QString("%1 /s %2").arg(login).arg(password));
        tcpSocket->write(request.toUtf8());
    }
}

void ClientSocket::sendContactListRequest()
{
    qDebug() << "Contact list requested. Username: " << client->userLogin;
    QString request = "CTCS";

    if (tcpSocket->state() != QAbstractSocket::ConnectedState) {
        QStringList userRequest;
        userRequest.append(request);
        client->clientUI->handleConnectionError(userRequest);
    } else {
        request.append(QString("%1").arg(client->getToken()));
        tcpSocket->write(request.toUtf8());
    }
}

void ClientSocket::sendChatRequest(QString secondUser)
{
    qDebug() << "Chat requested. First user: " << client->userLogin <<
                ", second user: " << secondUser;
    QString request = "CHAT";

    if (tcpSocket->state() != QAbstractSocket::ConnectedState) {
        QStringList userRequest;
        userRequest.append(request);
        userRequest.append(secondUser);
        client->clientUI->handleConnectionError(userRequest);
    } else {
        request.append(QString("%1 /s %2 /s %3").arg(client->userLogin).arg(secondUser)
                                            .arg(client->getToken()));
        tcpSocket->write(request.toUtf8());
//        qDebug() << "Written in socket";
    }
}

void ClientSocket::sendSendMessageRequest(QString secondUser, QString message)
{
    qDebug() << "Sending message requested. First user: " << client->userLogin <<
                ", second user: " << secondUser << ". Message: " << message;
    QString request = "MSSG";

    if (tcpSocket->state() != QAbstractSocket::ConnectedState) {
        QStringList userRequest;
        userRequest.append(request);
        userRequest.append(secondUser);
        userRequest.append(message);
        client->clientUI->handleConnectionError(userRequest);
    } else {
        request.append(QString("%1 /s %2 /s %3 /s %4").arg(client->userLogin).arg(secondUser)
                                                        .arg(message).arg(client->getToken()));
        tcpSocket->write(request.toUtf8());
    }
}

void ClientSocket::sendLogOutRequest()
{
    qDebug() << "Log out requested from user: " << client->userLogin;
    QString request = "LOGO"; // log out

    if (tcpSocket->state() != QAbstractSocket::ConnectedState) {
        QStringList userRequest;
        userRequest.append(request);
        client->clientUI->handleConnectionError(userRequest);
    } else {
        tcpSocket->write(request.toUtf8());
    }
}
