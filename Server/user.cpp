#include "user.h"

User::User()
{
    this->login = "";
    socket = nullptr;
}

void User::setUserName(QString name)
{
    this->login = name;
}

QString User::getUserName()
{
    return login;
}

void User::setSocket(QTcpSocket* socket)
{
    this->socket = socket;
}

QTcpSocket* User::getSocket()
{
    return socket;
}


User::~User()
{

}
