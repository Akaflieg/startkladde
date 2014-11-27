#include "qMargins.h"

#include <QMargins>
#include <QWidget>
#include <QStyle>

QSize minimumSize (const QMargins &margins)
{
	return QSize (
		margins.left () + margins.right  (),
		margins.top  () + margins.bottom ());
}

QMargins marginsFromStyle (const QWidget *widget)
{
	int left  =widget->style ()->pixelMetric (QStyle::PM_LayoutLeftMargin  , 0, widget);
	int right =widget->style ()->pixelMetric (QStyle::PM_LayoutRightMargin , 0, widget);
	int top   =widget->style ()->pixelMetric (QStyle::PM_LayoutTopMargin   , 0, widget);
	int bottom=widget->style ()->pixelMetric (QStyle::PM_LayoutBottomMargin, 0, widget);

	return QMargins (left, top, right, bottom);
}

