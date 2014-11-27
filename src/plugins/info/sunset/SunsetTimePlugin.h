#ifndef SUNSETTIMEPLUGIN_H_
#define SUNSETTIMEPLUGIN_H_

#include "SunsetPluginBase.h"
#include "src/i18n/notr.h"

class SunsetTimePlugin: public SunsetPluginBase
{
	Q_OBJECT
	SK_PLUGIN

	public:
		SunsetTimePlugin (QString caption=QString (), bool enabled=true, const QString &filename=notr ("sunsets.txt"));
		virtual ~SunsetTimePlugin ();

		virtual void start ();

		value_accessor (bool, DisplayUtc, displayUtc);

		virtual void infoPluginReadSettings (const QSettings &settings);
		virtual void infoPluginWriteSettings (QSettings &settings);


	protected slots:
		void languageChanged ();

	private:
		bool displayUtc;
};

#endif
