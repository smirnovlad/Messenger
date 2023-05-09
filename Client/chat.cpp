#include "chat.h"
#include "./ui_chat.h"

#include <QDebug>

Chat::Chat(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Chat)
{
    ui->setupUi(this);
}

Chat::~Chat()
{
    delete ui;
}

void Chat::setMessageList(QList<QString> &fromUser, QList<QString> &toUser,
                          QList<QString> &text, QList<QString> &timestamp)
{

}
