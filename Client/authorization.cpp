#include "authorization.h"
#include "ui_authorization.h"
#include "registration.h"
#include "ui_registration.h"

Authorization::Authorization(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Authorization)
    , registrationWindow(nullptr)
{
    ui->setupUi(this);
    this->setWindowTitle("Authorization");

    connect(ui->loginButton, SIGNAL(clicked()), this, SLOT(log_in()));
}

Authorization::~Authorization()
{
    delete ui;
}

void Authorization::log_in()
{
    QString login = ui->loginLabel->text();
    QString password = ui->passwordLabel->text();
    emit logInRequest(login, password);
}
