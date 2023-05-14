#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QMessageBox>
#include <QObject>

#include "clientui.h"
#include "clientsocket.h"

class ClientUI;
class ClientSocket;

class Client: public QObject
{
Q_OBJECT

friend class ClientSocket;
friend class ClientUI;

public:
    explicit Client(QObject *parent = nullptr);
    ~Client();

private:
    void saveToken(QString token);
    QString getToken();

private:
    ClientSocket *clientSocket;
    ClientUI *clientUI;
    QString userLogin;

};

#endif // CLIENT_H
