#ifndef EXTERNALINFOPLUGIN_H_
#define EXTERNALINFOPLUGIN_H_

#include <QProcess> // Required for QProcess::ExitStatus

#include "src/plugin/info/InfoPlugin.h"

class SkProcess;

/**
 * Example command: "date +%H:%M"
 */
class ExternalInfoPlugin: public InfoPlugin
{
		Q_OBJECT
		SK_PLUGIN

	public:
		ExternalInfoPlugin (const QString &caption=QString (), bool enabled=true, const QString &command="", bool richText=false);
		virtual ~ExternalInfoPlugin ();

		virtual void start ();
		virtual void terminate ();

		virtual PluginSettingsPane *infoPluginCreateSettingsPane (QWidget *parent=NULL);

		virtual void infoPluginReadSettings (const QSettings &settings);
		virtual void infoPluginWriteSettings (QSettings &settings);

		value_accessor (QString, Command , command );
		value_accessor (bool   , RichText, richText);

		virtual QString configText () const;

	protected slots:
		void languageChanged ();
		void lineReceived (const QString &line);
		void processExited (int exitCode, QProcess::ExitStatus exitStatus);


	private:
		QString command;
		bool richText;

		SkProcess *process;
};

#endif
