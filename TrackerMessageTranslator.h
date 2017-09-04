#ifndef TRACKERMESSAGETRANSLATOR_H
#define TRACKERMESSAGETRANSLATOR_H

#include <QByteArray>

class TrackerMessageTranslator
{
public:
	virtual ~TrackerMessageTranslator();

	static int translate(const QByteArray& msg);

private:
	explicit TrackerMessageTranslator();
};

#endif // TRACKERMESSAGETRANSLATOR_H
