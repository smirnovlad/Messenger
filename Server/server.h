#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QMessageBox>
#include <QTimer>
#include <QDateTime>
#include <QList>
#include <QPair>

#include "sqlitedb.h"
#include "user.h"

class Server : public QObject
{
    Q_OBJECT

public:
    Server(QObject *parent = nullptr);
    ~Server();

private:
    SQLiteDB *sqlitedb;
    QTcpServer *tcpServer;
    QList<User*> userConnections;

    quint32 nextBlockSize;

    QString getConnectionTimeStamp();

private:
    void handleRegistrationRequest(QTcpSocket *clientSocket, QString &login, QString &password);
    void sendRegistrationResponse(QTcpSocket *clientSocket, QString message);

    void handleAuthorizationRequest(QTcpSocket *clientSocket, QString &login, QString &password);
    void sendAuthorizationResponse(QTcpSocket *clientSocket, QString message);

    void handleContactListRequest(QTcpSocket *clientSocket, QString &login, QString &password);
    void sendContactListResponse(QTcpSocket *clientSocket, QString& contactList);

    void handleMessageListRequest(QTcpSocket *clientSocket, QString firstUser, QString secondUser);
    void sendMessageListResponse(QTcpSocket *clientSocket, QString& messageList);

private slots:
    void handleConnectionRequest();
    void getRequest();

    QStringList requestSeparation(QString text);
};
#endif // SERVER_H
