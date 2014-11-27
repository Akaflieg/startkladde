#ifndef MetarPluginSETTINGSPANE_H
#define MetarPluginSETTINGSPANE_H

#include "src/plugin/settings/PluginSettingsPane.h"
#include "ui_MetarPluginSettingsPane.h"

#include <QtGui/QWidget>

class MetarPlugin;

class MetarPluginSettingsPane: public PluginSettingsPane
{
		Q_OBJECT

	public:
		MetarPluginSettingsPane (MetarPlugin *plugin, QWidget *parent=NULL);
		~MetarPluginSettingsPane();

	public slots:
		virtual void readSettings ();
		virtual bool writeSettings ();

	protected:
		virtual void changeEvent (QEvent *event);

	private:
		Ui::MetarPluginSettingsPaneClass ui;

		MetarPlugin *plugin;
};

#endif
