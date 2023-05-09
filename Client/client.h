#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QMessageBox>
#include <QObject>

#include "clientui.h"
#include "clientsocket.h"

class ClientUI;
class ClientSocket;

class Client : public QObject
{
    Q_OBJECT
public:
    explicit Client(QObject *parent = nullptr);

public:
    ClientSocket *clientSocket;
    ClientUI *clientUI;
    quint32 nextBlockSize;

// private slots:

public:
    int sendMessage();
    int logIn(QString login, QString password);
};

#endif // CLIENT_H
