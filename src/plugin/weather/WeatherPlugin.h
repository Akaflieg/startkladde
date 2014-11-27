#ifndef WEATHERPLUGIN_H_
#define WEATHERPLUGIN_H_

#include <QUuid>

#include "src/plugin/Plugin.h"
#include "src/accessor.h"

class QImage;
class QTimer;
class QTemporaryFile;

class SkMovie;

/**
 * A plugin which displays a weather image (or an error message text)
 */
class WeatherPlugin: public Plugin
{
	Q_OBJECT

	public:
		// ***********
		// ** Types **
		// ***********

		class Descriptor
		{
			public:
				virtual ~Descriptor () {}

				virtual WeatherPlugin *create () const=0;
				virtual QUuid   getId          () const=0;
				virtual QString getName        () const=0;
				virtual QString getDescription () const=0;

				static bool nameLessThan (const WeatherPlugin::Descriptor &d1, const WeatherPlugin::Descriptor &d2);
				static bool nameLessThanP (const WeatherPlugin::Descriptor *d1, const WeatherPlugin::Descriptor *d2);

			private:
				QString id, name, description;
		};

		template<class T> class DefaultDescriptor: public Descriptor
		{
			public:
				virtual WeatherPlugin *create () const { return new T (); }
				virtual QUuid   getId          () const { return T::_getId          (); }
				virtual QString getName        () const { return T::_getName        (); }
				virtual QString getDescription () const { return T::_getDescription (); }
		};


		// ******************
		// ** Construction **
		// ******************

		WeatherPlugin ();
		virtual ~WeatherPlugin ();


	public:
		virtual void start ();
		virtual void terminate ();

		virtual void readSettings (const QSettings &settings) { (void)settings; }
		virtual void writeSettings (QSettings &settings) { (void)settings; }
		virtual PluginSettingsPane *createSettingsPane (QWidget *parent=NULL) { (void)parent; return NULL; }
		virtual QString configText () const { return QString (); }

		virtual void enableRefresh (unsigned int seconds);
		virtual void disableRefresh ();

	public slots:
		virtual void refresh ()=0;
		virtual void abort ()=0;

	signals:
		void textOutput (const QString &text, Qt::TextFormat format);
		void imageOutput (const QImage &image);
		void movieOutput (SkMovie &movie);

	protected:
		void outputText (const QString &text, Qt::TextFormat format=Qt::PlainText);
		void outputImage (const QImage &image);
		void outputMovie (SkMovie &movie);

	private:
		QTimer *refreshTimer;

		bool refreshEnabled;
		unsigned int refreshInterval; // In seconds

		void updateRefreshTimer ();
};

#endif
