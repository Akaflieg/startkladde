#ifndef EXTERNALWEATHERPLUGIN_H_
#define EXTERNALWEATHERPLUGIN_H_

#include <QProcess> // Required for QProcess::ExitStatus

#include "src/plugin/weather/WeatherPlugin.h"

class SkProcess;

class ExternalWeatherPlugin: public WeatherPlugin
{
		Q_OBJECT
		SK_PLUGIN

	public:
		ExternalWeatherPlugin (const QString &command="");
		virtual ~ExternalWeatherPlugin ();

		value_accessor (QString, Command, command);

	public slots:
		virtual void refresh ();
		virtual void abort ();

	protected slots:
		void languageChanged ();
		void lineReceived (const QString &line);
		void processExited (int exitCode, QProcess::ExitStatus exitStatus);

	private:
		QString command;

		SkProcess *process;
};

#endif
