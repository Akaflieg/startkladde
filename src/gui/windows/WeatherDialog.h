#ifndef WEATHERDIALOG_H_
#define WEATHERDIALOG_H_

#include <QDialog>

class QResizeEvent;
class QHBoxLayout;

class WeatherPlugin;
class WeatherWidget;

class WeatherDialog: public QDialog
{
	public:
		WeatherDialog (WeatherPlugin *plugin, QWidget *parent=NULL);
		~WeatherDialog ();

		void restartPlugin ();

	protected:
		virtual void resizeEvent (QResizeEvent *);

	private:
		WeatherWidget *ww;
		WeatherPlugin *plugin;
		QHBoxLayout *weatherLayout;

};

#endif

