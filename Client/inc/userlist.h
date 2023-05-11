#ifndef USERLIST_H
#define USERLIST_H

#include <QDialog>
#include <QKeyEvent>
#include <QTcpSocket>
#include <QTime>

namespace Ui {
class UserList;
}

class UserList : public QWidget
{
    Q_OBJECT

public:
    explicit UserList(QWidget *parent = nullptr);
    ~UserList();

public:
    Ui::UserList *ui;

    void setContactList(QList< QPair<QString, QString> > contactList);
};

#endif // USERLIST_H
