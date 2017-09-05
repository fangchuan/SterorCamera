#include <QCoreApplication>
#include "TrackerServer.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    TrackerServer server;


    return a.exec();
}
