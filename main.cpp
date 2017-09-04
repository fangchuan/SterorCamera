#include "TrackerServer.h"
#include <QtCore/QCoreApplication>

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	TrackerServer server;
	server.listen(QHostAddress::Any, 34210);

	return a.exec();
}
