#include "clientsocket.h"

ClientSocket::ClientSocket(QObject *parent, Client* client)
    : QObject(parent)
    , client(client)
    , tcpSocket(new QTcpSocket(this))
{
    QString hostname = "127.0.0.1";
    quint32 port = 55155;
    tcpSocket->abort();
    tcpSocket->connectToHost(hostname, port);

    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(getMessage()));
}

void ClientSocket::getMessage()
{
    QString typePacket;
    QString message;

    typePacket = tcpSocket->read(4);
    message = tcpSocket->readAll();

    QStringList commandList;
    QString fromname;

    enum class COMMAND { NONE,
                         USERLIST,
                         USERMSG,
                         NEWMSG,
                         FINDUSER,
                         INVITE,
                         GETFILE,
                         USINFO,
                         ISONLINE,
                         REGINFO
                       };
    COMMAND cmd = COMMAND::NONE;

    if (typePacket == "REGI") {
        cmd = COMMAND::REGINFO;
    }

    switch (cmd)
    {
    case COMMAND::REGINFO:
        client->clientUI->handleRegistration(message);

        break;
    }
}

void ClientSocket::sendSignUpRequest(QString login, QString password)
{
    qDebug() << "Sign Up requested. Login: " << login << ", password: " << password;
    QString request = "REGI";
    request.append(QString("%1 /s %2 /s ").arg(login).arg(password));
    tcpSocket->write(request.toUtf8());
}
