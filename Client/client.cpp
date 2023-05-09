#include "client.h"

enum class ErrorCode {
    BAD_LOGIN_OR_PASSWORD_LENGTH = 1,
    ILLEGAL_CHARACTERS = 2
};

Client::Client(QObject *parent)
    : QObject(parent)
    , clientUI(new ClientUI(NULL, this))
    , clientSocket(new ClientSocket(NULL, this))
{}

int Client::logIn(QString login, QString password)
{
    qDebug() << "Log In requested. Login: " << login << ", password: " << password;
    qDebug() << "Checking data to log in...";
    // TODO
    return 0;
}
