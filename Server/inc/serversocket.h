#ifndef SERVERSOCKET_H
#define SERVERSOCKET_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include "server.h"

class Server;

class ServerSocket: public QObject
{
Q_OBJECT

    friend class Server;

public:
    explicit ServerSocket(QObject *parent = nullptr, Server *server = nullptr);

private:
    Server *server;
    QTcpServer *tcpServer;

private:
    QStringList requestSeparation(QString text, QString sep);

    void sendRegistrationResponse(QTcpSocket *clientSocket, QString result, QString message);
    void sendAuthorizationResponse(QTcpSocket *clientSocket, QString result, QString message);
    void sendContactListResponse(QTcpSocket *clientSocket, QString message);
    void sendMessageListResponse(QTcpSocket *clientSocket, QString message);
    void sendSendMessageResponse(QTcpSocket *clientSocket, QString result, QString message,
                                 QString firstUser, QString secondUser, QString timestamp,
                                 int32_t messageId, bool toActualSender);
    void sendLogOutResponse(QTcpSocket *clientSocket, QString result);
    void sendEditMessageResponse(QTcpSocket *clientSocket, QString result,
                                 QString sender, QString receiver,
                                 QString editedMessage, int32_t messageChatIndex);

private slots:
    void handleConnectionRequest();
    void handleDisconnection();
    void getRequest();

};

#endif // SERVERSOCKET_H
