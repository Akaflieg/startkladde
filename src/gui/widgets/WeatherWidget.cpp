#include "WeatherWidget.h"

#include <QEvent>
#include <QImage>
#include <QMovie>
#include <QPainter>
#include <QRegExp>
#include <QResizeEvent>

#include "src/config/Settings.h" // TODO remove dependency, set from MainWindow
#include "src/i18n/notr.h"


WeatherWidget::WeatherWidget (QWidget *parent):
	SkLabel (parent)
{
	if (Settings::instance ().coloredLabels)
	{
		setAutoFillBackground (true);
		setPaletteBackgroundColor (QColor (127, 127, 127));
	}

	setAlignment (Qt::AlignHCenter | Qt::AlignVCenter);
	setWordWrap (true);
	setScaledContents (true);
	setTextFormat (Qt::RichText);
}

WeatherWidget::~WeatherWidget ()
{
}

void WeatherWidget::setImage (const QImage &image)
{
	setWordWrap (false);
	QPixmap pixmap=QPixmap::fromImage (image);
	setPixmap (pixmap);

//	int height=Settings::instance ().weatherPluginHeight;
//	setFixedSize (height*pixmap.width()/pixmap.height(), height);
}

void WeatherWidget::setText (const QString &text)
{
	setWordWrap (true);
	QLabel::setText (text);
	adjustSize ();
}

void WeatherWidget::setMovie (SkMovie &movie)
{
	// NB: wenn hier WordWrap=true ist (zum Beispiel, weil vorher ein Text
	// gesetzt war), dann funktioniert die Movie-Darstellung nicht mehr, wenn
	// einmal ein Text gesetzt war: das Fenster, das nur ein Layout mit diesem
	// Widget enthält, wird auf Höhe 0 gesetzt.
	// Nach diesem Fehler kann man bequem 2 Stunden lang suchen.
	setWordWrap (false);

	// Clear the contents so we can delete the movie
	setText ("");

	SkLabel::setMovie (movie.getMovie ());
	movie.getMovie ( )->start ();

	// Store the movie. The temporary file will be deleted when all copies of
	// this SkMovie have been deleted. If there is an old file, this will
	// probably happen right now.
	newMovie=movie;
}

void WeatherWidget::mouseDoubleClickEvent (QMouseEvent *e)
{
	(void)e;
	emit (doubleClicked ());
}

void WeatherWidget::resizeEvent (QResizeEvent *e)
{
	QLabel::resizeEvent (e);
	emit sizeChanged (e->size ());
}
