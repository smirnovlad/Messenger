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

private:
    Ui::UserList *ui;
};

#endif // USERLIST_H
