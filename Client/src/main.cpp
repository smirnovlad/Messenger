#include "inc/clientui.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Client client;
    return a.exec();
}
