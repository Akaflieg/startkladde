#include "LongitudeInput.h"

LongitudeInput::LongitudeInput (QWidget *parent):
	QWidget (parent)
{
	ui.setupUi (this);

	setFocusProxy (ui.degreesInput);
}

LongitudeInput::~LongitudeInput ()
{
}

void LongitudeInput::setLongitude (const Longitude &longitude)
{
	Longitude lon=longitude.normalized ();

	unsigned int degrees, minutes, seconds;
	bool positive;

	lon.toDms (degrees, minutes, seconds, positive);

	ui.degreesInput->setValue (degrees);
	ui.minutesInput->setValue (minutes);
	ui.secondsInput->setValue (seconds);
	ui.signInput->setCurrentIndex (positive?0:1);
}

Longitude LongitudeInput::getLongitude () const
{
	return Longitude (
		ui.degreesInput->value (),
		ui.minutesInput->value (),
		ui.secondsInput->value (),
		ui.signInput->currentIndex ()==0
		);
}

void LongitudeInput::changeEvent (QEvent *event)
{
	if (event->type () == QEvent::LanguageChange)
	{
		ui.retranslateUi (this);
		adjustSize ();
	}
	else
		QWidget::changeEvent (event);
}
