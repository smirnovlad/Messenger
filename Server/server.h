#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QMessageBox>
#include <QTimer>
#include <QDateTime>
#include <QList>
#include <QFile>
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

public:
    void sendRegistrationResponse(QTcpSocket *clientSocket, QString message);

private slots:
    void handleConnectionRequest();
    void getRequest();

    void handleRegistrationRequest(QTcpSocket *clientSocket, QString &login, QString &password);
    QStringList requestSeparation(QString text);
};
#endif // SERVER_H
