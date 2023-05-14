#include "inc/server.h"

#include <QDir>
#include <random>
#include <chrono>

Server::Server(QObject *parent)
    : QObject(parent)
{
    sqlitedb = new SQLiteDB;
    serverSocket = new ServerSocket(NULL, this);
}

void Server::handleDisconnection(QTcpSocket *clientSocket)
{
    int32_t userId = 0;
    auto firstIt = socketToUserID.find(clientSocket);
    if (firstIt != socketToUserID.end()) {
        userId = firstIt.value();
        socketToUserID.erase(firstIt);
        auto secondIt = userIDToSocket.find(userId, clientSocket);
        // secondIt != userIDToSocket.end()
        userIDToSocket.erase(secondIt);
    }
    else {
        // It can occur in case of disconnected server
        // and further reconnect to server.
        // Formally, it's not an error
        qDebug() << "Error while disconnecting socket";
    }
}

QStringList Server::requestSeparation(QString text, QString sep)
{
    QString outStr = text;
    QStringList list = outStr.split(sep);

    return list;
}

void Server::handleRegistrationRequest(QTcpSocket *clientSocket, QString &login, QString &password)
{
    qDebug() << "Checking data to sign up...";
    qDebug() << "login: " << login << ", password: " << password;

    QString result, message;

    // add: login and password don't contain symbols like space and others
    if (login.length() < 4 || password.length() < 4) {
        result = "ILEN"; // incorrect length
    }
    else if (!isCorrectLogin(login) || !isCorrectPassword(password)) {
        result = "ISYM"; // incorrect symbols
        message = incorrectLoginSymbols + " /s " + incorrectPasswordSymbols;
    }
    else if (sqlitedb->findUser(login) != -1) {
        result = "ALRD";
    }
    else {
        int32_t id = sqlitedb->addUser(login, password);
        qDebug() << "New user id: " << id;
        result = "SCSS";
        message = login;
    }
    serverSocket->sendRegistrationResponse(clientSocket, result, message);
}

void Server::handleAuthorizationRequest(QTcpSocket *clientSocket, QString &login, QString &password)
{
    qDebug() << "Checking data to log in...";
    qDebug() << "login: " << login << ", password: " << password;

    QString result, message;

    if (login.length() < 4 || password.length() < 4) {
        result = "ILEN"; // incorrect length
    }
    else if (!isCorrectLogin(login) || !isCorrectPassword(password)) {
        result = "ISYM"; // incorrect symbols
        message = incorrectLoginSymbols + " /s " + incorrectPasswordSymbols;
    }
    else {
        int32_t userId = sqlitedb->findUser(login);
        qDebug() << "User ID in DB: " << userId;
        if (userId == -1) {
            result = "NFND"; // not found
        }
        else if (!sqlitedb->checkPassword(userId, password)) {
            result = "IPSW";// incorrect password
        }
        else {
            qDebug() << "Sender user sockets count before log in: " << userIDToSocket.count(userId);
            // QString token = generateToken();
            QString token, creationTimestamp;
            sqlitedb->getToken(userId, token, creationTimestamp);
            if (token == "" || !checkTimeStamp(creationTimestamp)) {
                token = generateToken();
                qDebug() << "Generated token: " << token;
                sqlitedb->updateToken(userId, token, getConnectionTimeStamp());
            }
            result = "SCSS";
            message = token + " /s " + login;
            userIDToSocket.insert(userId, clientSocket);
            socketToUserID[clientSocket] = userId;
            qDebug() << "Sender user sockets count after log in: " << userIDToSocket.count(userId);
        }
    }
    serverSocket->sendAuthorizationResponse(clientSocket, result, message);
}

void Server::handleContactListRequest(QTcpSocket *clientSocket, QString token)
{
    QString contactList;
    QString result;
    if (checkToken(clientSocket, token)) {
        sqlitedb->getContactList(contactList);
        result = "SCSS /n ";
    }
    else {
        result = "ITKN /n "; // incorrect token
    }
    serverSocket->sendContactListResponse(clientSocket, result + contactList);
}

void Server::handleMessageListRequest(QTcpSocket *clientSocket, QString firstUser,
                                      QString secondUser, QString token)
{
    QString messageList;
    QString result;
    if (checkToken(clientSocket, token)) {
        sqlitedb->getMessageList(messageList, firstUser, secondUser);
        result = "SCSS /n ";
    }
    else {
        result = "ITKN /n "; // incorrect token
    }
    // TODO: solve problem if message contains separator " /s "
    serverSocket->sendMessageListResponse(clientSocket, result + messageList);
}

void Server::handleSendMessageRequest(QTcpSocket *clientSocket, QString sender,
                                      QString receiver, QString message, QString token)
{
    QString result;
    QString timestamp = getConnectionTimeStamp();
    int32_t messageId = -1;
    if (checkToken(clientSocket, token)) {
        sqlitedb->sendMessage(sender, receiver, message, timestamp, messageId);
        result = "SCSS";
    }
    else {
        result = "ITKN"; // incorrect token
    }

    if (result == "ITKN") {
        // Two cases:
        // 1) At moment of request processing userIDToSocket can be empty,
        //    if server was shut down.
        // 2) The token could also expire
        serverSocket->sendSendMessageResponse(clientSocket, result, message, sender,
                                              receiver, timestamp, messageId, true);
        return;
    }

    // Send response to other clients only if result is success
    uint32_t senderUserId = sqlitedb->findUser(sender);
    auto senderUserSockets = userIDToSocket.equal_range(senderUserId);
    qDebug() << "Actual sender clientSocket: " << clientSocket;
    qDebug() << "Sender user sockets count: " << userIDToSocket.count(senderUserId);
    for (auto it = senderUserSockets.first; it != senderUserSockets.second; ++it) {
        if (it.value() == clientSocket) {
            serverSocket->sendSendMessageResponse(it.value(), result, message, sender,
                                                  receiver, timestamp, messageId, true);
        }
        else {
            serverSocket->sendSendMessageResponse(it.value(), result, message, sender,
                                                  receiver, timestamp, messageId, false);
        }
    }
    if (sender != receiver) {
        uint32_t receiverUserId = sqlitedb->findUser(receiver);
        qDebug() << "Receiver user sockets count: " << userIDToSocket.count(receiverUserId);
        auto receiverUserSockets = userIDToSocket.equal_range(receiverUserId);
        for (auto it = receiverUserSockets.first; it != receiverUserSockets.second; ++it) {
            serverSocket->sendSendMessageResponse(it.value(), result, message, sender,
                                                  receiver, timestamp, messageId, false);
        }
    }
}

void Server::handleLogOutRequest(QTcpSocket *clientSocket)
{
    QString result;
//  qDebug() << "Actual sender clientSocket: " << clientSocket;
    auto firstIt = socketToUserID.find(clientSocket);
    if (firstIt != socketToUserID.end()) {
        uint32_t id = firstIt.value();
        qDebug() << "Sender user sockets count before log out: " << userIDToSocket.count(id);
        socketToUserID.erase(firstIt);
        auto secondIt = userIDToSocket.find(id, clientSocket);
        // secondIt != userIDToSocket.end()
        userIDToSocket.erase(secondIt);
        qDebug() << "Sender user sockets count after log out: " << userIDToSocket.count(id);
    }
    else {
        // It can occur in case of disconnected server
        // and further reconnect to server.
        // Formally, it's not an error
        qDebug() << "Error while log out";
    }
    result = "SCSS";
    serverSocket->sendLogOutResponse(clientSocket, result);
}

void Server::handleEditMessageRequest(QTcpSocket *clientSocket,
                                      QString sender,
                                      QString receiver,
                                      int32_t messageId,
                                      QString editedMessage,
                                      int32_t messageChatIndex,
                                      QString token)
{
    QString result;
    if (checkToken(clientSocket, token)) {
        sqlitedb->editMessage(sender, receiver, messageId, editedMessage);
        sqlitedb->editMessage(receiver, sender, messageId, editedMessage);
        result = "SCSS";
    }
    else {
        result = "ITKN"; // incorrect token
    }

    if (result == "ITKN") {
        serverSocket->sendEditMessageResponse(clientSocket, result, sender,
                                              receiver, editedMessage, messageChatIndex);
        return;
    }

    uint32_t senderUserId = sqlitedb->findUser(sender);
    auto senderUserSockets = userIDToSocket.equal_range(senderUserId);
    qDebug() << "Actual sender clientSocket: " << clientSocket;
    qDebug() << "Sender user sockets count: " << userIDToSocket.count(senderUserId);
    for (auto it = senderUserSockets.first; it != senderUserSockets.second; ++it) {
        serverSocket->sendEditMessageResponse(it.value(), result, sender, receiver,
                                              editedMessage, messageChatIndex);
    }
    if (sender != receiver) {
        uint32_t receiverUserId = sqlitedb->findUser(receiver);
        qDebug() << "Receiver user sockets count: " << userIDToSocket.count(receiverUserId);
        auto receiverUserSockets = userIDToSocket.equal_range(receiverUserId);
        for (auto it = receiverUserSockets.first; it != receiverUserSockets.second; ++it) {
            serverSocket->sendEditMessageResponse(it.value(), result, sender, receiver,
                                                  editedMessage, messageChatIndex);
        }
    }
}

QString Server::getConnectionTimeStamp()
{
    QString time = QDateTime::currentDateTime().toString();
    return "[" + time + "]";
}

QString Server::generateToken()
{
    std::mt19937 gen(std::chrono::high_resolution_clock::now().time_since_epoch().count());

    const QString possibleCharacters("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");
    const uint32_t randomStringLength = 32;

    QString token;
    for (uint32_t i = 0; i < randomStringLength; ++i) {
        uint32_t index = gen() % possibleCharacters.length();
        QChar nextChar = possibleCharacters.at(index);
        token.append(nextChar);
    }
    return token;
}

bool Server::checkToken(QTcpSocket *clientSocket, QString token)
{
    int32_t userId = -1;
    // if server was disconnected, socketToUserID can be empty, so...
    auto it = socketToUserID.find(clientSocket);
    if (it != socketToUserID.end()) {
        userId = *it;
    }

    QString lastToken, lastCreationTimeStamp;
    sqlitedb->getToken(userId, lastToken, lastCreationTimeStamp);
    qDebug() << "userId = " << userId << ", passed token: " << token << ", last token = " << lastToken;
    return (token == lastToken && checkTimeStamp(lastCreationTimeStamp));
}

bool Server::checkTimeStamp(QString timeStamp)
{
    QString currentTimeStamp = getConnectionTimeStamp();
    QStringList splitCTS = requestSeparation(currentTimeStamp, " "),
        splitTS = requestSeparation(timeStamp, " ");
    QString clocksCTS = splitCTS[3],
        clocksTS = splitTS[3];
    if (splitCTS[1] == splitTS[1] && splitCTS[2] == splitTS[2]) { // compare days
        QStringList splitClocksCTS = requestSeparation(clocksCTS, ":");
        QStringList splitClocksTS = requestSeparation(clocksTS, ":");
        if (std::abs(splitClocksCTS[0].toInt() - splitClocksTS[0].toInt()) <= 12) { // compare hours
            return true;
        }
    }
    return false;
}

bool Server::isCorrectLogin(QString login)
{
    for (QChar symbol : incorrectLoginSymbols) {
        if (login.contains(symbol)) {
            return false;
        }
    }
    return true;
}

bool Server::isCorrectPassword(QString password)
{
    for (QChar symbol : incorrectPasswordSymbols) {
        if (password.contains(symbol)) {
            return false;
        }
    }
    // check if at least one non space symbol
    for (QChar symbol : password) {
        if (symbol != " ") {
            return true;
        }
    }
    return false;
}

Server::~Server()
{
    delete sqlitedb;
    delete serverSocket;
}

