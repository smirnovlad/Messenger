#ifndef CLIENTSOCKET_H
#define CLIENTSOCKET_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include "client.h"

class Client;

class ClientSocket : public QObject
{
    Q_OBJECT
public:
    explicit ClientSocket(QObject *parent = nullptr, Client* client = nullptr);

private:
    Client *client;
    QTcpSocket *tcpSocket;

    QStringList requestSeparation(QString text, QString sep);

private slots:
    void getResponse();

public slots:
    void sendSignUpRequest(QString login, QString password);
    void sendLogInRequest(QString login, QString password);
    void sendContactListRequest(QString login, QString password);
    void sendChatRequest(QString firstUser, QString secondUser);

signals:

};

#endif // CLIENTSOCKET_H
