#include "chat.h"
#include "./ui_chat.h"
#include "message.h"
#include "./ui_message.h"

#include <QDebug>

Chat::Chat(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Chat)
{
    ui->setupUi(this);

    connect(this->ui->sendButton, SIGNAL(clicked()), this, SLOT(sendMessage()));
}

Chat::~Chat()
{
    delete ui;
}

void Chat::setMessageList(QList<QString> &fromUser, QList<QString> &toUser,
                          QList<QString> &text, QList<QString> &timestamp,
                          QString userName)
{
    for (uint32_t i = 0; i < text.count(); ++i) {
        Message* messageWidget = new Message(this);
        if (userName == fromUser[i]) {
            messageWidget->ui->messageFromLabel->setText(text[i]);
            messageWidget->ui->timestampFromLabel->setText(timestamp[i]);
            messageWidget->ui->messageToLabel->setText("");
            messageWidget->ui->timestampToLabel->setText("");
        } else {
            messageWidget->ui->messageToLabel->setText(text[i]);
            messageWidget->ui->timestampToLabel->setText(timestamp[i]);
            messageWidget->ui->messageFromLabel->setText("");
            messageWidget->ui->timestampFromLabel->setText("");
        }
        // qDebug() << "info: " << userWidget->ui->userID->text() << userWidget->ui->userLogin->text();

        QListWidgetItem *item = new QListWidgetItem(ui->messageList);
        item->setSizeHint(messageWidget->sizeHint());
        ui->messageList->addItem(item);
        ui->messageList->setItemWidget(item, messageWidget);
    }
    ui->messageList->scrollToBottom();
}

void Chat::sendMessage()
{
    QString message = ui->lineEdit->text();
    if (message.length()) {
        emit sendMessageRequest(ui->userNameLabel->text(), message);
    }
}

void Chat::sendMessage(QString message, QString timestamp)
{
    Message* messageWidget = new Message(this);

    messageWidget->ui->messageFromLabel->setText(message);
    messageWidget->ui->messageToLabel->setText("");
    messageWidget->ui->timestampFromLabel->setText(timestamp);
    messageWidget->ui->timestampToLabel->setText("");

    QListWidgetItem *item = new QListWidgetItem(ui->messageList);
    item->setSizeHint(messageWidget->sizeHint());
    ui->messageList->addItem(item);
    ui->messageList->setItemWidget(item, messageWidget);

    ui->lineEdit->setText("");

    ui->messageList->scrollToBottom();
}

void Chat::receiveMessage(QString message, QString timestamp)
{
    Message* messageWidget = new Message(this);

    messageWidget->ui->messageFromLabel->setText("");
    messageWidget->ui->messageToLabel->setText(message);
    messageWidget->ui->timestampFromLabel->setText("");
    messageWidget->ui->timestampToLabel->setText(timestamp);

    QListWidgetItem *item = new QListWidgetItem(ui->messageList);
    item->setSizeHint(messageWidget->sizeHint());
    ui->messageList->addItem(item);
    ui->messageList->setItemWidget(item, messageWidget);

    ui->messageList->scrollToBottom();
}
