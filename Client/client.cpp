#include "client.h"

Client::Client(QObject *parent)
    : QObject(parent)
    , clientUI(new ClientUI(NULL, this))
    , clientSocket(new ClientSocket(NULL, this))
{}
