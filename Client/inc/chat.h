#ifndef CHAT_H
#define CHAT_H

#include <QDialog>

namespace Ui
{
class Chat;
}

class Chat: public QWidget
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
                        QList<QString> &messageId, QString userName);
    void sendMessage(QString message, QString timestamp, int32_t messageId,
                     bool clearEditLine);
    void receiveMessage(QString message, QString timestamp, int32_t messageId);
    void editMessage(int32_t messageChatIndex, QString editedMessage);

private slots:
    void sendMessage();
    void showContextMenu(const QPoint &position);

signals:
    void sendMessageRequest(QString secondUser, QString message);
    void editMessageRequest(QString receiver, int32_t messageId, QString editedMessage,
                            int32_t messageChatIndex);
};

#endif // CHAT_H
