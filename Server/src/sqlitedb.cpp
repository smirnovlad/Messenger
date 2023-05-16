#include "inc/sqlitedb.h"

#include <QDir>

SQLiteDB::SQLiteDB(QObject *parent)
    : QObject(parent)
{
    DB = QSqlDatabase::addDatabase("QSQLITE");

    //    qDebug() << "Current directory: " << QDir::currentPath();

    QString pathToDB = QString(QDir::currentPath() + "/../MessengerDB");
    DB.setDatabaseName(pathToDB);

    QFileInfo checkFile(pathToDB);
    if (checkFile.isFile()) {
        if (DB.open()) {
            qDebug() << "[+] Connected to Database File";
        }
        else {
            qDebug() << "[!] Database File does not exist";
        }
    }
    else {
        qDebug() << "DB: Error while connecting. Path to DB: " << pathToDB;
    }
}

SQLiteDB::~SQLiteDB()
{
    qDebug() << "DB: Closing the connection to Database file on exist.";
    DB.close();
}

QString SQLiteDB::getLoginByID(int32_t id)
{
    QSqlQuery query(DB);
    if (query.exec("SELECT login FROM Users WHERE id=" + QString::number(id))) {
        if (query.next()) {
            return query.value(0).toString();
        }
    }
    else {
        qDebug() << "DB: Bad query while getting login by id.";
    }
    return "";
}

QString SQLiteDB::getChatName(QString firstUser, QString secondUser)
{
    QString chatName = "Chat_" + firstUser + "_" + secondUser;
    if (DB.tables().contains(chatName)) {
        return chatName;
    }
    chatName = "Chat_" + secondUser + "_" + firstUser;
    if (DB.tables().contains(chatName)) {
        return chatName;
    }
    QSqlQuery query(DB);
    if (query.exec("CREATE TABLE " + chatName +
        " (id INTEGER PRIMARY KEY, fromID INTEGER NOT NULL, toID INTEGER NOT NULL, "
        "message TEXT NOT NULL, timestamp TEXT NOT NULL)")) {
        qDebug() << "New chat created: " << chatName;
    }
    else {
        qDebug() << "DB: Bad query while creating new chat: " << chatName;
    }
    return chatName;
}

int32_t SQLiteDB::findUser(QString login)
{
    QSqlQuery query(DB);
    if (query.exec("SELECT id FROM Users WHERE login='" + login + "'")) {
        if (query.next()) {
            return query.value(0).toInt();
        }
    }
    else {
        qDebug() << "DB: Bad query while finding user.";
    }
    return -1;
}

bool SQLiteDB::checkPassword(int32_t id, QString password)
{
    QSqlQuery query(DB);
    if (query.exec("SELECT password FROM Users WHERE id=" + QString::number(id))) {
        if (query.next()) {
            return password == query.value(0).toString();
        }
    }
    else {
        qDebug() << "DB: Bad query while checking password.";
    }
    return false;
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
    qDebug() << "DB: Bad query while getting new user id.";
    return -1;
}

void SQLiteDB::getContactList(QString &contactList)
{
    QSqlQuery query(DB);
    if (query.exec("SELECT id, login FROM Users")) {
        while (query.next()) {
            if (contactList.length()) {
                contactList.append(" /n ");
            }
            contactList.append(query.value(0).toString() + " /s " + query.value(1).toString());
        }
        return;
    }
}

void SQLiteDB::getMessageList(QString &messageList, QString firstUser, QString secondUser)
{
    QSqlQuery query(DB);
    QString chatName = getChatName(firstUser, secondUser);
    if (query.exec("SELECT * FROM " + chatName)) {
        while (query.next()) {
            if (messageList.length()) {
                messageList.append(" /n ");
            }
            messageList.append(getLoginByID(query.value(1).toInt()) + " /s " +
                getLoginByID(query.value(2).toInt()) + " /s " +
                query.value(3).toString() + " /s " +
                query.value(4).toString() + " /s " +
                query.value(0).toString());
        }
        return;
    }
    else {
        qDebug() << "DB: Bad query while selecting chat history.";
    }
}

void SQLiteDB::sendMessage(QString firstUser, QString secondUser, QString message, QString timestamp,
                           int32_t &messageId)
{
    QSqlQuery query(DB);
    QString chatName = getChatName(firstUser, secondUser);
    int32_t fromID = findUser(firstUser);
    int32_t toID = findUser(secondUser);

    query.prepare("INSERT INTO " + chatName + "(fromID, toID, message, timestamp) "
                                              "VALUES (:fromID, :toID, :message, :timestamp)");
    query.bindValue(":fromID", fromID);
    query.bindValue(":toID", toID);
    query.bindValue(":message", message);
    query.bindValue(":timestamp", timestamp);
    if (query.exec()) {
        messageId = query.lastInsertId().toInt();
        qDebug() << "DB: Message sent.";
    }
    else {
        qDebug() << "DB: Bad query while creating chat table for second user.";
    }
}

void SQLiteDB::editMessage(QString firstUser, QString secondUser, int32_t messageId, QString editedMessage)
{
    QSqlQuery query(DB);
    QString chatName = getChatName(firstUser, secondUser);
    if (query.exec("UPDATE " + chatName + " SET message='" + editedMessage +
        "' WHERE id=" + QString::number(messageId))) {
        qDebug() << "DB: Message updated.";
    }
    else {
        qDebug() << "DB: Bad query while updating message.";
    }
}

void SQLiteDB::updateToken(int32_t userId, QString token, QString timestamp)
{
    QSqlQuery query(DB);
    if (query.exec("SELECT id FROM Tokens WHERE userId=" + QString::number(userId))) {
        if (query.next()) {
            qDebug() << "Token updating";
            if (!query.exec("UPDATE Tokens SET token='" + token +
                "' WHERE userId=" + QString::number(userId))) {
                qDebug() << "Bad query while updating token.";
            }
            query.exec("UPDATE Tokens SET creationTimeStamp='" + timestamp +
                "' WHERE userId=" + QString::number(userId));
        }
        else {
            qDebug() << "Token creating";
            query.prepare("INSERT INTO Tokens (userId, token, creationTimeStamp) "
                          "VALUES (:userId, :token, :creationTimeStamp)");
            query.bindValue(":userId", userId);
            query.bindValue(":token", token);
            query.bindValue(":creationTimeStamp", timestamp);
            query.exec();
        }
        return;
    }
    else {
        qDebug() << "DB: Bad query while searching user token.";
    }
}

void SQLiteDB::getToken(int32_t userId, QString &token, QString &timestamp)
{
    QSqlQuery query(DB);
    if (query.exec("SELECT token, creationTimeStamp FROM Tokens WHERE userId=" + QString::number(userId))) {
        if (query.next()) {
            token = query.value(0).toString();
            timestamp = query.value(1).toString();
        }
        return;
    }
    else {
        qDebug() << "DB: Bad query while getting token.";
    }
}
