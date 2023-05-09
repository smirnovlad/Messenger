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
    QString typePacket;

    QStringList splitWords;

    QTcpSocket *clientSocket = static_cast<QTcpSocket *>(QObject::sender());

    typePacket = clientSocket->read(4);

    int command = 0;

    if (typePacket == "REGI")
            command = 5;

    switch (command)
    {
        case 5:
        {
            splitWords = requestSeparation(clientSocket->readAll());
            qDebug() << splitWords;
            handleRegistrationRequest(clientSocket, splitWords[0], splitWords[1]);
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
    qDebug() << "log: " << login << ", pass: " << password;

    if(login.length() < 4 || password.length() < 4) {
        sendRegistrationResponse(clientSocket, "ILEN");
    } else {
        sendRegistrationResponse(clientSocket, "SCSS");
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

