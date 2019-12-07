#ifndef WEATHERWIDGET_H_
#define WEATHERWIDGET_H_

#include <QPixmap>
#include <QImageReader>

#include "src/graphics/SkMovie.h"
#include "src/gui/widgets/SkLabel.h"

class QImage;
class QMovie;
class QTemporaryFile;

class WeatherWidget:public SkLabel
{
	Q_OBJECT

	public:
		WeatherWidget (QWidget *parent=NULL);
		virtual ~WeatherWidget ();
		virtual void setText (const QString&);

	public slots:
		virtual void setImage (const QImage &image);
		virtual void setMovie (SkMovie &movie);

	signals:
		void doubleClicked ();
		void sizeChanged (const QSize&);

	protected:
		virtual void mouseDoubleClickEvent (QMouseEvent *e);
        virtual void resizeEvent (QResizeEvent *);
        int heightForWidth(int width) const;
        QSize sizeHint();

	private:
		SkMovie newMovie;
};

#endif

