#include "SkLabel.h"

#include <iostream>

#include "src/util/color.h"

// ******************
// ** Construction **
// ******************

SkLabel::SkLabel (QWidget *parent, Qt::WindowFlags f):
	QLabel (parent, f),
	concealed (false),
	error (false),
	defaultBackgroundColor (255, 255, 255),
	useDefaultBackgroundColor (false),
	errorColor (255, 0, 0)
{
}

SkLabel::SkLabel (const QString &text, QWidget *parent, Qt::WindowFlags f):
	QLabel (text, parent, f),
	concealed (false),
	error (false),
	defaultBackgroundColor (255, 255, 255),
	useDefaultBackgroundColor (false),
	errorColor (255, 0, 0)
{
}


// *********************
// ** Property access **
// *********************

void SkLabel::setDefaultBackgroundColor (const QColor &color)
{
	defaultBackgroundColor=color;
	useDefaultBackgroundColor=true;

	updateColors ();
}

void SkLabel::resetDefaultBackgroundColor ()
{
	useDefaultBackgroundColor=false;

	updateColors ();
}

QColor SkLabel::getDefaultBackgroundColor ()
{
	return defaultBackgroundColor;
}

void SkLabel::setDefaultForegroundColor (const QColor &color)
{
	// TODO: this will use the parent's foreground color as this widget's
	// foreground color, which means that it will not be updated if the
	// style is changed.
	defaultForegroundColor=color;
	updateColors ();
}

void SkLabel::resetDefaultForegroundColor ()
{
	defaultForegroundColor=QColor ();
	updateColors ();
}

QColor SkLabel::getDefaultForegroundColor ()
{
	return defaultForegroundColor;
}

void SkLabel::setConcealed (bool concealed)
{
	this->concealed=concealed;
	updateColors ();
}

void SkLabel::setError (bool error)
{
	this->error=error;
	updateColors ();
}

/**
 * Use setDefaultForegroundColor instead
 */
// TODO remove
void SkLabel::setPaletteForegroundColor (const QColor &color)
{
	QPalette palette;
	palette.setColor (foregroundRole (), color);
	setPalette(palette);
}

/**
 * Use setDefaultBackgroundColor instead
 */
// TODO remove
void SkLabel::setPaletteBackgroundColor (const QColor &color)
{
	QPalette palette;
	palette.setColor (backgroundRole (), color);
	setPalette(palette);
}


// ******************
// ** State update **
// ******************

void SkLabel::updateColors ()
{
	QPalette p=palette ();

	if (concealed)
	{
		// Concealed => foreground and background like parent background
		setAutoFillBackground (true);

        p.setColor (QPalette::WindowText, parentWidget ()->palette ().window ().color ());
        p.setColor (QPalette::Window    , parentWidget ()->palette ().window ().color ());
	}
	else if (error)
	{
		// Error => given error background, same foreground as parent
		setAutoFillBackground (true);

        p.setColor (QPalette::WindowText, parentWidget ()->palette ().windowText ().color ());
		p.setColor (QPalette::Window    , errorColor);
	}
	else
	{
		if (defaultForegroundColor.isValid ())
			p.setColor (QPalette::WindowText, defaultForegroundColor);
		else
            p.setColor (QPalette::WindowText, parentWidget ()->palette ().windowText ().color ());

		if (useDefaultBackgroundColor)
			p.setColor (QPalette::Window    , defaultBackgroundColor);
		else
            p.setColor (QPalette::Window    , parentWidget ()->palette ().window ().color ());

		setAutoFillBackground (useDefaultBackgroundColor);
	}

	setPalette (p);
}


// **************
// ** Contents **
// **************

void SkLabel::setNumber (int number)
{
	setText (QString::number (number));
}

void SkLabel::setNumber (float number)
{
	setText (QString::number (number));
}

void SkLabel::setNumber (double number)
{
	setText (QString::number (number));
}

void SkLabel::setText (const QString &text, Qt::TextFormat format)
{
	setTextFormat (format);
	setText (text);
}

void SkLabel::setTextAndColor (const QString &text, const QColor &color)
{
	setText (text);
	setDefaultForegroundColor (color);
	updateColors ();
}

void SkLabel::setTextAndDefaultColor (const QString &text)
{
	setTextAndColor (text, QColor ());
}
