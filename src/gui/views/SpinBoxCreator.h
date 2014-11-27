/*
 * SpinBoxCreator.h
 *
 *  Created on: 16.03.2010
 *      Author: Martin Herrmann
 */

#ifndef SPINBOXCREATOR_H_
#define SPINBOXCREATOR_H_

#include <QStandardItemEditorCreator>

class QSpinBox;

/**
 * Creates a QSpinBox with a minimum, special value text and a suffix
 *
 * For an example of how to use this, see SettingsWindow#SettingsWindow. This
 * functionality could also be integrated into SpecialIntDelegate.
 */
class SpinBoxCreator: public QStandardItemEditorCreator<QSpinBox>
{
	public:
		SpinBoxCreator (int minimum, const QString &specialValueText, const QString &suffix=QString ());
		virtual ~SpinBoxCreator ();

		virtual QWidget *createWidget (QWidget *parent) const;

	private:
		int minimum;
		QString specialValueText;
		QString suffix;
};

#endif
