#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QMessageBox>
#include <QTimer>
#include <QDateTime>
#include <QList>
#include <QPair>

#include "sqlitedb.h"

class Server : public QObject
{
    Q_OBJECT

public:
    Server(QObject *parent = nullptr);
    ~Server();

private:
    SQLiteDB *sqlitedb;
    QTcpServer *tcpServer;
    QMultiMap<uint32_t, QTcpSocket*> userIDToSocket;
    QMap<QTcpSocket*, uint32_t> socketToUserID;

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
    void handleRegistrationRequest(QTcpSocket *clientSocket, QString &login, QString &password);
    void sendRegistrationResponse(QTcpSocket *clientSocket, QString result, QString message);

    void handleAuthorizationRequest(QTcpSocket *clientSocket, QString &login, QString &password);
    void sendAuthorizationResponse(QTcpSocket *clientSocket, QString result, QString message);

    void handleContactListRequest(QTcpSocket *clientSocket, QString token);
    void sendContactListResponse(QTcpSocket *clientSocket, QString message);

    void handleMessageListRequest(QTcpSocket *clientSocket, QString firstUser,
                                  QString secondUser, QString token);
    void sendMessageListResponse(QTcpSocket *clientSocket, QString message);

    void handleSendMessageRequest(QTcpSocket *clientSocket, QString sender, QString receiver,
                                  QString message, QString token);
    void sendSendMessageResponse(QTcpSocket *clientSocket, QString result, QString message,
                                 QString firstUser, QString secondUser, QString timestamp);

    void handleLogOutRequest(QTcpSocket *clientSocket);
    void sendLogOutResponse(QTcpSocket *clientSocket, QString result);

private slots:
    void handleConnectionRequest();
    void handleDisconnection();
    void getRequest();
};
#endif // SERVER_H
