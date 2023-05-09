#include "server.h"

#include <QDir>

Server::Server(QObject *parent)
    : QObject(parent)
    , nextBlockSize(0)
{
    sqlitedb = new SQLiteDB;
    tcpServer = new QTcpServer(this);

    if (!tcpServer->listen(QHostAddress::Any, 55155)) {
        qDebug() << tr("Unable to start up server: %1").arg(tcpServer->errorString());
    }

    connect(tcpServer, SIGNAL(newConnection()), this, SLOT(handleConnectionRequest()));
}

void Server::handleConnectionRequest()
{
    QTcpSocket *newSocket = tcpServer->nextPendingConnection();

//    qDebug() << "newSocket: " << newSocket;

    connect(newSocket, SIGNAL(disconnected()), this, SLOT(handleDisconnection()));
    connect(newSocket, SIGNAL(readyRead()), this, SLOT(getRequest()));

    qDebug() << "New connection: " << newSocket->socketDescriptor();
}

void Server::handleDisconnection()
{
    qDebug() << "Disconnect socket";

    QTcpSocket *clientSocket = static_cast<QTcpSocket *>(QObject::sender());
    uint32_t id = socketToUserID[clientSocket];
    socketToUserID.remove(clientSocket);
    userIDToSocket.remove(id);
}

QStringList Server::requestSeparation(QString text)
{
    QString outStr =  text;
    QStringList list = outStr.split(" /s ");

    return list;
}

void Server::getRequest()
{
    QTcpSocket *clientSocket = static_cast<QTcpSocket *>(QObject::sender());

    QString packetType = clientSocket->read(4);

    enum class COMMAND { NONE,
                         REGISTRATION_REQUEST,
                         AUTHORIZATION_REQUEST,
                         CONTACT_LIST_REQUEST,
                         MESSAGE_LIST_REQUEST,
                         SEND_MESSAGE_REQUEST
                       };
    COMMAND command = COMMAND::NONE;

    qDebug() << "Packet type: " << packetType;

    if (packetType == "REGI") {
        command = COMMAND::REGISTRATION_REQUEST;
    } else if (packetType == "AUTH") {
        command = COMMAND::AUTHORIZATION_REQUEST;
    } else if (packetType == "CTCS") {
        command = COMMAND::CONTACT_LIST_REQUEST;
    } else if (packetType == "CHAT") {
        command = COMMAND::MESSAGE_LIST_REQUEST;
    } else if (packetType == "MSSG") {
        command = COMMAND::SEND_MESSAGE_REQUEST;
    }

    QStringList splitWords;

    switch (command)
    {
        case COMMAND::REGISTRATION_REQUEST:
        {
            splitWords = requestSeparation(clientSocket->readAll());
            qDebug() << "REGISTRATION_REQUEST: " << splitWords;
            handleRegistrationRequest(clientSocket, splitWords[0], splitWords[1]);
            break;
        }

        case COMMAND::AUTHORIZATION_REQUEST:
        {
            splitWords = requestSeparation(clientSocket->readAll());
            qDebug() << "AUTHORIZATION_REQUEST: " << splitWords;
            handleAuthorizationRequest(clientSocket, splitWords[0], splitWords[1]);
            break;
        }

        case COMMAND::CONTACT_LIST_REQUEST:
        {
            splitWords = requestSeparation(clientSocket->readAll());
            qDebug() << "GET_CONTACT_LIST_REQUEST: " << splitWords;
            // TODO: separate authorization method???
            handleContactListRequest(clientSocket, splitWords[0], splitWords[1]);
            break;
        }

        case COMMAND::MESSAGE_LIST_REQUEST:
        {
            splitWords = requestSeparation(clientSocket->readAll());
            qDebug() << "GET_MESSAGE_LIST_REQUEST: " << splitWords;
            handleMessageListRequest(clientSocket, splitWords[0], splitWords[1]);
            break;
        }

        case COMMAND::SEND_MESSAGE_REQUEST:
        {
            splitWords = requestSeparation(clientSocket->readAll());
            qDebug() << "SEND_MESSAGE_REQUEST: " << splitWords;
            handleSendMessageRequest(clientSocket, splitWords[0], splitWords[1], splitWords[2]);
            break;
        }
    }
}

void Server::sendRegistrationResponse(QTcpSocket *clientSocket, QString message)
{
    QString response = "REGI";
    response.append(message);
    clientSocket->write(response.toUtf8());
}

void Server::handleRegistrationRequest(QTcpSocket *clientSocket, QString &login, QString &password)
{
    qDebug() << "Checking data to sign up...";
    qDebug() << "login: " << login << ", password: " << password;

    // add: login and password don't contain symbols like space and others
    if(login.length() < 4 || password.length() < 4) {
        sendRegistrationResponse(clientSocket, "ILEN");
    } else if (sqlitedb->findUser(login) != -1){
        sendRegistrationResponse(clientSocket, "ALRD");
    } else {
        int32_t id = sqlitedb->addUser(login, password);
        qDebug() << "new user id: " << id;
        sendRegistrationResponse(clientSocket, "SCSS");
    }
}

void Server::sendAuthorizationResponse(QTcpSocket *clientSocket, QString message)
{
    QString response = "AUTH";
    response.append(message);
    clientSocket->write(response.toUtf8());
}

void Server::handleAuthorizationRequest(QTcpSocket *clientSocket, QString &login, QString &password)
{
    qDebug() << "Checking data to log in...";
    qDebug() << "login: " << login << ", password: " << password;

    if(login.length() < 4 || password.length() < 4) {
        sendAuthorizationResponse(clientSocket, "ILEN");
    } else {
        int32_t id = sqlitedb->findUser(login);
        qDebug() << "User ID in DB: " << id;
        if (id == -1){
            sendAuthorizationResponse(clientSocket, "NFND"); // not found
        } else if (!sqlitedb->checkPassword(id, password)) {
            sendAuthorizationResponse(clientSocket, "IPSW"); // incorrect password
        } else {
            sendAuthorizationResponse(clientSocket, "SCSS");
            userIDToSocket[id] = clientSocket;
            socketToUserID[clientSocket] = id;
        }
    }
}

void Server::sendContactListResponse(QTcpSocket *clientSocket, QString& contactList)
{
    QString response = "CTCS";
    response.append(contactList);
    qDebug() << "Contact list: " << contactList;
    clientSocket->write(response.toUtf8());
}

void Server::handleContactListRequest(QTcpSocket *clientSocket, QString &login, QString &password)
{
    // TODO: separate authorization method???
    QString contactList;
    sqlitedb->getContactList(contactList);
    sendContactListResponse(clientSocket, contactList);
}

void Server::sendMessageListResponse(QTcpSocket *clientSocket, QString &messageList)
{
    QString response = "CHAT";
    response.append(messageList);
    qDebug() << "Message list: " << messageList;
    clientSocket->write(response.toUtf8());
}

void Server::handleMessageListRequest(QTcpSocket *clientSocket, QString firstUser, QString secondUser)
{
    QString messageList;
    // TODO: solve problem if message contains separator " /s "
    sqlitedb->getMessageList(messageList, firstUser, secondUser);
    sendMessageListResponse(clientSocket, messageList);
}

void Server::sendSendMessageResponse(QTcpSocket *clientSocket, QString result, QString message,
                                     QString sender, QString receiver, QString timestamp)
{
    QString response = "MSSG";
    response.append(result + " /s " + message + " /s " + sender + " /s " +
                    receiver + " /s " + timestamp);
    clientSocket->write(response.toUtf8());
}

void Server::handleSendMessageRequest(QTcpSocket *clientSocket, QString sender,
                                      QString receiver, QString message)
{
    QString timestamp = getConnectionTimeStamp();
    sqlitedb->sendMessage(sender, receiver, message, timestamp);
    sendSendMessageResponse(clientSocket, "SCSS", message, sender,
                            receiver, timestamp);
    uint32_t secondUserID = sqlitedb->findUser(receiver);
    if (userIDToSocket.find(secondUserID) != userIDToSocket.end()) {
        sendSendMessageResponse(userIDToSocket[secondUserID], "SCSS", message, sender,
                                receiver, timestamp);
    }
}

QString Server::getConnectionTimeStamp()
{
    QString time = QDateTime::currentDateTime().toString();
    return "[" + time + "]";
}

Server::~Server()
{
}

