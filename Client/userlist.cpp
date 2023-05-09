#include "userlist.h"
#include "./ui_userlist.h"
#include "user.h"
#include "./ui_user.h"

#include <QListWidget>

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

void UserList::setContactList(QList<QPair<QString, QString>> contactList)
{
    for (auto& contact: contactList) {
        User* userWidget = new User(this);
        userWidget->ui->IDLabel->setText("ID: " + contact.first);
        userWidget->ui->loginLabel->setText("Login: " + contact.second);

        // qDebug() << "info: " << userWidget->ui->userID->text() << userWidget->ui->userLogin->text();

        QListWidgetItem *item = new QListWidgetItem(ui->userList);
        item->setSizeHint(userWidget->sizeHint());
        ui->userList->addItem(item);
        ui->userList->setItemWidget(item, userWidget);
    }
}
