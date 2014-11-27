#ifndef PLOTWIDGET_H_
#define PLOTWIDGET_H_

#include <QFrame>
#include <QPointF>
#include <QTransform>

#include "src/numeric/Angle.h"

class QEvent;
class QKeyEvent;
class QMouseEvent;
class QResizeEvent;
class QSettings;
class QWheelEvent;

/**
 * A widget for plotting, with a plot coordinate system that can be moved,
 * scaled and rotated interactively and programmatically
 *
 * By default, the plot coordinate system is centered in the widget.
 *
 * The plot coordinate system can be rotated by setting the orientation. For an
 * orientation of 0°, the x axis of the plot coordinate system points right (+x
 * in widget coordinates) and the y axis of the plot coordinate system points up
 * (-y in widget coordinates). A positive orientation rotates the plot
 * coordinate system counter-clockwise, i. e. for an orientation of +90°, the x
 * axis points up and the y axis points left. The default orientation is 0°.
 *
 * Apart for the mirrored y axis, the plot coordinate system is always scaled
 * uniformly with respect to the widget coordinate system. The scale factor is
 * determined by the widget size and the diameter" (specified in plot
 * coordinates). The diameter specifies the diameter of a circle inscribed into
 * the widget. Zooming changes the diameter and subsequently the scale factor.
 * When the widget size changes, the zoom factor is updated to honor the
 * diameter. By default, the diameter is set to 1.
 *
 * All position, rotation and distance identifier names indicate the coordinate
 * system they refer to by a suffix of _w (for the widget coordinate system) or
 * _p (for the plot coordinate system).
 *
 * Note that the recommended convention for coordinate system transforms is
 * a_T_b for the transform that describes the a system with respect to the b
 * system. That way, points can be transformed like so:
 *   point_c = point_a * a_T_b * b_T_c;
 * Note that Qt interprets points as row vectors and stores the unit vectors in
 * row vectors in transformation matrices, giving a multiplication order of
 * point*matrix.
 */
class PlotWidget: public QFrame
{
		Q_OBJECT

	public:
		PlotWidget (QWidget *parent);
		virtual ~PlotWidget ();

		// State
		void loadState (QSettings &settings);
		void saveState (QSettings &settings);

		// Position
		QPointF center_p () const;
		void setCenter_p (const QPointF &center_p);
		void scrollToCenter (const QPointF &position_p);
		void scrollTo (const QPointF &position_p, const QPointF &position_w);
		void scrollBy (double dx_w, double dy_w);

		// Orientation
		virtual Angle orientation () const;
		virtual void setOrientation (const Angle &orientation);
		void rotateBy (const Angle &rotation);

		// Scale
		double diameter_p () const;
		void setDiameter_p (double diameter_p);
		QSizeF size_p () const;
		void zoomInBy (double factor);
		double widgetScale_p () const;
		double plotScale_w () const;

	signals:
		void viewChanged () const;
		void mouseMoved_p (QPointF position_p) const;
		void mouseLeft () const;
		void orientationChanged () const;
		void showMessage (QString message) const;
		void clearMessage () const;

	protected:
		// Qt mouse events
		virtual void mousePressEvent   (QMouseEvent  *event);
		virtual void mouseReleaseEvent (QMouseEvent  *event);
		virtual void mouseMoveEvent    (QMouseEvent  *event);
		virtual void leaveEvent        (QEvent       *event);
		virtual void wheelEvent        (QWheelEvent  *event);
		// Qt keyboard events
		virtual void keyPressEvent     (QKeyEvent    *event);
		// Other Qt events
		virtual void resizeEvent       (QResizeEvent *event);

		// Transforms
		QPointF toWidget (const QPointF &point_p) const;
		QPointF toPlot   (const QPointF &point_w) const;
		QPointF toWidget (double x_p, double y_p) const;
		QPointF toPlot   (double x_w, double y_w) const;
		QPolygonF toWidget (const QPolygonF &polygon_p) const;
		QPolygonF toPlot   (const QPolygonF &polygon_w) const;
		double toWidget (double length_p) const;
		double toPlot (double length_w) const;
		void transformToPlot (QPainter &painter) const;
		QRectF rect_w () const;
		QRectF boundingRect_p () const;


	private:
		class Transforms
		{
			public:
				Transforms (PlotWidget &widget);
				void invalidate ();
				void update ();
				QTransform &w_T_p ();
				QTransform &p_T_w ();

			private:
				PlotWidget &_widget;
				QTransform _w_T_p, _p_T_w;
				bool _valid;
		};

		// The values defining the plot coordinate system
		QPointF _center_p;
		double _diameter_p;
		Angle _orientation;

		// Coordinate system
		mutable Transforms _transforms;
		void emitViewChanged () const;
		QTransform &w_T_p () const { return _transforms.w_T_p (); }
		QTransform &p_T_w () const { return _transforms.p_T_w (); }

		// Mouse scrolling
		bool    _mouseScrollActive;
		QPointF _mouseScrollPosition_p;

		// Mouse zooming
		double _mouseZoomDoubleDistance_w;
		bool   _mouseZoomActive;
		QPointF _mouseZoomStartPosition_w;
		QPointF _mouseZoomStartPosition_p;
		double _mouseZoomOriginalDiameter_p;

		// Mouse rotation
		double _mouseRotationRevolutionDistance_w;
		bool   _mouseRotationActive;
		QPoint _mouseRotationStartPosition_w;
		Angle  _mouseRotationOriginalOrientation;

		// Mouse wheel zooming
		Angle _mouseWheelZoomDoubleAngle;

		// Mouse wheel rotation
		Angle _mouseWheelRotationRevolutionAngle;

		// Keyboard zooming
		int _keyboardZoomDoubleCount;

		// Keyboard rotation
		int _keyboardRotationRevolutionCount;
};

#endif
