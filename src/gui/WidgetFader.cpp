#include "src/gui/WidgetFader.h"

#include <iostream>

#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QWidget>
#include <QTimer>

WidgetFader::WidgetFader (QWidget *widget): QObject (NULL),
	_widget (widget), _waitTime (0), _fadeTime (0), _fadeInProgress (false)
{
}

WidgetFader::~WidgetFader ()
{
}

void WidgetFader::start ()
{
	// If the wait time is 0, start fading immediately. Otherwise, set up a
	// timer to start fading after the wait time.
	if (_waitTime==0)
		startFade ();
	else
		QTimer::singleShot (_waitTime, this, SLOT (startFade ()));
}

void WidgetFader::startFade ()
{
	if (_fadeInProgress)
		return;
	_fadeInProgress=true;

	// Create the opacity effect and assign it to the widget. The effect will
	// be deleted when the widget (which is the effect's parent) is deleted,
	// when a new effect is assigned to the widget, or after the animation ends.
	QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect (_widget);
	_widget->setGraphicsEffect (effect);

	// Create the animation for the opacity. The animation will be deleted when
	// the widget (which is the animation's parent) is is deleted, or after the
	// animation ends.
	QPropertyAnimation *animation = new QPropertyAnimation (effect, "opacity", _widget);

	// Close the widget at the end of the animation. The widget may choose to
	// ignore the close event.
	QObject::connect (animation, SIGNAL (finished ()), _widget, SLOT (close ()));

	// Delete the effect, the animation and the fader after the animation finishes.
	QObject::connect (animation, SIGNAL (finished ()), effect   , SLOT (deleteLater ()));
	QObject::connect (animation, SIGNAL (finished ()), animation, SLOT (deleteLater ()));
	QObject::connect (animation, SIGNAL (finished ()), this     , SLOT (deleteLater ()));

	// Setup and start the animation
	animation->setDuration (_fadeTime);
	animation->setStartValue (1);
	animation->setEndValue (0);
	animation->start ();
}

WidgetFader *WidgetFader::fadeOutAndClose (QWidget *widget, uint32_t fadeTime, uint32_t waitTime)
{
	// Will take care of its own deletion when the animation finishes
	WidgetFader *fader=new WidgetFader (widget);

	fader->_fadeTime=fadeTime;
	fader->_waitTime=waitTime;
	fader->start ();

	return fader;
}
