/*
 * SpecialIntDelegate.h
 *
 *  Created on: 16.03.2010
 *      Author: Martin Herrmann
 */

#ifndef SPECIALINTDELEGATE_H_
#define SPECIALINTDELEGATE_H_

#include <QStyledItemDelegate>

class SpecialIntDelegate: public QStyledItemDelegate
{
	public:
		SpecialIntDelegate (int specialValue, const QString &specialValueText, const QString &suffix=QString (), QObject *parent=NULL);
		virtual ~SpecialIntDelegate ();

		virtual QString displayText (const QVariant &value, const QLocale &locale) const;

	private:
		int specialValue;
		QString specialValueText;
		QString suffix;
};

#endif
