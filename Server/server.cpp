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
    User *newUser = new User;
    newUser->setSocket(tcpServer->nextPendingConnection());
    QTcpSocket *newSocket = newUser->getSocket();

    connect(newSocket, SIGNAL(disconnected()), this, SLOT(onDisconnect()));
    connect(newSocket, SIGNAL(readyRead()), this, SLOT(getRequest()));
    userConnections.append(newUser);

    qDebug() << "New connection: " << newSocket->socketDescriptor();
}

QStringList Server::requestSeparation(QString text)
{
    QString outStr =  text;
    QStringList list = outStr.split(" /s ");

    return list;
}

void Server::getRequest()
{
    QString time;

    QTcpSocket *clientSocket = static_cast<QTcpSocket *>(QObject::sender());

    QString packetType = clientSocket->read(4);

    enum class COMMAND { NONE,
                         REGISTRATION_REQUEST,
                         AUTHORIZATION_REQUEST,
                         CONTACT_LIST_REQUEST,
                         MESSAGE_LIST_REQUEST
                       };
    COMMAND command = COMMAND::NONE;

    qDebug() << "packetType: " << packetType;

    if (packetType == "REGI") {
        command = COMMAND::REGISTRATION_REQUEST;
    } else if (packetType == "AUTH") {
        command = COMMAND::AUTHORIZATION_REQUEST;
    } else if (packetType == "CTCS") {
        command = COMMAND::CONTACT_LIST_REQUEST;
    } else if (packetType == "CHAT") {
        command = COMMAND::MESSAGE_LIST_REQUEST;
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

    // TODO: check in DB
    if(login.length() < 4 || password.length() < 4) {
        sendAuthorizationResponse(clientSocket, "ILEN");
    } else {
        int32_t id = sqlitedb->findUser(login);
        qDebug() << "id = " << id;
        if (id == -1){
            sendAuthorizationResponse(clientSocket, "NFND"); // not found
        } else if (!sqlitedb->checkPassword(id, password)) {
            sendAuthorizationResponse(clientSocket, "IPSW"); // incorrect password
        } else {
            sendAuthorizationResponse(clientSocket, "SCSS");
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
    // TODO: solve problem if message containts separator " /s "
    sqlitedb->getMessageList(messageList, firstUser, secondUser);
    sendMessageListResponse(clientSocket, messageList);
}

QString Server::getConnectionTimeStamp()
{
    QString time = QDateTime::currentDateTime().toString();
    return "[" + time + "]";
}

Server::~Server()
{
}

