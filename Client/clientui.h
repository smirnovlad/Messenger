#ifndef CLIENTUI_H
#define CLIENTUI_H

#include <QMainWindow>

#include "client.h"
#include "authorization.h"

QT_BEGIN_NAMESPACE
namespace Ui { class ClientUI; }
QT_END_NAMESPACE

class Client;

class ClientUI : public QMainWindow
{
    Q_OBJECT

public:
    ClientUI(QWidget *parent = nullptr, Client* client = nullptr);
    ~ClientUI();

private:
    Client *client;
    Ui::ClientUI *ui;
    QWidget *activeLayout;
    // Authorization *authorizationWindow;

private:
    void clearLayout();

public:
    void handleRegistration(QString message);

private slots:
    void log_in(QString login, QString password);
//    void sign_up(QString login, QString password);
    void setRegistrationWidget();
    void setAuthorizationWidget();
};
#endif // CLIENTUI_H
