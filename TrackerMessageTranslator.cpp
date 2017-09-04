#include "TrackerMessageTranslator.h"
#include "Tracker.h"

#include <QDataStream>

TrackerMessageTranslator::TrackerMessageTranslator()
{

}

TrackerMessageTranslator::~TrackerMessageTranslator()
{

}

int TrackerMessageTranslator::translate(const QByteArray& msg)
{
	QDataStream stream(msg);
	QString order;
	stream >> order;
	order = order.toLower();
	if (order == "starttracking")
	{
		return Tracker::START_TRACKING;
	}
	else if (order == "stoptracking")
	{
		return Tracker::STOP_TRACKING;
	}
	else if (order == "startmanufacturing")
	{
		return Tracker::START_MANUFACTURING;
	}
	else if (order == "stopmanufacturing")
	{
		return Tracker::STOP_MANUFACTURING;
	}
	else if (order == "startdiagnosing")
	{
		return Tracker::START_DIAGNOSING;
	}
	else if (order == "stopdiagnosing")
	{
		return Tracker::STOP_DIAGNOSING;
	}

	return -1;
}