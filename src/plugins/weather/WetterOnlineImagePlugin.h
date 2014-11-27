#ifndef WETTERONLINEIMAGEPLUGIN_H_
#define WETTERONLINEIMAGEPLUGIN_H_

#include "src/plugin/weather/WeatherPlugin.h"

#include <QNetworkReply> // Required for QNetworkReply::NetworkError

class Downloader;

class WetterOnlineImagePlugin: public WeatherPlugin
{
		SK_PLUGIN
		Q_OBJECT

	public:
		WetterOnlineImagePlugin ();
		virtual ~WetterOnlineImagePlugin ();

	public slots:
		virtual void refresh ();
		virtual void abort ();

		void downloadSucceeded (int state, QNetworkReply *reply);
		void downloadFailed    (int state, QNetworkReply *reply, QNetworkReply::NetworkError code);

	protected slots:
		void languageChanged ();

	private:
		Downloader *downloader;
};

#endif
