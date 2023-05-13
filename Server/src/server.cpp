#include "inc/server.h"

#include <QDir>
#include <random>
#include <chrono>

Server::Server(QObject *parent)
    : QObject(parent) {
  sqlitedb = new SQLiteDB;
  tcpServer = new QTcpServer(this);

  if (!tcpServer->listen(QHostAddress::Any, 55155)) {
    qDebug() << tr("Unable to start up server: %1").arg(tcpServer->errorString());
  }

  connect(tcpServer, SIGNAL(newConnection()), this, SLOT(handleConnectionRequest()));
}

void Server::handleConnectionRequest() {
  QTcpSocket *newSocket = tcpServer->nextPendingConnection();

  connect(newSocket, SIGNAL(disconnected()), this, SLOT(handleDisconnection()));
  connect(newSocket, SIGNAL(readyRead()), this, SLOT(getRequest()));

  qDebug() << "New connection: " << newSocket->socketDescriptor();
}

void Server::handleDisconnection() {
  QTcpSocket *clientSocket = static_cast<QTcpSocket *>(QObject::sender());
  qDebug() << "Disconnect socket: " << clientSocket;
  int32_t userId = 0;
  auto firstIt = socketToUserID.find(clientSocket);
  if (firstIt != socketToUserID.end()) {
    userId = firstIt.value();
    socketToUserID.erase(firstIt);
    auto secondIt = userIDToSocket.find(userId, clientSocket);
    // secondIt != userIDToSocket.end()
    userIDToSocket.erase(secondIt);
  } else {
    // It can occur in case of disconnected server
    // and further reconnect to server.
    // Formally, it's not an error
    qDebug() << "Error while disconnecting socket";
  }
}

QStringList Server::requestSeparation(QString text, QString sep) {
  QString outStr = text;
  QStringList list = outStr.split(sep);

  return list;
}

void Server::getRequest() {
  QTcpSocket *clientSocket = static_cast<QTcpSocket *>(QObject::sender());

  QString packetType = clientSocket->read(4);

  enum class COMMAND {
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
  } else if (packetType == "AUTH") {
    command = COMMAND::AUTHORIZATION_REQUEST;
  } else if (packetType == "CTCS") {
    command = COMMAND::CONTACT_LIST_REQUEST;
  } else if (packetType == "CHAT") {
    command = COMMAND::MESSAGE_LIST_REQUEST;
  } else if (packetType == "MSSG") {
    command = COMMAND::SEND_MESSAGE_REQUEST;
  } else if (packetType == "LOGO") {
    command = COMMAND::LOG_OUT_REQUEST;
  } else if (packetType == "EMSG") {
    command = COMMAND::EDIT_MESSAGE_REQUEST;
  }

  switch (command) {
    case COMMAND::REGISTRATION_REQUEST: {
      QStringList splitWords = requestSeparation(clientSocket->readAll(), " /s ");
      qDebug() << "REGISTRATION_REQUEST: " << splitWords;
      handleRegistrationRequest(clientSocket, splitWords[0], splitWords[1]);
      break;
    }

    case COMMAND::AUTHORIZATION_REQUEST: {
      QStringList splitWords = requestSeparation(clientSocket->readAll(), " /s ");
      qDebug() << "AUTHORIZATION_REQUEST: " << splitWords;
      handleAuthorizationRequest(clientSocket, splitWords[0], splitWords[1]);
      break;
    }

    case COMMAND::CONTACT_LIST_REQUEST: {
      QString token = clientSocket->readAll();
      qDebug() << "GET_CONTACT_LIST_REQUEST. Passed token: " << token;
      handleContactListRequest(clientSocket, token);
      break;
    }

    case COMMAND::MESSAGE_LIST_REQUEST: {
      QStringList splitWords = requestSeparation(clientSocket->readAll(), " /s ");
      qDebug() << "GET_MESSAGE_LIST_REQUEST. From: " << splitWords[0]
               << ", to: " << splitWords[1] << ", passed token: " << splitWords[2];
      handleMessageListRequest(clientSocket, splitWords[0], splitWords[1], splitWords[2]);
      break;
    }

    case COMMAND::SEND_MESSAGE_REQUEST: {
      QStringList splitWords = requestSeparation(clientSocket->readAll(), " /s ");
      qDebug() << "SEND_MESSAGE_REQUEST: " << splitWords;
      handleSendMessageRequest(clientSocket, splitWords[0], splitWords[1], splitWords[2],
                               splitWords[3]);
      break;
    }

    case COMMAND::LOG_OUT_REQUEST: {
      int32_t userId = 0;
      // socketToUserID can be empty if log out was requested
      // after server shut down
      if (socketToUserID.find(clientSocket) != socketToUserID.end()) {
        userId = socketToUserID[clientSocket];
      }
      qDebug() << "LOG_OUT_REQUEST from user: " << userId;
      handleLogOutRequest(clientSocket);
      break;
    }

    case COMMAND::EDIT_MESSAGE_REQUEST: {
      QStringList splitWords = requestSeparation(clientSocket->readAll(), " /s ");
      qDebug() << "EDIT_MESSAGE_REQUEST: " << splitWords;
      handleEditMessageRequest(clientSocket, splitWords[0], splitWords[1], splitWords[2].toInt(),
                               splitWords[3], splitWords[4].toInt(), splitWords[5]);
      break;
    }
  }
}

void Server::sendRegistrationResponse(QTcpSocket *clientSocket, QString result, QString message) {
  QString response = "REGI";
  response.append(result + " /s " + message);
  clientSocket->write(response.toUtf8());
}

void Server::handleRegistrationRequest(QTcpSocket *clientSocket, QString &login, QString &password) {
  qDebug() << "Checking data to sign up...";
  qDebug() << "login: " << login << ", password: " << password;

  QString result, message;

  // add: login and password don't contain symbols like space and others
  if (login.length() < 4 || password.length() < 4) {
    result = "ILEN"; // incorrect length
  } else if (!isCorrectLogin(login) || !isCorrectPassword(password)) {
    result = "ISYM"; // incorrect symbols
    message = incorrectLoginSymbols + " /s " + incorrectPasswordSymbols;
  } else if (sqlitedb->findUser(login) != -1) {
    result = "ALRD";
  } else {
    int32_t id = sqlitedb->addUser(login, password);
    qDebug() << "New user id: " << id;
    result = "SCSS";
    message = login;
  }
  sendRegistrationResponse(clientSocket, result, message);
}

void Server::sendAuthorizationResponse(QTcpSocket *clientSocket, QString result, QString message) {
  QString response = "AUTH";
  response.append(result + " /s " + message);
  clientSocket->write(response.toUtf8());
}

void Server::handleAuthorizationRequest(QTcpSocket *clientSocket, QString &login, QString &password) {
  qDebug() << "Checking data to log in...";
  qDebug() << "login: " << login << ", password: " << password;

  QString result, message;

  if (login.length() < 4 || password.length() < 4) {
    result = "ILEN"; // incorrect length
  } else if (!isCorrectLogin(login) || !isCorrectPassword(password)) {
    result = "ISYM"; // incorrect symbols
    message = incorrectLoginSymbols + " /s " + incorrectPasswordSymbols;
  } else {
    int32_t userId = sqlitedb->findUser(login);
    qDebug() << "User ID in DB: " << userId;
    if (userId == -1) {
      result = "NFND"; // not found
    } else if (!sqlitedb->checkPassword(userId, password)) {
      result = "IPSW";// incorrect password
    } else {
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
  sendAuthorizationResponse(clientSocket, result, message);
}

void Server::sendContactListResponse(QTcpSocket *clientSocket, QString message) {
  QString response = "CTCS";
  response.append(message);
  qDebug() << "Result, contact list: " << message;
  clientSocket->write(response.toUtf8());
}

void Server::handleContactListRequest(QTcpSocket *clientSocket, QString token) {
  QString contactList;
  QString result;
  if (checkToken(clientSocket, token)) {
    sqlitedb->getContactList(contactList);
    result = "SCSS /n ";
  } else {
    result = "ITKN /n "; // incorrect token
  }
  sendContactListResponse(clientSocket, result + contactList);
}

void Server::sendMessageListResponse(QTcpSocket *clientSocket, QString message) {
  QString response = "CHAT";
  response.append(message);
  qDebug() << "Message list response: " << response;
  clientSocket->write(response.toUtf8());
}

void Server::handleMessageListRequest(QTcpSocket *clientSocket, QString firstUser,
                                      QString secondUser, QString token) {
  QString messageList;
  QString result;
  if (checkToken(clientSocket, token)) {
    sqlitedb->getMessageList(messageList, firstUser, secondUser);
    result = "SCSS /n ";
  } else {
    result = "ITKN /n "; // incorrect token
  }
  // TODO: solve problem if message contains separator " /s "
  sendMessageListResponse(clientSocket, result + messageList);
}

void Server::sendSendMessageResponse(QTcpSocket *clientSocket, QString result, QString message,
                                     QString sender, QString receiver, QString timestamp,
                                     int32_t messageId, bool toActualSender) {
  QString response = "MSSG";
  response.append(result + " /s " + message + " /s " + sender + " /s " +
      receiver + " /s " + timestamp + " /s " + QString::number(messageId) +
      " /s " + QString::number(toActualSender));
  clientSocket->write(response.toUtf8());
}

void Server::handleSendMessageRequest(QTcpSocket *clientSocket, QString sender,
                                      QString receiver, QString message, QString token) {
  QString result;
  QString timestamp = getConnectionTimeStamp();
  int32_t messageId = -1;
  if (checkToken(clientSocket, token)) {
    sqlitedb->sendMessage(sender, receiver, message, timestamp, messageId);
    result = "SCSS";
  } else {
    result = "ITKN"; // incorrect token
  }

  if (result == "ITKN") {
    // Two cases:
    // 1) At moment of request processing userIDToSocket can be empty,
    //    if server was shut down.
    // 2) The token could also expire
    sendSendMessageResponse(clientSocket, result, message, sender,
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
      sendSendMessageResponse(it.value(), result, message, sender,
                              receiver, timestamp, messageId, true);
    } else {
      sendSendMessageResponse(it.value(), result, message, sender,
                              receiver, timestamp, messageId, false);
    }
  }
  if (sender != receiver) {
    uint32_t receiverUserId = sqlitedb->findUser(receiver);
    qDebug() << "Receiver user sockets count: " << userIDToSocket.count(receiverUserId);
    auto receiverUserSockets = userIDToSocket.equal_range(receiverUserId);
    for (auto it = receiverUserSockets.first; it != receiverUserSockets.second; ++it) {
      sendSendMessageResponse(it.value(), result, message, sender,
                              receiver, timestamp, messageId, false);
    }
  }
}

void Server::sendLogOutResponse(QTcpSocket *clientSocket, QString result) {
  QString response = "LOGO";
  response.append(result);
  clientSocket->write(response.toUtf8());
}

void Server::handleLogOutRequest(QTcpSocket *clientSocket) {
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
  } else {
    // It can occur in case of disconnected server
    // and further reconnect to server.
    // Formally, it's not an error
    qDebug() << "Error while log out";
  }
  result = "SCSS";
  sendLogOutResponse(clientSocket, result);
}

void Server::sendEditMessageResponse(QTcpSocket *clientSOcket,
                                     QString result,
                                     QString sender,
                                     QString receiver,
                                     QString editedMessage,
                                     int32_t messageChatIndex) {
  QString response = "EMSG";
  response.append(result + " /s " + sender + " /s " + receiver + " /s " +
                  editedMessage + " /s " + QString::number(messageChatIndex));
  clientSOcket->write(response.toUtf8());
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
  } else {
    result = "ITKN"; // incorrect token
  }

  if (result == "ITKN") {
    sendEditMessageResponse(clientSocket, result, sender,
                            receiver, editedMessage, messageChatIndex);
    return;
  }

  uint32_t senderUserId = sqlitedb->findUser(sender);
  auto senderUserSockets = userIDToSocket.equal_range(senderUserId);
  qDebug() << "Actual sender clientSocket: " << clientSocket;
  qDebug() << "Sender user sockets count: " << userIDToSocket.count(senderUserId);
  for (auto it = senderUserSockets.first; it != senderUserSockets.second; ++it) {
    sendEditMessageResponse(it.value(), result, sender, receiver,
                            editedMessage, messageChatIndex);
  }
  if (sender != receiver) {
    uint32_t receiverUserId = sqlitedb->findUser(receiver);
    qDebug() << "Receiver user sockets count: " << userIDToSocket.count(receiverUserId);
    auto receiverUserSockets = userIDToSocket.equal_range(receiverUserId);
    for (auto it = receiverUserSockets.first; it != receiverUserSockets.second; ++it) {
      sendEditMessageResponse(it.value(), result, sender, receiver,
                              editedMessage, messageChatIndex);
    }
  }
}

QString Server::getConnectionTimeStamp() {
  QString time = QDateTime::currentDateTime().toString();
  return "[" + time + "]";
}

QString Server::generateToken() {
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

bool Server::checkToken(QTcpSocket *clientSocket, QString token) {
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

bool Server::checkTimeStamp(QString timeStamp) {
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

bool Server::isCorrectLogin(QString login) {
  for (QChar symbol : incorrectLoginSymbols) {
    if (login.contains(symbol)) {
      return false;
    }
  }
  return true;
}

bool Server::isCorrectPassword(QString password) {
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

Server::~Server() {
}

