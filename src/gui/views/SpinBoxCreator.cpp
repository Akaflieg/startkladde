#include "SpinBoxCreator.h"

#include <iostream>
#include <climits>

#include <QSpinBox>

SpinBoxCreator::SpinBoxCreator (int minimum, const QString &specialValueText, const QString &suffix):
	minimum (minimum), specialValueText (specialValueText), suffix (suffix)
{
}

SpinBoxCreator::~SpinBoxCreator ()
{
}

QWidget *SpinBoxCreator::createWidget (QWidget *parent) const
{
	QSpinBox *spinBox=new QSpinBox (parent);
	spinBox->setFrame (false);
	spinBox->setMaximum (INT_MAX);
	spinBox->setMinimum (minimum);
	spinBox->setSpecialValueText (specialValueText);
	spinBox->setSuffix (suffix);
	spinBox->setAutoFillBackground (true);
	return spinBox;
}
