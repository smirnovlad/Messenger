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

private slots:
    void getMessage();

public slots:
    void sendSignUpRequest(QString login, QString password);

signals:

};

#endif // CLIENTSOCKET_H
