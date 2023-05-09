#ifndef CLIENTUI_H
#define CLIENTUI_H

#include <QMainWindow>
#include <QLayoutItem>
#include <QListWidget>

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
    QLayoutItem* getActiveItem();

public:
    void handleRegistration(QString result);
    void handleAuthorization(QString result);
    void handleContactList(const QList< QPair<QString, QString> >& result);
    void handleChat(QList<QString>& fromUser, QList<QString>& toUser,
                    QList<QString>& text, QList<QString>& timestamp);
    void handleSendMessage(QPair<QString, QString> result);

private slots:
    void setRegistrationWidget();
    void setAuthorizationWidget();
    void setUserListWidget();
    void setChatWidget(QString name);
    void sendChatRequest(QListWidgetItem* contact);
};
#endif // CLIENTUI_H
