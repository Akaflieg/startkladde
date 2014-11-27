#include "NotificationWidget.h"

#include <iostream>

#include <QApplication>
#include <QCloseEvent>
#include <QEvent>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QRect>
#include <QStyle>

#include "src/util/qRect.h"
#include "src/util/qSize.h"

// The geometry of this widget is determined by the bubble size (determined, in
// turn, by the contents widget's size hint and the margins) and the position
// of the arrow tip. The arrow tip may be on either side of the bubble both
// horizontally and vertically, but since the arrow base is always drawn on the
// left side of the bubble, the only useful positions of the arrow tip are those
// on the left side of the bubble, either above, beside or below the bubble.
//
// If the arrow is somewhere to the left of the bubble:
// .-------.--------.
// | arrow | bubble |
// |  tip  |        |
// '-------'--------'
//
// If the arrow is to the left and above the bubble:
// .----------------.
// | arrow          |
// |  tip           |
// |       .--------.
// |       | bubble |
// |       |        |
// '-------'--------'
//
// If the arrow is to the left and below the bubble:
// .--------.--------.
// |        | bubble |
// |        |        |
// |        '--------'
// | arrow           |
// |  tip            |
// '-----------------'
//
// If the arrow is not to the left, the layout is similar, but the arrow will
// overlap the bubble.
//
// Note that the arrow tip position is specified relative to the upper left
// corner of the bubble. This scheme allows us to draw the arrow on any side of
// the bubble (though this is not implemented at the moment), but the position
// of the arrow tip must be specified relative to the top left corner of the
// bubble.
//
// The geometry concepts for this widget are much more complicated than it may
// seem. In particular, allowing the resizing of this widget does not seem to
// make a lot of sense because the size may be given either by the bubble (or
// contents size) or by the arrow position.

// ******************
// ** Construction **
// ******************

/**
 * Creates a new notification widget as a child of the given parent widget
 *
 * No contents widget will be set. Therefore, the bubble will be empty. The size
 * of the bubble will be just large enough for the rounded corners and the arrow
 * base on the left side. The initial position of both the bubble and the arrow
 * is undefined.
 *
 * You'll generally want to use setContents() or setText() first, and moveTo()
 * after that.
 */
NotificationWidget::NotificationWidget (QWidget *parent, Qt::WindowFlags f):
	QWidget (parent, f),
	_contents (NULL), bubbleColor (QColor (0, 0, 0, 191))
{
	// Make label text white and the widget background a rather transparent
	// black (note that this is the widget background, which is not drawn unless
	// the autoFillBackground property is set to true; the bubble is drawn in
	// the bubble color).
	QPalette widgetPalette=palette ();
	widgetPalette.setColor (QPalette::WindowText, Qt::white);
	widgetPalette.setColor (backgroundRole ()   , QColor (0, 0, 0, 63));
	setPalette (widgetPalette);

	// Get the default margin from the style
	_margin=style ()->pixelMetric (QStyle::PM_LayoutTopMargin, 0, this);

	// For the corner radius and the arrow width, the margin size are reasonable
	// defaults
	_cornerRadius=_margin;
	_arrowWidth=_margin;
}

NotificationWidget::~NotificationWidget ()
{
	// When setting a contents widget, its parent will be set to this, so it
	// will be deleted automatically.
}


// **************
// ** Contents **
// **************

/**
 * Uses the given widget as contents widget
 *
 * The contents widget will be reparented to this notification widget. The
 * notification widget takes ownership of the contents widget. The old contents
 * widget, if any, will be deleted.
 *
 * The bubble size will automatically be updated.
 *
 * You can retrieve the current contents widget by calling contents(). Do not
 * delete or reparent the contents manually. There is currently no way to get
 * back ownership of the contents widget. If you need this, add a takeContents()
 * method.
 */
void NotificationWidget::setContents (QWidget *contents)
{
	if (contents==_contents)
		return;

	if (_contents)
	{
		// Delete the old contents widget
		_contents->deleteLater ();
	}

	// Set the new contents widget
	_contents=contents;

	if (_contents)
	{
		// Reparent the new widget to this - it will be deleted in the
		// destructor. This also makes the contents widget invisible.
		_contents->setParent (this);
		_contents->show ();
	}

	invalidate ();
}

/**
 * A convenience method for setting a text instead of a contents widget
 *
 * This creates a new QLabel to hold the text. If a contents widget has been
 * set, it will be deleted. The QLabel can be accessed by calling contents(),
 * for example, to change its properties or to set an image.
 *
 * Note that you can use this method whenever the contents widget is a QLabel,
 * even if it has been set manually. This may be useful when using a QLabel
 * subclass.
 *
 * @see text
 */
void NotificationWidget::setText (const QString &text)
{
	// If the contents widget is not a label, make it one
	QLabel *label=dynamic_cast<QLabel *> (_contents);
	if (!label)
	{
		label=new QLabel (this);

		QPalette labelPalette=label->palette ();
		labelPalette.setColor (QPalette::WindowText, Qt::white);
		labelPalette.setColor (QPalette::Text, Qt::white);
		label->setPalette (labelPalette);

		setContents (label);
	}

	label->setText (text);
}

/**
 * If the contents widget is a QLabel, returns its text. This is most useful
 * with setText().
 *
 * @see setText
 */
QString NotificationWidget::text () const
{
	QLabel *label=dynamic_cast<QLabel *> (_contents);
	if (label)
		return label->text ();
	else
		return QString ();
}


// ****************
// ** Parameters **
// ****************

/**
 * Sets the width of the arrow base and updates the widget size, if necessary
 *
 * @see arrowWidth
 */
void NotificationWidget::setArrowWidth (int arrowWidth)
{
	_arrowWidth=arrowWidth;
	invalidate ();
}

/**
 * Sets the corner radius and updates the widget size, if necessary
 *
 * Note that this is only used for drawing the bubble, not for layout. You'll
 * generally want to set the margins to at least the same value for rectangular
 * contents widgets, or the contents may appear outside the bubble at the
 * corners.
 *
 * @see cornerRadius
 * @see setMargin
 */
void NotificationWidget::setCornerRadius (int cornerRadius)
{
	_cornerRadius=cornerRadius;
	invalidate ();
}

/**
 * Sets the margin and updates the widget size, if necessary
 *
 * The margin is the space between the contents widget's edges and the bubble's
 * edges. It is always identical on all sides.
 *
 * @see margin
 */
void NotificationWidget::setMargin (int margin)
{
	_margin=margin;
	invalidate ();
}


// **************
// ** Position **
// **************

/**
 * Determines a useful position for the bubble when the arrow tip is at the
 * specified position
 *
 * This is useful for determining the "ideal" position in order to lay out
 * notification widgets. If you just want to set the arrow position, you can use
 * moveTo(const QPoint &). The bubble position will be determined automatically.
 */
QPoint NotificationWidget::defaultBubblePosition (const QPoint &arrowTip) const
{
	// By default, the arrow tip is placed such that the arrow points straight
	// to the left from its default position right under the top-left corner,
	// and is twice as long as wide.
	int arrowX = -2 *_arrowWidth;
	int arrowY = _cornerRadius + _arrowWidth/2;

	QPoint relativeArrowPosition (arrowX, arrowY);
	return arrowTip - relativeArrowPosition;
}

/**
 * Moves (and potentially resizes) the widget such that the arrow tip and the
 * bubble (specifically, its top left corner) are at the specified positions
 *
 * The arguments are in parent coordinates.
 */
void NotificationWidget::moveTo (const QPoint &arrowTip, const QPoint &bubblePosition)
{
	_arrowTipFromBubblePosition=arrowTip-bubblePosition;
	invalidate ();

	// Move the widget. For calculating the position (in parent coordinates), we
	// can use either the arrow tip or the bubble position, the results should
	// be equal. We use the arrow tip position because it is more important.
	move (arrowTip - this->arrowTip ());
}

/**
 * Moves (and potentially resizes) the widget such that the arrow tip is at the
 * specified position
 *
 * The bubble position is chosen automatically, using the defaultBubblePosition
 * method.
 */
void NotificationWidget::moveTo (const QPoint &arrowTip)
{
	moveTo (arrowTip, defaultBubblePosition (arrowTip));
}


// ************
// ** Layout **
// ************

/**
 * Determines the recommended size for the bubble
 *
 * The recommended bubble size is large enough to hold the contents widget (at
 * its recommended size) with its margins on one hand, and the rounded corners
 * and the arrow at the other hand.
 */
QSize NotificationWidget::bubbleSizeHint () const
{
	// Determine the space we need for the contents (may be zero if we have no
	// contents).
	QSize contentsSizeHint;
	if (_contents)
		contentsSizeHint=_contents->sizeHint ();

	// Add the margin around the contents
	QSize bubbleSize=contentsSizeHint+QSize (2*_margin, 2*_margin);

	// Enlarge the bubble to the minimum bubble size, if necessary
	bubbleSize=max (bubbleSize, minimumBubbleSize ());

	return bubbleSize;
}

/**
 * Reimplemented from QWidget
 *
 * The recommended size of the widget is determined by the recommended size of
 * the bubble plus the space for the arrow.
 */
QSize NotificationWidget::sizeHint () const
{
	// Determine the bubble size
	QSize bubbleSize=bubbleSizeHint ();

	// Extend the bubble size to include the arrow tip
	int width =qMax (bubbleSize.width  (), right  ())+left ();
	int height=qMax (bubbleSize.height (), bottom ())+top  ();

	return QSize (width, height);
}

/**
 * Invalidates cached information about the geometry and posts a layout request
 * to this widget. Even when this method is called multiple times (before
 * returning to the event loop), the layout will only be performed once.
 *
 * The parent widget will also receive a layout request. Note that this may
 * cause the parent layout to be activated even though it does not manage the
 * notification widget at all. This is an implication of Qt layout management.
 */
void NotificationWidget::invalidate ()
{
	// Notify the parent that our size hint (may have) changed
	updateGeometry ();

	// Schedule an update of our own layout, potentially resizing the widget
	QApplication::postEvent (this, new QEvent (QEvent::LayoutRequest));
}

/**
 * Resizes the contents widget to its recommended size and sets it position
 *
 * This method also invalidates the path (since it has to be recalculated) and
 * causes the widget to be repainted.
 *
 * You don't usually have to call this method automatically; it will be called
 * whenever a layout request is received (see layoutRequestEvent).
 */
void NotificationWidget::doLayout ()
{
	if (_contents)
	{
		QSize contentsSize=_contents->sizeHint ();
		QPoint contentsPosition=centeredIn (contentsSize, bubbleGeometry ());

		_contents->setGeometry (QRect (contentsPosition, contentsSize));
	}

	// Clear the path
	_path_=QPainterPath ();

	// Note that even if the contents' size changed, the widget's size does not
	// necessarily change: if the arrow tip is to the bottom and/or right of the
	// bubble, the widget's size is defined by the arrow tip position. In this
	// case, we won't get a resize event, but we still have to make sure that
	// the widget is repainted.
	update ();
}


// **************
// ** Geometry **
// **************

/**
 * Returns the actual geometry of the bubble, in widget coordinates
 *
 * Note that the bubble position depends only on the shape parameters, not on
 * the contents.
 */
QRect NotificationWidget::bubbleGeometry () const
{
	return QRect (bubblePosition (), bubbleSizeHint ());
}

/**
 * Returns the actual geometry of the bubble, in parent coordinates
 *
 * @see bubbleGeometry
 */
QRect NotificationWidget::bubbleGeometryParent () const
{
	return bubbleGeometry ().translated (pos ());
}

/**
 * Returns the path for the bubble, including the arrow
 *
 * This is used for both painting and collision check (see mousePressEvent).
 *
 * The path is cached. As long as it is not invalidated, calling this method
 * several times is very fast.
 */
QPainterPath NotificationWidget::path ()
{
	if (_path_.isEmpty ())
	{
		QPoint arrowTip=this->arrowTip ();

		QRect bubble=bubbleGeometry ();

		// Calculate the corner rectangles
		// We use QRectF here in order to be able to use the bottom/right methods,
		// which are not usable for QRect.
		QRectF northWest=northWestCorner (bubble, 2*_cornerRadius);
		QRectF northEast=northEastCorner (bubble, 2*_cornerRadius);
		QRectF southWest=southWestCorner (bubble, 2*_cornerRadius);
		QRectF southEast=southEastCorner (bubble, 2*_cornerRadius);

		// Calculate the arrow base coordinates
		// This tries to place the arrow base on the same height as the tip to make
		// it as straight as possible, if that is allowed by the bubble dimensions.
		// If not, it will be right below the top left corner or right above the
		// bottom left corner of the bubble.
		int arrowCenter=qBound (
			bubble.top ()                 +_cornerRadius+_arrowWidth/2,
			arrowTip.y (),
			bubble.top ()+bubble.height ()-_cornerRadius-_arrowWidth/2);

		QPoint arrowTop    (bubble.left (), arrowCenter-_arrowWidth/2);
		QPoint arrowBottom (bubble.left (), arrowCenter+_arrowWidth/2);


		// Clear the path
		_path_=QPainterPath ();

		// Draw the path counter-clockwise, starting with the bottom left corner arc
		// and ending after the top left corner arc.
		_path_.moveTo (southWest.topLeft     ().toPoint ()); _path_.arcTo (southWest, 180, 90);
		_path_.lineTo (southEast.bottomLeft  ().toPoint ()); _path_.arcTo (southEast, 270, 90);
		_path_.lineTo (northEast.bottomRight ().toPoint ()); _path_.arcTo (northEast,   0, 90);
		_path_.lineTo (northWest.topRight    ().toPoint ()); _path_.arcTo (northWest,  90, 90);
		//
		// Draw the arrow
		_path_.lineTo (arrowTop);
		_path_.lineTo (arrowTip);
		_path_.lineTo (arrowBottom);
		//
		// Close the path
		_path_.closeSubpath ();
	}

	return _path_;
}


// ************
// ** Events **
// ************

/**
 * Reimplemented from QObject
 *
 * This method only dispatches the layout request event to the
 * layoutRequestEvent method.
 */
bool NotificationWidget::event (QEvent *e)
{
	if (e->type ()==QEvent::LayoutRequest)
	{
		layoutRequestEvent ();
		return true;
	}

	return QWidget::event (e);
}

/**
 * The a close event is received, the closed() signal is emitted.
 */
void NotificationWidget::closeEvent (QCloseEvent  *e)
{
	(void)e;
	emit closed ();
}

/**
 * When a layout request event is received, updates the own position and lays
 * out the contents
 *
 * This can be caused by a parameter change (e. g. the margins) or by a geometry
 * change of the contents.
 */
void NotificationWidget::layoutRequestEvent ()
{
	// Make sure that the widget is at the correct size to hold the contents.
	// This is perforemd here rather than in doLayout so we can call doLayout
	// from resizeEvent without the risk of causing recursion.
	setFixedSize (sizeHint ());
	doLayout ();
}

/**
 * When the mouse is clicked inside the bubble (including the arrow), the
 * clicked() event is emitted. Note that the clicked event may be intercepted by
 * the contents widget.
 */
void NotificationWidget::mousePressEvent (QMouseEvent *e)
{
	// If the event position is outside the bubble, let the parent widget
	// receive the event.
	if (!path ().contains (e->pos ()))
	{
		e->ignore ();
		return;
	}

	// React to the event
	emit clicked ();
}

/**
 * Repaints the widget
 */
void NotificationWidget::paintEvent (QPaintEvent *e)
{
    (void)e;

	QWidget::paintEvent (e);

	QPainter painter (this);
	painter.setRenderHint (QPainter::Antialiasing);

	// Well, this is easy: we just paint the bubble path
	painter.setPen (Qt::NoPen);
	painter.setBrush (bubbleColor);
	painter.drawPath (path ());
}
