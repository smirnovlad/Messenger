#include "userlist.h"
#include "ui_userlist.h"

UserList::UserList(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::UserList)
{
    ui->setupUi(this);
}

UserList::~UserList()
{
    delete ui;
}
