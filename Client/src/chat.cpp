#include "inc/chat.h"
#include "./ui_chat.h"
#include "inc/message.h"
#include "./ui_message.h"

#include <QDebug>
#include <QMenu>
#include <QInputDialog>

Chat::Chat(QWidget *parent)
    :
    QWidget(parent),
    ui(new Ui::Chat)
{
    ui->setupUi(this);
    ui->messageList->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(this->ui->sendButton, SIGNAL(clicked()), this, SLOT(sendMessage()));
    connect(this->ui->messageList, SIGNAL(customContextMenuRequested(const QPoint &)),
            this, SLOT(showContextMenu(const QPoint &)));
}

Chat::~Chat()
{
    delete ui;
}

void Chat::setMessageList(QList<QString> &fromUser, QList<QString> &toUser,
                          QList<QString> &text, QList<QString> &timestamp,
                          QList<QString> &messageId, QString userName)
{
    for (uint32_t i = 0; i < text.count(); ++i) {
        Message *messageWidget = new Message(this);
        messageWidget->ui->messageLabel->setText(text[i]);
        messageWidget->ui->timestampLabel->setText(timestamp[i]);
        messageWidget->messageId = messageId[i].toInt();
        if (userName == fromUser[i]) {
            messageWidget->ui->messageLabel->setAlignment(Qt::AlignRight);
            //messageWidget->ui->editButton->setFixedHeight(10);
        }
        else {
            messageWidget->ui->messageLabel->setAlignment(Qt::AlignLeft);
//            messageWidget->ui->editButton->setVisible(false);
//            messageWidget->ui->messageSpacer->changeSize(0, 0);
//            messageWidget->ui->verticalSpacer->changeSize(0, 0);
//            messageWidget->ui->horizontalSpacer->changeSize(0, 0);
        }
        // qDebug() << "info: " << userWidget->ui->userID->text() << userWidget->ui->userLogin->text();
        QListWidgetItem *item = new QListWidgetItem(ui->messageList);
        item->setSizeHint(messageWidget->sizeHint());
        ui->messageList->addItem(item);
        ui->messageList->setItemWidget(item, messageWidget);
//        messageWidget->ui->pushButton->setFixedHeight(20);
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

void Chat::sendMessage(QString message, QString timestamp, int32_t messageId, bool clearEditLine)
{
    Message *messageWidget = new Message(this);

    messageWidget->ui->messageLabel->setText(message);
    messageWidget->ui->timestampLabel->setText(timestamp);
    messageWidget->ui->messageLabel->setAlignment(Qt::AlignRight);
    messageWidget->messageId = messageId;
//    messageWidget->ui->pushButton->setFixedHeight(20);

    QListWidgetItem *item = new QListWidgetItem(ui->messageList);
    item->setSizeHint(messageWidget->sizeHint());
    ui->messageList->addItem(item);
    ui->messageList->setItemWidget(item, messageWidget);

    if (clearEditLine) {
        ui->lineEdit->setText("");
    }

    ui->messageList->scrollToBottom();
}

void Chat::receiveMessage(QString message, QString timestamp, int32_t messageId)
{
    Message *messageWidget = new Message(this);

    messageWidget->ui->messageLabel->setText(message);
    messageWidget->ui->timestampLabel->setText(timestamp);
    messageWidget->ui->messageLabel->setAlignment(Qt::AlignLeft);
    messageWidget->messageId = messageId;
//    messageWidget->ui->editButton->setVisible(false);
//    messageWidget->ui->pushButton->setFixedHeight(20);

    QListWidgetItem *item = new QListWidgetItem(ui->messageList);
    item->setSizeHint(messageWidget->sizeHint());
    ui->messageList->addItem(item);
    ui->messageList->setItemWidget(item, messageWidget);

    ui->messageList->scrollToBottom();
}

void Chat::showContextMenu(const QPoint &position)
{
    QListWidgetItem *messageItem = ui->messageList->itemAt(position);
    Message *messageWidget = static_cast<Message *>(ui->messageList->itemWidget(messageItem));
    QPoint item = ui->messageList->mapToGlobal(position);
    QMenu *menu = new QMenu(this);
    if (messageWidget->ui->messageLabel->alignment() & Qt::AlignRight) {
        menu->addAction(new QAction("Edit", this));
    }
    QAction *rightClickItem = menu->exec(item);
    if (rightClickItem &&
        rightClickItem->text() == "Edit") {
        QString message = messageWidget->ui->messageLabel->text();
        bool ok;
        QString editedMessage = QInputDialog::getText(this, tr("Edit message"),
                                                      tr("New message:"), QLineEdit::Normal,
                                                      message, &ok);
        if (ok && !editedMessage.isEmpty()) {
            QString receiver = this->ui->userNameLabel->text();
            int32_t messageId = messageWidget->messageId;
            int32_t messageChatIndex = this->ui->messageList->row(messageItem);
            emit editMessageRequest(receiver, messageId, editedMessage, messageChatIndex);
        }
//      ui->messageList->takeItem(ui->messageList->indexAt(position).row());
    }
}

void Chat::editMessage(int32_t messageChatIndex, QString editedMessage)
{
    qDebug() << "items count: " << ui->messageList->count();
    QListWidgetItem *messageItem = ui->messageList->item(messageChatIndex);
    Message *messageWidget = static_cast<Message*>(ui->messageList->itemWidget(messageItem));
    messageWidget->ui->messageLabel->setText(editedMessage);
}