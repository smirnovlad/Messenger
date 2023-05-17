#include "inc/serversocket.h"

ServerSocket::ServerSocket(QObject *parent, Server *server)
    : QObject(parent), server(server), tcpServer(new QTcpServer(this))
{
    if (!tcpServer->listen(QHostAddress::Any, 55155)) {
        qDebug() << tr("Unable to start up server: %1").arg(tcpServer->errorString());
    }

    connect(tcpServer, SIGNAL(newConnection()), this, SLOT(handleConnectionRequest()));
}

void ServerSocket::handleConnectionRequest()
{
    QTcpSocket *newSocket = tcpServer->nextPendingConnection();

    connect(newSocket, SIGNAL(disconnected()), this, SLOT(handleDisconnection()));
    connect(newSocket, SIGNAL(readyRead()), this, SLOT(getRequest()));

    qDebug() << "New connection: " << newSocket->socketDescriptor();
}

void ServerSocket::handleDisconnection()
{
    QTcpSocket *clientSocket = static_cast<QTcpSocket *>(QObject::sender());
    qDebug() << "Disconnect socket: " << clientSocket;
    server->handleDisconnection(clientSocket);
}

QStringList ServerSocket::requestSeparation(QString text, QString sep)
{
    QString outStr = text;
    QStringList list = outStr.split(sep);

    return list;
}

void ServerSocket::getRequest()
{
    QTcpSocket *clientSocket = static_cast<QTcpSocket *>(QObject::sender());

    QString packetType = clientSocket->read(4);

    enum class COMMAND
    {
        NONE,
        REGISTRATION_REQUEST,
        AUTHORIZATION_REQUEST,
        CONTACT_LIST_REQUEST,
        MESSAGE_LIST_REQUEST,
        SEND_MESSAGE_REQUEST,
        LOG_OUT_REQUEST,
        EDIT_MESSAGE_REQUEST
    };
    COMMAND command = COMMAND::NONE;

    qDebug() << "Packet type: " << packetType;

    if (packetType == "REGI") {
        command = COMMAND::REGISTRATION_REQUEST;
    }
    else if (packetType == "AUTH") {
        command = COMMAND::AUTHORIZATION_REQUEST;
    }
    else if (packetType == "CTCS") {
        command = COMMAND::CONTACT_LIST_REQUEST;
    }
    else if (packetType == "CHAT") {
        command = COMMAND::MESSAGE_LIST_REQUEST;
    }
    else if (packetType == "MSSG") {
        command = COMMAND::SEND_MESSAGE_REQUEST;
    }
    else if (packetType == "LOGO") {
        command = COMMAND::LOG_OUT_REQUEST;
    }
    else if (packetType == "EMSG") {
        command = COMMAND::EDIT_MESSAGE_REQUEST;
    }

    switch (command) {
        case COMMAND::REGISTRATION_REQUEST: {
            QStringList splitWords = requestSeparation(clientSocket->readAll(), " /s ");
            qDebug() << "REGISTRATION_REQUEST: " << splitWords;
            server->handleRegistrationRequest(clientSocket, splitWords[0], splitWords[1]);
            break;
        }

        case COMMAND::AUTHORIZATION_REQUEST: {
            QStringList splitWords = requestSeparation(clientSocket->readAll(), " /s ");
            qDebug() << "AUTHORIZATION_REQUEST: " << splitWords;
            server->handleAuthorizationRequest(clientSocket, splitWords[0], splitWords[1]);
            break;
        }

        case COMMAND::CONTACT_LIST_REQUEST: {
            QString token = clientSocket->readAll();
            qDebug() << "GET_CONTACT_LIST_REQUEST. Passed token: " << token;
            server->handleContactListRequest(clientSocket, token);
            break;
        }

        case COMMAND::MESSAGE_LIST_REQUEST: {
            QStringList splitWords = requestSeparation(clientSocket->readAll(), " /s ");
            qDebug() << "GET_MESSAGE_LIST_REQUEST. From: " << splitWords[0]
                     << ", to: " << splitWords[1] << ", passed token: " << splitWords[2];
            server->handleMessageListRequest(clientSocket, splitWords[0], splitWords[1], splitWords[2]);
            break;
        }

        case COMMAND::SEND_MESSAGE_REQUEST: {
            QStringList splitWords = requestSeparation(clientSocket->readAll(), " /s ");
            qDebug() << "SEND_MESSAGE_REQUEST: " << splitWords;
            server->handleSendMessageRequest(clientSocket, splitWords[0], splitWords[1], splitWords[2],
                                             splitWords[3]);
            break;
        }

        case COMMAND::LOG_OUT_REQUEST: {
            int32_t userId = -1;
            // socketToUserID can be empty if log out was requested
            // after server shut down
            if (server->socketToUserID.find(clientSocket) != server->socketToUserID.end()) {
                userId = server->socketToUserID[clientSocket];
            }
            qDebug() << "LOG_OUT_REQUEST from user: " << userId;
            server->handleLogOutRequest(clientSocket);
            break;
        }

        case COMMAND::EDIT_MESSAGE_REQUEST: {
            QStringList splitWords = requestSeparation(clientSocket->readAll(), " /s ");
            qDebug() << "EDIT_MESSAGE_REQUEST: " << splitWords;
            server->handleEditMessageRequest(clientSocket, splitWords[0], splitWords[1], splitWords[2].toInt(),
                                             splitWords[3], splitWords[4].toInt(), splitWords[5]);
            break;
        }
    }
}

void ServerSocket::sendRegistrationResponse(QTcpSocket *clientSocket, QString result, QString message)
{
    QString response = "REGI";
    response.append(result + " /s " + message);
    clientSocket->write(response.toUtf8());
}

void ServerSocket::sendAuthorizationResponse(QTcpSocket *clientSocket, QString result, QString message)
{
    QString response = "AUTH";
    response.append(result + " /s " + message);
    clientSocket->write(response.toUtf8());
}

void ServerSocket::sendContactListResponse(QTcpSocket *clientSocket, QString message)
{
    QString response = "CTCS";
    response.append(message);
    qDebug() << "Result, contact list: " << message;
    clientSocket->write(response.toUtf8());
}

void ServerSocket::sendMessageListResponse(QTcpSocket *clientSocket, QString message)
{
    QString response = "CHAT";
    response.append(message);
    qDebug() << "Message list response: " << response;
    clientSocket->write(response.toUtf8());
}

void ServerSocket::sendSendMessageResponse(QTcpSocket *clientSocket, QString result, QString message,
                                           QString sender, QString receiver, QString timestamp,
                                           int32_t messageId, bool toActualSender)
{
    QString response = "MSSG";
    response.append(result + " /s " + message + " /s " + sender + " /s " +
        receiver + " /s " + timestamp + " /s " + QString::number(messageId) +
        " /s " + QString::number(toActualSender));
    clientSocket->write(response.toUtf8());
}

void ServerSocket::sendLogOutResponse(QTcpSocket *clientSocket, QString result)
{
    QString response = "LOGO";
    response.append(result);
    clientSocket->write(response.toUtf8());
}

void ServerSocket::sendEditMessageResponse(QTcpSocket *clientSOcket,
                                           QString result,
                                           QString sender,
                                           QString receiver,
                                           QString editedMessage,
                                           int32_t messageChatIndex)
{
    QString response = "EMSG";
    response.append(result + " /s " + sender + " /s " + receiver + " /s " +
        editedMessage + " /s " + QString::number(messageChatIndex));
    clientSOcket->write(response.toUtf8());
}

