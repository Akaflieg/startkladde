#ifndef NOTIFICATIONWIDGET_H
#define NOTIFICATIONWIDGET_H

#include <QWidget>
#include <QPainterPath>

#include "src/util/math.h"

class QEvent;
class QCloseEvent;
class QMouseEvent;
class QPaintEvent;

/**
 * A semi-transparent, shaped speech-bubble-like widget that can contain any
 * contents widget. It is intended as an unobstrusive means to show a message
 * to the user without requiring action or disturbing input focus.
 *
 * The bubble consists of a rounded rectangle and an arrow. The width of the
 * arrow at its base, the radius of the rounded corners and the margin of the
 * contents from the bubble edges can be configured. The bubble (and the whole
 * widget) is sized automatically to fit the contents.
 *
 * To use this class, create an instance with the parent widget as for any other
 * widget. Set a contents widget with setContents() (you can also use setText()
 * to set a text. In this case, a contents widget will be created
 * automatically). Use moveTo() to move the arrow tip (and potentially the
 * bubble) to a specific location. The widget will always be sized automatically
 * in a way such that the bubble size is the recommended size of the contents
 * widget (as determined by its size hint) plus the margins. When the contents's
 * recommended size changes, the bubble (and, if necessary, the whole widget)
 * will automatically be resized while keeping the positions of the bubble and
 * the arrow tip.
 *
 * The NotificationWidget emits a signal when it is clicked and another signal
 * when it is closed. These signals may be useful to manage the visibility and
 * layout of notification widgets.
 *
 * This is just the plain widget. You may find the NotificationsLayout and/or
 * the WidgetFader classes useful in conjunction with this class.
 *
 * You should not use the QWidget::resize method on a bubble widget. The size is
 * calculated automatically to accommodate the contents, the margin and the
 * arrow. You can use QWidget::move, although the moveTo method will probably be
 * more useful.
 *
 * Note that when the user clicks at a point that is inside the widget but
 * outside the bubble, the event will be received by the parent widget of the
 * notification widget, not by any sibling widget that may be located at this
 * point. This could potentially be solved by using a widget mask for the
 * notification widget.
 */
class NotificationWidget: public QWidget
{
		Q_OBJECT

	public:
		// Construction
		explicit NotificationWidget (QWidget *parent, Qt::WindowFlags f=0);
		~NotificationWidget ();

		// Contents
		void setContents (QWidget *contents);
		QWidget *contents () const  { return _contents; }
		void setText (const QString &text);
		QString text () const;

		// Parameters
		void setArrowWidth   (int arrowWidth  );
		void setCornerRadius (int cornerRadius);
		void setMargin       (int margin      );

		int arrowWidth   () const { return _arrowWidth;   }
		int cornerRadius () const { return _cornerRadius; }
		int margin       () const { return _margin;       }

		// Position
		QPoint defaultBubblePosition (const QPoint &arrowTip) const;
		void moveTo (const QPoint &arrowTip, const QPoint &bubblePosition);
		void moveTo (const QPoint &arrowTip);

		// Layout
		virtual QSize sizeHint () const;

		// Geometry
		QRect bubbleGeometry () const;
		QRect bubbleGeometryParent () const;

	signals:
		void closed ();
		void clicked ();

	protected:
		// Layout
		virtual void invalidate ();
		virtual void doLayout ();

		// Depends only on the parameters - in widget coordinates
		int top    () const { return ifPositive (-_arrowTipFromBubblePosition.y ()); }
		int left   () const { return ifPositive (-_arrowTipFromBubblePosition.x ()); }
		int bottom () const { return ifPositive ( _arrowTipFromBubblePosition.y ()); }
		int right  () const { return ifPositive ( _arrowTipFromBubblePosition.x ()); }
		QPoint arrowTip       () const { return QPoint (right (), bottom ()); }
		QPoint bubblePosition () const { return QPoint (left  (), top    ()); }
		QSize minimumBubbleSize () const { return QSize (2*_cornerRadius, 2*_cornerRadius+_arrowWidth); }

		// Depends only on the parameters and the contents
		QSize bubbleSizeHint () const;

		// Depends on the actual layout
		QPainterPath path ();

		// Qt events
		virtual bool event              (QEvent       *e);
		virtual void closeEvent         (QCloseEvent  *e);
		virtual void layoutRequestEvent ();
		virtual void mousePressEvent    (QMouseEvent  *e);
		virtual void paintEvent         (QPaintEvent  *e);


	private:
		// Contents
		QWidget *_contents;

		// Colors
		QColor bubbleColor;

		// Geometry parameters - set by the user
		int _arrowWidth;
		int _cornerRadius;
		int _margin;
		QPoint _arrowTipFromBubblePosition; // NB!

		// The actual geometry - known after doing the layout
		QPainterPath _path_;

};

#endif
