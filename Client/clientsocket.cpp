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

    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(getResponse()));
}

QStringList ClientSocket::requestSeparation(QString text, QString sep)
{
    QString outStr =  text;
    QStringList list = outStr.split(sep);

    return list;
}

void ClientSocket::getResponse()
{
    QString packetType;
    QString message;

    packetType = tcpSocket->read(4);
    message = tcpSocket->readAll();

    QStringList commandList;
    QString fromname;

    enum class COMMAND { NONE,
                         REGISTRATION_RESPONSE,
                         AUTHORIZATION_RESPONSE,
                         CONTACT_LIST_RESPONSE
                       };
    COMMAND cmd = COMMAND::NONE;

    if (packetType == "REGI") {
        cmd = COMMAND::REGISTRATION_RESPONSE;
    } else if (packetType == "AUTH") {
        cmd = COMMAND::AUTHORIZATION_RESPONSE;
    } else if (packetType == "CTCS") {
        cmd = COMMAND::CONTACT_LIST_RESPONSE;
    }

    switch (cmd)
    {
        case COMMAND::REGISTRATION_RESPONSE:
        {
            client->clientUI->handleRegistration(message);
            break;
        }

        case COMMAND::AUTHORIZATION_RESPONSE:
        {
            client->clientUI->handleAuthorization(message);
            break;
        }

        case COMMAND::CONTACT_LIST_RESPONSE:
        {
            QList< QPair<QString, QString> > result;
            QStringList pairs = requestSeparation(message, " /n ");
            for (QString& pairString: pairs) {
                // qDebug() << "pairString: " << pairString;
                QStringList pair = requestSeparation(pairString, " /s ");
                result.push_back({pair[0], pair[1]});
            }
            client->clientUI->handleContactList(result);
            break;
        }
    }
}

void ClientSocket::sendSignUpRequest(QString login, QString password)
{
    qDebug() << "Sign Up requested. Login: " << login << ", password: " << password;
    QString request = "REGI";
    request.append(QString("%1 /s %2").arg(login).arg(password));
    tcpSocket->write(request.toUtf8());
}

void ClientSocket::sendLogInRequest(QString login, QString password)
{
    qDebug() << "Log in requested. Login: " << login << ", password: " << password;
    QString request = "AUTH";
    request.append(QString("%1 /s %2").arg(login).arg(password));
    tcpSocket->write(request.toUtf8());
}

void ClientSocket::sendContactListRequest(QString login, QString password)
{
    qDebug() << "Contact list requested. Login: " << login << ", password: " << password;
    QString request = "CTCS";
    request.append(QString("%1 /s %2").arg(login).arg(password));
    tcpSocket->write(request.toUtf8());
}
