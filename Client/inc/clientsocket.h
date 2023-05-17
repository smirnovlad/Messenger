#ifndef CLIENTSOCKET_H
#define CLIENTSOCKET_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include "client.h"
#include "clientui.h"

class Client;
class CLientUI;

class ClientSocket: public QObject
{
Q_OBJECT

    friend class Client;
    friend class ClientUI;

private:
    Client *client;
    QTcpSocket *tcpSocket;

public:
    explicit ClientSocket(QObject *parent = nullptr, Client *client = nullptr);

private:
    QStringList requestSeparation(QString text, QString sep);
    void connectSocketToHost();

private slots:
    void getResponse();
    void sendSignUpRequest(QString login, QString password);
    void sendLogInRequest(QString login, QString password);
    void sendContactListRequest();
    void sendChatRequest(QString secondUser);
    void sendSendMessageRequest(QString secondUser, QString message);
    void sendLogOutRequest();
    void sendEditMessageRequest(QString receiver, int32_t messageId, QString editedMessage,
                                int32_t messageChatIndex);

};

#endif // CLIENTSOCKET_H
