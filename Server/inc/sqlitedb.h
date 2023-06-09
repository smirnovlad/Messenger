#ifndef SQLITEDB_H
#define SQLITEDB_H

#include <QObject>
#include <QtSql>
#include <QPair>

typedef QList<QPair<QString, QList<QPair<QString, QString> > > > ChatListVector;

class SQLiteDB: public QObject
{
Q_OBJECT

public:
    explicit SQLiteDB(QObject *parent = nullptr);
    ~SQLiteDB();

private:
    QSqlDatabase DB;
    QString getLoginByID(int32_t id);
    QString getChatName(QString firstUser, QString secondUser);

public:
    int32_t findUser(QString login);
    bool checkPassword(int32_t id, QString password);
    int32_t addUser(QString login, QString password);
    void getContactList(QString &contactList);
    void getMessageList(QString &messageList, QString firstUser, QString secondUser);
    void sendMessage(QString sender, QString recipient, QString message, QString timestamp,
                     int32_t &messageId);
    void editMessage(QString firstUser, QString secondUser, int32_t messageId, QString editedMessage);

    void updateToken(int32_t userId, QString token, QString timestamp);
    void getToken(int32_t userId, QString &token, QString &timestamp);
};

#endif // SQLITEDB_H
