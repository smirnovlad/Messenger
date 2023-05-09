#include "sqlitedb.h"

#include <QDir>

SQLiteDB::SQLiteDB(QObject *parent) : QObject(parent)
{
    DB = QSqlDatabase::addDatabase("QSQLITE");

    // qDebug() << "Current directory: " << QDir::currentPath();

    QString pathToDB = QString(QDir::currentPath() + "/MessengerDB.sqlite");
    DB.setDatabaseName(pathToDB);

    QFileInfo checkFile(pathToDB);
    if (checkFile.isFile()) {
        if (DB.open()) {
            qDebug() << "[+] Connected to Database File";
        } else {
            qDebug() <<"[!] Database File does not exist";
        }
    } else {
        // TODO
    }
}

SQLiteDB::~SQLiteDB()
{
    qDebug() << "Closing the connection to Database file on exist";
    DB.close();
}
