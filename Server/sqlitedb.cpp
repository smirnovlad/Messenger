#include "sqlitedb.h"

#include <QDir>

SQLiteDB::SQLiteDB(QObject *parent) : QObject(parent)
{
    DB = QSqlDatabase::addDatabase("QSQLITE");

    qDebug() << "Current directory: " << QDir::currentPath();

    QString pathToDB = QString(QDir::currentPath() + "/../MessengerDB");
    DB.setDatabaseName(pathToDB);

    QFileInfo checkFile(pathToDB);
    if (checkFile.isFile()) {
        if (DB.open()) {
            qDebug() << "[+] Connected to Database File";
        } else {
            qDebug() <<"[!] Database File does not exist";
        }
    } else {
        qDebug() << "Error while connecting. Path to DB: " << pathToDB;
    }
}

SQLiteDB::~SQLiteDB()
{
    qDebug() << "Closing the connection to Database file on exist";
    DB.close();
}

int32_t SQLiteDB::findUser(QString login) {
    QSqlQuery query(DB);
    if(query.exec("SELECT id FROM Users WHERE login='" + login + "'")) {
        if (query.next()) {
            return query.value(0).toInt();
        } else {
            return -1;
        }
    } else {
        qDebug() << "Bad query";
    }
}

bool SQLiteDB::checkPassword(int32_t id, QString password)
{
    QSqlQuery query(DB);
    qDebug() << "string id = " << QString::number(id);
    if(query.exec("SELECT password FROM Users WHERE id=" + QString::number(id))) {
        if (query.next()) {
            qDebug() << "pass in DB: " << query.value(0).toString();
            qDebug() << "arg pass: " << password;
            return password == query.value(0).toString();
        } else {
            return false;
        }
    } else {
        qDebug() << "Bad query";
    }
}


int32_t SQLiteDB::addUser(QString login, QString password)
{
    QSqlQuery query(DB);
    query.prepare("INSERT INTO Users (login, password) "
                  "VALUES (:login, :password)");
    query.bindValue(":login", login);
    query.bindValue(":password", password);
    query.exec();
    if (query.exec("SELECT id FROM Users WHERE login='" + login + "'")) {
        if (query.next()) {
            return query.value(0).toInt();
        }
    }
}

void SQLiteDB::getContactList(QString& contactList)
{
    QSqlQuery query(DB);
    if (query.exec("SELECT id, login FROM Users")) {
        while (query.next()) {
            if (contactList.length()) {
                contactList.append(" /n ");
            }
            contactList.append(query.value(0).toString() + " /s " + query.value(1).toString());
        }
    }
}
