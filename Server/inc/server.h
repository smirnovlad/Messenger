#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QTimer>
#include <QDateTime>

#include "sqlitedb.h"
#include "serversocket.h"

class SQLiteDB;
class ServerSocket;

class Server: public QObject
{
Q_OBJECT

    friend class SQLiteDB;
    friend class ServerSocket;

public:
    Server(QObject *parent = nullptr);
    ~Server();

private:
    SQLiteDB *sqlitedb;
    ServerSocket *serverSocket;
    QMultiMap<uint32_t, QTcpSocket *> userIDToSocket;
    QMap<QTcpSocket *, uint32_t> socketToUserID;

    QString incorrectLoginSymbols = "_' ,.!@#$%^&*()<>+=-/|~`\"";
    QString incorrectPasswordSymbols = "/";

    QString getConnectionTimeStamp();
    QString generateToken();
    bool checkToken(QTcpSocket *clientSocket, QString token);
    bool checkTimeStamp(QString timeStamp);

    QStringList requestSeparation(QString text, QString sep);

    bool isCorrectLogin(QString login);
    bool isCorrectPassword(QString password);

private:
    void handleDisconnection(QTcpSocket *clientSocket);
    void handleRegistrationRequest(QTcpSocket *clientSocket, QString &login, QString &password);
    void handleAuthorizationRequest(QTcpSocket *clientSocket, QString &login, QString &password);
    void handleContactListRequest(QTcpSocket *clientSocket, QString token);
    void handleMessageListRequest(QTcpSocket *clientSocket, QString firstUser,
                                  QString secondUser, QString token);
    void handleSendMessageRequest(QTcpSocket *clientSocket, QString sender, QString receiver,
                                  QString message, QString token);
    void handleLogOutRequest(QTcpSocket *clientSocket);
    void handleEditMessageRequest(QTcpSocket *clientSocket, QString sender, QString receiver,
                                  int32_t messageId, QString editedMessage, int32_t messageChatIndex,
                                  QString token);
};
#endif // SERVER_H
