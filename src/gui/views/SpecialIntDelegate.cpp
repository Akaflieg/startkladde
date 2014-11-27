#include "SpecialIntDelegate.h"

#include <iostream>

SpecialIntDelegate::SpecialIntDelegate (int specialValue, const QString &specialValueText, const QString &suffix, QObject *parent):
	QStyledItemDelegate (parent),
	specialValue (specialValue), specialValueText (specialValueText), suffix (suffix)
{
}

SpecialIntDelegate::~SpecialIntDelegate ()
{
}

QString SpecialIntDelegate::displayText (const QVariant &value, const QLocale &locale) const
{
	if (value.toInt ()==specialValue)
		return specialValueText;
	else
		return QStyledItemDelegate::displayText (value, locale)+suffix;
}
