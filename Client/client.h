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

public:
    QString userLogin;

};

#endif // CLIENT_H
