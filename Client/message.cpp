#include "message.h"
#include "ui_message.h"

Message::Message(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Message)
{
    ui->setupUi(this);
}

Message::~Message()
{
    delete ui;
}
