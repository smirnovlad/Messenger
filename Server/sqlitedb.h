#ifndef SQLITEDB_H
#define SQLITEDB_H

#include <QObject>
#include <QtSql>
#include <QPair>

typedef QList <QPair <QString, QList<QPair <QString, QString> > > > ChatListVector;

class SQLiteDB : public QObject
{
    Q_OBJECT
public:
    explicit SQLiteDB(QObject *parent = nullptr);
    ~SQLiteDB();

private:
    QSqlDatabase DB;

public:
    int32_t findUser(QString login);
    bool checkPassword(int32_t id, QString password);
    int32_t addUser(QString login, QString password);
    void getContactList(QString& contactList);
};

#endif // SQLITEDB_H
