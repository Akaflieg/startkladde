#include "WeatherDialog.h"

#include <QLayout>

#include "src/gui/widgets/WeatherWidget.h"
#include "src/plugin/weather/WeatherPlugin.h"

class SkMovie;

WeatherDialog::WeatherDialog (WeatherPlugin *plugin, QWidget *parent):
	QDialog (parent),
	plugin (plugin)
{
	ww=new WeatherWidget (this);
	ww->setWordWrap (false);

	QObject::connect (ww, SIGNAL (doubleClicked ()), plugin, SLOT (restart ()));
	connect (plugin, SIGNAL (textOutput (const QString &, Qt::TextFormat)), ww, SLOT (setText (const QString &, Qt::TextFormat)));
	connect (plugin, SIGNAL (imageOutput (const QImage &)), ww, SLOT (setImage (const QImage &)));
	connect (plugin, SIGNAL (movieOutput (SkMovie &)), ww, SLOT (setMovie (SkMovie &)));
	plugin->start ();

	weatherLayout=new QHBoxLayout ();
	weatherLayout->addWidget (ww);
	setLayout (weatherLayout);
}

WeatherDialog::~WeatherDialog ()
{
	delete plugin;
}

void WeatherDialog::resizeEvent (QResizeEvent *e)
{
	(void)e;
//	// Bei Größenänderung: so verschieben, dass das Zentrum des Dialogs an der
//	// gleichen Stelle bleibt.
//
//	// So oder so?
//	//int oldCenterX=geometry ().x ()+e->oldSize ().width ()/2;
//	//int oldCenterY=geometry ().y ()+e->oldSize ().height ()/2;
//	int oldCenterX=x ()+e->oldSize ().width ()/2;
//	int oldCenterY=y ()+e->oldSize ().height ()/2;
//
//	int newX=oldCenterX-frameGeometry ().width ()/2;
//	int newY=oldCenterY-frameGeometry ().height ()/2;
//
//	move (newX, newY);
}

void WeatherDialog::restartPlugin ()
{
	 if (plugin)
		 plugin->restart ();
}

