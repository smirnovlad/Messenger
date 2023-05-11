#ifndef REGISTRATION_H
#define REGISTRATION_H

#include <QDialog>
#include <QKeyEvent>
#include <QTcpSocket>
#include <QTime>


QT_BEGIN_NAMESPACE
namespace Ui { class Registration; }
QT_END_NAMESPACE

class Registration : public QWidget
{
    Q_OBJECT

public:
    explicit Registration(QWidget *parent = nullptr);
    ~Registration();

public:
    Ui::Registration *ui;

private slots:
    void sign_up();

signals:
  void signUpRequest(QString login, QString password);
};

#endif // REGISTRATION_H
