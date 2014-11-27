#include "src/util/qWidget.h"

#include <QWidget>
#include <QColor>
#include <QPalette>

void setBackgroundColor (QWidget *widget, const QColor &color)
{
	QPalette palette=widget->palette ();
	palette.setColor (widget->backgroundRole (), color);
	widget->setPalette (palette);
}
