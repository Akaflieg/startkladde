#include "PlotWidget.h"

#include <cmath>

#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QSettings>
#include <QWheelEvent>

// Notes:
//   * Whenever the plot coordinate system changes, call invalidateView.
//   * For accessing the transforms, use w_T_p()/p_T_w(). The transforms will be
//     updated automatically, if necessary

PlotWidget::PlotWidget (QWidget *parent): QFrame (parent),
	_transforms (*this)
{
	// Default orientation: plot coordinate system origin at widget center,
	// smaller side 1, x axis right
	_center_p=QPointF (0, 0);
	_diameter_p=1;
	_orientation=Angle::zero ();

	// None of the mouse actions are active
	_mouseScrollActive=false;
	_mouseZoomActive=false;
	_mouseRotationActive=false;

	// The zoom factors: the magnitude of action to zoom in by a factor of two
	// Mouse wheel down (back) means zooming out. This is the convention that
	// many other applications, including Firefox and Gimp, use.
	_mouseZoomDoubleDistance_w=50;
	_mouseWheelZoomDoubleAngle=Angle::fromDegrees (120);
	_keyboardZoomDoubleCount=8;

	// The rotation factors: the magnitude of action to rotate for a full
	// revolution
	_mouseRotationRevolutionDistance_w=-360; // 1° per pixel
	_keyboardRotationRevolutionCount=36; // 10° per keypress
	_mouseWheelRotationRevolutionAngle=Angle::fromDegrees (540); // 10° rotate per "standard" wheel increment (15°)

	// For the mouse actions, the initial mouse positions are irrelevant as they
	// are set when the actions starts and ignored as long as the action is not
	// active, and so are the original values of the respective properties (they
	// are still initialized for good coding practice).
	_mouseZoomStartPosition_w=QPoint (0, 0);
	_mouseRotationStartPosition_w=QPoint (0, 0);
	//
	_mouseScrollPosition_p=QPointF (0, 0);
	_mouseZoomOriginalDiameter_p=1;
	_mouseRotationOriginalOrientation=Angle::zero ();
}

PlotWidget::~PlotWidget ()
{
}


// ***********
// ** State **
// ***********

void PlotWidget::saveState (QSettings &settings)
{
	settings.setValue ("centerX"    , _center_p.x ());
	settings.setValue ("centerY"    , _center_p.y ());
	settings.setValue ("diameter"   , _diameter_p);
	settings.setValue ("orientation", _orientation.toDegrees ());
}

void PlotWidget::loadState (QSettings &settings)
{
	if (settings.contains ("centerX") && settings.contains ("centerY"))
	{
		double centerX=settings.value ("centerX").toDouble ();
		double centerY=settings.value ("centerY").toDouble ();
		setCenter_p (QPointF (centerX, centerY));
	}
	if (settings.contains ("diameter"))
	{
		setDiameter_p (settings.value ("diameter").toDouble ());
	}
	if (settings.contains ("orientation"))
	{
		setOrientation (Angle::fromDegrees (settings.value ("orientation").toDouble ()));
	}

	_transforms.invalidate ();
}


// **************
// ** Position **
// **************

QPointF PlotWidget::center_p () const
{
	return _center_p;
}

void PlotWidget::setCenter_p (const QPointF &center_p)
{
	_center_p=center_p;
	_transforms.invalidate ();
}

void PlotWidget::scrollToCenter (const QPointF &position_p)
{
	setCenter_p (position_p);
}

void PlotWidget::scrollTo (const QPointF &position_p, const QPointF &position_w)
{
	// We want the plot position position_p to be at the target position
	// position_w. We therefore calculate the location that is currently at this
	// position and correct the center location by this difference. We do  this
	// in plot coordinates because this is the coordinate system the center
	// location is stored in.
	QPointF currentPosition_p=toPlot (position_w);
	setCenter_p (center_p ()+position_p-currentPosition_p);
}

void PlotWidget::scrollBy (double dx_w, double dy_w)
{
	scrollTo (toPlot (QPointF (dx_w, dy_w)), QPointF (0, 0));
}


// *****************
// ** Orientation **
// *****************

void PlotWidget::setOrientation (const Angle &orientation)
{
	// Normalize the new orientation
	Angle normalizedOrientation=orientation.normalized ();

	// If the value is already current, stop
	if (_orientation==normalizedOrientation)
		return;

	// Assign the (normalized) new value
	_orientation=normalizedOrientation;

	// Invalidate cached data
	_transforms.invalidate ();

	// Emit the orientationChanged signal, *after* setting the new value and
	// invalidating cached data
	emit orientationChanged ();
}

Angle PlotWidget::orientation () const
{
	return _orientation;
}

void PlotWidget::rotateBy (const Angle &rotation)
{
	setOrientation (_orientation+rotation);
}


// ***********
// ** Scale **
// ***********

double PlotWidget::diameter_p () const
{
	return _diameter_p;
}

void PlotWidget::setDiameter_p (double diameter_p)
{
	_diameter_p=diameter_p;
	_transforms.invalidate ();
}

QSizeF PlotWidget::size_p () const
{
	double widgetAspectRatio = width () / (double)height ();

	if (widgetAspectRatio>=1)
		// The widget is wider than high
		return QSizeF (_diameter_p*widgetAspectRatio, _diameter_p);
	else
		// The widget is higher than wide
		return QSizeF (_diameter_p, _diameter_p/widgetAspectRatio);
}

void PlotWidget::zoomInBy (double factor)
{
	setDiameter_p (_diameter_p/factor);
}

/**
 * Calculates the length, in plot coordinates, of one widget coordinate length
 * unit (one pixel)
 */
double PlotWidget::widgetScale_p () const
{
	return _diameter_p/qMin (width (), height ());
}

/**
 * Calculates the length, in widget coordinates (pixels), of one plot coordinate
 * length unit
 */
double PlotWidget::plotScale_w () const
{
	return 1/widgetScale_p ();
}


// ***********************
// ** Coordinate system **
// ***********************

PlotWidget::Transforms::Transforms (PlotWidget &widget):
	_widget (widget), _valid (false)
{
}

void PlotWidget::Transforms::invalidate ()
{
	// Mark the transforms as invalid. They will be recalculated the next time
	// they are accessed. The widget's viewChanged signal will also be emitted
	// at that point.
	_valid=false;

	// Schedule a repaint of the widget
	_widget.update ();
}

void PlotWidget::Transforms::update ()
{
	if (_valid)
		return;

	//qDebug () << "Recalculating transforms with " << _center_p << _orientation << _diameter_p;

	// _w_T_p is supposed to describe the widget coordinate system in plot
	// coordinates. This is achieved by transforming in several steps. The
	// transforms always apply to the current coordinate system.
	//
	// Start at the plot coordinate system itself
	_w_T_p=QTransform ();
	// Translate it so that its origin is at the center of the widget
	_w_T_p.translate (_widget._center_p.x (), _widget._center_p.y ());
	// Rotate it so that the x axis is parallel to the widget's x axis; this
	// will point the y axis opposite to the widget's y axis.
	_w_T_p.rotateRadians (-_widget._orientation.toRadians ());
	// Flip it upside down so both axes are parallel to the widget's axes
	_w_T_p.scale (1, -1);
	// Scale it so that the diameter fits
	double scale=_widget._diameter_p/qMin (_widget.width (), _widget.height ());
	_w_T_p.scale (scale, scale);
	// Translate it to the origin of the widget coordinate system
	_w_T_p.translate (-_widget.width ()/2.0, -_widget.height ()/2.0);

	// Calculate the inverse transform
	_p_T_w=_w_T_p.inverted ();

	_valid=true;
	_widget.emitViewChanged ();
}

QTransform &PlotWidget::Transforms::w_T_p ()
{
	update ();
	return _w_T_p;
}

QTransform &PlotWidget::Transforms::p_T_w ()
{
	update ();
	return _p_T_w;
}

void PlotWidget::emitViewChanged () const
{
	emit viewChanged ();
}




// *********************
// ** Qt mouse events **
// *********************

void PlotWidget::mousePressEvent (QMouseEvent *event)
{
	if (event->button ()==Qt::LeftButton)
	{
        _mouseScrollPosition_p=toPlot (event->localPos ());
		_mouseScrollActive=true;
		event->accept ();
	}
	else if (event->button ()==Qt::MiddleButton)
	{
        _mouseZoomStartPosition_w=event->localPos ();
		_mouseZoomStartPosition_p=toPlot (_mouseZoomStartPosition_w);
		_mouseZoomOriginalDiameter_p=diameter_p ();
		_mouseZoomActive=true;

		_mouseRotationStartPosition_w=event->pos ();
		_mouseRotationOriginalOrientation=orientation ();
		_mouseRotationActive=true;
		event->accept ();

		emit showMessage (tr ("Ctrl: zoom only; Shift: rotate only"));
	}
	else
	{
		QFrame::mousePressEvent (event);
	}
}

void PlotWidget::mouseReleaseEvent (QMouseEvent *event)
{
	if (event->button ()==Qt::LeftButton)
	{
		_mouseScrollActive=false;
		event->accept ();
	}
	else if (event->button ()==Qt::MiddleButton)
	{
		_mouseZoomActive=false;
		_mouseRotationActive=false;
		event->accept ();
	}

	emit clearMessage ();
}

void PlotWidget::mouseMoveEvent (QMouseEvent *event)
{
	// Shift: move only
	// Control: zoom only

	bool shiftPressed  =((event->modifiers () & Qt::ShiftModifier  )!=0);
	bool controlPressed=((event->modifiers () & Qt::ControlModifier)!=0);

    emit mouseMoved_p (toPlot (event->localPos ()));

	if (_mouseScrollActive)
	{
        scrollTo (_mouseScrollPosition_p, event->localPos ());
	}

	if (_mouseZoomActive && !shiftPressed)
	{
		int deltaY=event->pos ().y () - _mouseZoomStartPosition_w.y ();
		setDiameter_p (_mouseZoomOriginalDiameter_p*pow (2, deltaY/_mouseZoomDoubleDistance_w));

		// Zoom around the initial mouse position
		scrollTo (_mouseZoomStartPosition_p, _mouseZoomStartPosition_w);
	}

	if (_mouseRotationActive && !controlPressed)
	{
		int deltaX=event->pos ().x () - _mouseRotationStartPosition_w.x ();
		Angle deltaAngle=Angle::fullCircle ()*deltaX/_mouseRotationRevolutionDistance_w;
		setOrientation (_mouseRotationOriginalOrientation+deltaAngle);
	}
}

void PlotWidget::wheelEvent (QWheelEvent *event)
{
    Angle angle=Angle::fromDegrees (event->angleDelta().y()/(double)8);

	if (event->modifiers ()==Qt::AltModifier)
	{
		double angleFraction=angle/_mouseWheelRotationRevolutionAngle;
		rotateBy (Angle::fullCircle () * angleFraction);
	}
	else
	{
		// Store the previous position so we can zoom around the mouse position
#ifdef QT_COMPAT
	QPointF position_w=QPointF (event->pos ());
#else
        QPointF position_w=QPointF (event->position ());
#endif
		QPointF position_p=toPlot (position_w);

		zoomInBy (pow (2, angle/_mouseWheelZoomDoubleAngle));

		// Zoom around the mouse position
		scrollTo (position_p, position_w);
	}
}


void PlotWidget::leaveEvent (QEvent *event)
{
	(void)event;
	emit mouseLeft ();
}


// ************************
// ** Qt keyboard events **
// ************************

void PlotWidget::keyPressEvent (QKeyEvent *event)
{
	// Note that we don't call event->accept (). The documentation for QWidget::
	// keyPressEvent recommends that implementations do not call the superclass
	// method if they act upon the key.
	const double keyboardZoomFactor=pow (2, 1/(double)_keyboardZoomDoubleCount);
	const Angle keyboardRotation=Angle::fullCircle ()/_keyboardRotationRevolutionCount;
	const int keyboardScrollDistance=0.1*qMin (width (), height ());

	switch (event->key ())
	{
		// Zoom in. The Key_Equal is a common hack for english keyboard layouts
		// where Key_Plus is on the same key, but shifted.
		case Qt::Key_Plus:         zoomInBy (keyboardZoomFactor); break;
		case Qt::Key_Equal:        zoomInBy (keyboardZoomFactor); break;

		// Zoom out
		case Qt::Key_Minus:        zoomInBy (1/keyboardZoomFactor); break;

		// Rotate
		case Qt::Key_BracketLeft:  rotateBy ( keyboardRotation); break;
		case Qt::Key_BracketRight: rotateBy (-keyboardRotation); break;

		// Scroll
		case Qt::Key_Right: case Qt::Key_L: scrollBy ( keyboardScrollDistance, 0); break;
		case Qt::Key_Left : case Qt::Key_H: scrollBy (-keyboardScrollDistance, 0); break;
		case Qt::Key_Up   : case Qt::Key_K: scrollBy (0, -keyboardScrollDistance); break;
		case Qt::Key_Down : case Qt::Key_J: scrollBy (0,  keyboardScrollDistance); break;

		// Other
		default: QFrame::keyPressEvent (event); break;
	}
}


// *********************
// ** Other Qt events **
// *********************

void PlotWidget::resizeEvent (QResizeEvent *event)
{
	(void)event;
	_transforms.invalidate ();
}


// ****************
// ** Transforms **
// ****************

QPointF PlotWidget::toWidget (const QPointF &point_p) const
{
	return point_p * p_T_w ();
}

QPointF PlotWidget::toPlot (const QPointF &point_w) const
{
	return point_w * w_T_p ();
}

QPointF PlotWidget::toWidget (double x_p, double y_p) const
{
	return toWidget (QPointF (x_p, y_p));
}

QPointF PlotWidget::toPlot (double x_w, double y_w) const
{
	return toPlot (QPointF (x_w, y_w));
}

QPolygonF PlotWidget::toWidget (const QPolygonF &Polygon_p) const
{
	return Polygon_p * p_T_w ();
}

QPolygonF PlotWidget::toPlot (const QPolygonF &Polygon_w) const
{
	return Polygon_w * w_T_p ();
}

double PlotWidget::toWidget (double length_p) const
{
	return length_p*plotScale_w ();
}

double PlotWidget::toPlot (double length_w) const
{
	return length_w*widgetScale_p ();
}

void PlotWidget::transformToPlot (QPainter &painter) const
{
	painter.setTransform (p_T_w (), false);
}

QRectF PlotWidget::rect_w () const
{
	return QRectF (rect ());
}

/**
 * Returns the bounding rectangle for the visible rectangle, in plot coordinates
 *
 * Note that the y axis of QRectF points up and the y axis of the plot
 * coordinate system points down, so the top of the returned rectangle will
 * actually be the south of the bounding rectangle.
 */
QRectF PlotWidget::boundingRect_p () const
{
	return w_T_p ().mapRect (rect_w ());
}
