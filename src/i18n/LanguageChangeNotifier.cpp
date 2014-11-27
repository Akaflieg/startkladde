#include "LanguageChangeNotifier.h"

#include <QEvent>

LanguageChangeNotifier *LanguageChangeNotifier::instance=NULL;

LanguageChangeNotifier::LanguageChangeNotifier ()
{
}

LanguageChangeNotifier::~LanguageChangeNotifier ()
{
}

void LanguageChangeNotifier::subscribe (QObject *subscriber)
{
	if (!instance)
		instance=new LanguageChangeNotifier ();

	connect (instance, SIGNAL (languageChanged ()), subscriber, SLOT (languageChanged ()));
}

void LanguageChangeNotifier::changeEvent (QEvent *event)
{
	if (event->type () == QEvent::LanguageChange)
	{
		emit languageChanged ();
	}
	else
		QWidget::changeEvent (event);
}

