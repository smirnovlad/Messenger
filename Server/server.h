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
#include "user.h"

class Server : public QObject
{
    Q_OBJECT

public:
    Server(QObject *parent = nullptr);
    ~Server();

private:
    SQLiteDB *sqlitedb;
    QTcpServer *tcpServer;
    QMap<uint32_t, QTcpSocket*> userIDToSocket;
    QMap<QTcpSocket*, uint32_t> socketToUserID;

    quint32 nextBlockSize;

    QString getConnectionTimeStamp();

private:
    void handleRegistrationRequest(QTcpSocket *clientSocket, QString &login, QString &password);
    void sendRegistrationResponse(QTcpSocket *clientSocket, QString message);

    void handleAuthorizationRequest(QTcpSocket *clientSocket, QString &login, QString &password);
    void sendAuthorizationResponse(QTcpSocket *clientSocket, QString message);

    void handleContactListRequest(QTcpSocket *clientSocket, QString &login, QString &password);
    void sendContactListResponse(QTcpSocket *clientSocket, QString& contactList);

    void handleMessageListRequest(QTcpSocket *clientSocket, QString firstUser, QString secondUser);
    void sendMessageListResponse(QTcpSocket *clientSocket, QString& messageList);

    void handleSendMessageRequest(QTcpSocket *clientSocket, QString sender, QString receiver,
                                  QString message);
    void sendSendMessageResponse(QTcpSocket *clientSocket, QString result, QString message,
                                 QString firstUser, QString secondUser, QString timestamp);

private slots:
    void handleConnectionRequest();
    void handleDisconnection();
    void getRequest();

    QStringList requestSeparation(QString text);
};
#endif // SERVER_H
