#ifndef CONFIG_H
#define CONFIG_H

#include <QTcpServer>

namespace server::config {

const QHostAddress HOST_ADDRESS = QHostAddress::Any;
const quint16 PORT = 55155;

const QString INCORRECT_LOGIN_CHARACTERS = "_' ,.!@#$%^&*()<>+=-/|~`\"";
const QString INCORRECT_PASSWORD_CHARACTERS = "/";

}

#endif //CONFIG_H
