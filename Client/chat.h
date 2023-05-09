#ifndef CHAT_H
#define CHAT_H

#include <QDialog>

namespace Ui {
class Chat;
}

class Chat : public QWidget
{
    Q_OBJECT

public:
    explicit Chat(QWidget *parent = nullptr);
    ~Chat();

public:
    Ui::Chat *ui;

public:
    void setMessageList(QList<QString> &fromUser, QList<QString> &toUser,
                        QList<QString> &text, QList<QString> &timestamp,
                        QString userName);
    void addMessage(QString timestamp);

private slots:
    void sendMessage();

signals:
    void sendMessageRequest(QString secondUser, QString message);
};

#endif // CHAT_H
