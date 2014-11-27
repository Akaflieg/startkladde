#ifndef SUNSETCOUNTDOWNPLUGIN_H_
#define SUNSETCOUNTDOWNPLUGIN_H_

#include "SunsetPluginBase.h"
#include "src/i18n/notr.h"

class SunsetCountdownPlugin: public SunsetPluginBase
{
		Q_OBJECT
		SK_PLUGIN

	public:
		SunsetCountdownPlugin (QString caption=QString (), bool enabled=true, const QString &filename=notr ("sunsets.txt"));
		virtual ~SunsetCountdownPlugin ();

		virtual void start ();

		virtual void minuteChanged ();

	protected slots:
		void languageChanged ();

	private:
		virtual void update ();
};

#endif
