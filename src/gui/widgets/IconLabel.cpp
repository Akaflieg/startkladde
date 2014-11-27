#include "IconLabel.h"

#include <QIcon>

/**
 * Creates a new IconLabel instance
 *
 * @param parent passed to the QWidget constructor
 * @param f passed to the QWidget constructor
 * @see QWidget::QWidget
 */
IconLabel::IconLabel (QWidget *parent, Qt::WindowFlags f): QWidget (parent, f)
{
	ui.setupUi (this);

	connect (ui.textLabel, SIGNAL (linkActivated (const QString &)), this, SIGNAL (linkActivated (const QString &)));
	connect (ui.textLabel, SIGNAL (linkHovered   (const QString &)), this, SIGNAL (linkHovered   (const QString &)));
}

IconLabel::~IconLabel()
{
}

/**
 * Sets the icon label to a standard icon
 *
 * @param standardPixmap the standard pixmap value
 * @see QStyle::StandardPixmap
 */
void IconLabel::setIcon (QStyle::StandardPixmap standardPixmap)
{
	// Don't set the icon if it is already current - avoid unnecessary layout
	// updates and, consequently, widget repaints
	if (icon.isValid () && icon.getValue ()==standardPixmap)
		return;
	icon.setValue (standardPixmap);

	QStyle *style=QApplication::style ();
	QIcon icon=style->standardIcon (standardPixmap, 0, this);
	ui.iconLabel->setPixmap (icon.pixmap (16));
}

/**
 * Sets the text of the text label to text
 */
void IconLabel::setText (const QString &text)
{
	ui.textLabel->setText (text);
}

/**
 * Returns a pointer to the text label
 *
 * The pointer may be used to manipulate the text label. If only a text is to be
 * set, the setText method is more convenient.
 *
 * The returned label may not be deleted.
 */
QLabel *IconLabel::textLabel ()
{
	return ui.textLabel;
}

void IconLabel::show (QStyle::StandardPixmap icon, const QString &text)
{
	setIcon (icon);
	setText (text);
	show ();
}

void IconLabel::showWarning (const QString &text)
{
	show (QStyle::SP_MessageBoxWarning, text);
}

void IconLabel::showCritical (const QString &text)
{
	show (QStyle::SP_MessageBoxCritical, text);
}

void IconLabel::showInformation (const QString &text)
{
	show (QStyle::SP_MessageBoxInformation, text);
}
