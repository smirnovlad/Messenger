#include "inc/client.h"

#include <QDir>

Client::Client(QObject *parent)
    : QObject(parent)
    , clientUI(new ClientUI(NULL, this))
    , clientSocket(new ClientSocket(NULL, this))
{}

Client::~Client()
{
    delete clientUI;
    delete clientSocket;
}

void Client::saveToken(QString token)
{
    QString tokenFileName = userLogin + "_token";
    QString pathToToken = QString(QDir::currentPath() + "/../../token/" + tokenFileName);
    QFile file;
    file.setFileName(pathToToken);
    if (file.open(QIODevice::WriteOnly|QIODevice::Text)) {
        QTextStream stream(&file);
        stream << token;
        file.close();
    } else {
        qDebug() << "Error while opening file with token";
    }
}

QString Client::getToken()
{
    QString tokenFileName = userLogin + "_token";
    QString pathToToken = QString(QDir::currentPath() + "/../../token/" + tokenFileName);
    QFile file;
    file.setFileName(pathToToken);
    QString token;
    if (file.open(QIODevice::ReadOnly|QIODevice::Text)) {
        QTextStream stream(&file);
        stream >> token;
        file.close();
    } else {
        qDebug() << "Error while opening file with token";
    }
    return token;
}