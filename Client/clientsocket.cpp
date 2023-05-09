#include "clientsocket.h"

ClientSocket::ClientSocket(QObject *parent, Client* client)
    : QObject(parent)
    , client(client)
    , tcpSocket(new QTcpSocket(this))
{
    QString hostname = "127.0.0.1";
    quint32 port = 55155;
    tcpSocket->abort();
    tcpSocket->connectToHost(hostname, port);

    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(getResponse()));
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
                         SEND_MESSAGE_RESPONSE
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
    }

    switch (command)
    {
        case COMMAND::REGISTRATION_RESPONSE:
        {
            client->clientUI->handleRegistration(message);
            break;
        }

        case COMMAND::AUTHORIZATION_RESPONSE:
        {
            client->clientUI->handleAuthorization(message);
            break;
        }

        case COMMAND::CONTACT_LIST_RESPONSE:
        {
            QList< QPair<QString, QString> > result;
            QStringList pairs = requestSeparation(message, " /n ");
            for (QString& pairString: pairs) {
                // qDebug() << "pairString: " << pairString;
                QStringList pair = requestSeparation(pairString, " /s ");
                result.push_back({pair[0], pair[1]});
            }
            client->clientUI->handleContactList(result);
            break;
        }

        case COMMAND::MESSAGE_LIST_RESPONSE:
        {
            QList<QString> fromUser, toUser, text, timestamp;
            qDebug() << "Message list response: " << message;
            if (message.length()) {
                QStringList tuples = requestSeparation(message, " /n ");
                for (QString& tupleString: tuples) {
                    QStringList tuple = requestSeparation(tupleString, " /s ");
                    fromUser.push_back(tuple[0]);
                    toUser.push_back(tuple[1]);
                    text.push_back(tuple[2]);
                    timestamp.push_back(tuple[3]);
                }
            }
            client->clientUI->handleChat(fromUser, toUser, text, timestamp);
            break;
        }

        case COMMAND::SEND_MESSAGE_RESPONSE:
        {
            QStringList splitWords = requestSeparation(message, " /s ");
            QPair<QString, QString> result = {splitWords[0], splitWords[1]};
            client->clientUI->handleSendMessage(result);
            break;
        }
    }
}

void ClientSocket::sendSignUpRequest(QString login, QString password)
{
    qDebug() << "Sign Up requested. Login: " << login << ", password: " << password;
    QString request = "REGI";
    client->userLogin = login; // TODO: send back with response from server and do it after
    request.append(QString("%1 /s %2").arg(login).arg(password));
    tcpSocket->write(request.toUtf8());
}

void ClientSocket::sendLogInRequest(QString login, QString password)
{
    qDebug() << "Log in requested. Login: " << login << ", password: " << password;
    QString request = "AUTH";
    client->userLogin = login; // TODO: send back with response from server and do it after
    request.append(QString("%1 /s %2").arg(login).arg(password));
    tcpSocket->write(request.toUtf8());
}

void ClientSocket::sendContactListRequest(QString login, QString password)
{
    qDebug() << "Contact list requested. Login: " << login << ", password: " << password;
    QString request = "CTCS";
    request.append(QString("%1 /s %2").arg(login).arg(password));
    tcpSocket->write(request.toUtf8());
}

void ClientSocket::sendChatRequest(QString secondUser)
{
    qDebug() << "Chat requested. First user: " << client->userLogin <<
                ", second user: " << secondUser;
    QString request = "CHAT";
    request.append(QString("%1 /s %2").arg(client->userLogin).arg(secondUser));
    tcpSocket->write(request.toUtf8());
}

void ClientSocket::sendSendMessageRequest(QString secondUser, QString message)
{
    qDebug() << "Sending message requested. First user: " << client->userLogin <<
                ", second user: " << secondUser << ". Message: " << message;
    QString request = "MSSG";
    request.append(QString("%1 /s %2 /s %3").arg(client->userLogin).arg(secondUser).arg(message));
    tcpSocket->write(request.toUtf8());
}
