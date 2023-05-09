#ifndef MESSAGE_H
#define MESSAGE_H

#include <QWidget>

namespace Ui {
class Message;
}

class Message : public QWidget
{
    Q_OBJECT

public:
    explicit Message(QWidget *parent = nullptr);
    ~Message();

public:
    Ui::Message *ui;
};

#endif // MESSAGE_H
