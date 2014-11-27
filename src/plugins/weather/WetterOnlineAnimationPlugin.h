#ifndef WETTERONLINEANIMATIONPLUGIN_H_
#define WETTERONLINEANIMATIONPLUGIN_H_

#include "src/plugin/weather/WeatherPlugin.h"

#include <QNetworkReply> // Required for QNetworkReply::NetworkError

class Downloader;

class WetterOnlineAnimationPlugin: public WeatherPlugin
{
		SK_PLUGIN
		Q_OBJECT

	public:
		WetterOnlineAnimationPlugin ();
		virtual ~WetterOnlineAnimationPlugin ();

	private:
		Downloader *downloader;

	public slots:
		virtual void refresh ();
		virtual void abort ();

		void downloadSucceeded (int state, QNetworkReply *reply);
		void downloadFailed    (int state, QNetworkReply *reply, QNetworkReply::NetworkError code);

	protected slots:
		void languageChanged ();
};

#endif
