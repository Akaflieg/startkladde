#ifndef INFOPLUGINSETTINGSPANE_H
#define INFOPLUGINSETTINGSPANE_H

#include <QtGui/QWidget>
#include "ui_InfoPluginSettingsPane.h"

#include "src/plugin/settings/PluginSettingsPane.h"

class InfoPlugin;
class PluginSettingsPane;

/**
 * The PluginSettingsPane implementation for an InfoPlugin
 *
 * This pane contains fields for the common properties of all InfoPlugins
 * as well as another PluginSettingsPane created by the
 * infoPluginCreateSettingsPane method of the plugin.
 */
class InfoPluginSettingsPane: public PluginSettingsPane
{
		Q_OBJECT

	public:
		InfoPluginSettingsPane (InfoPlugin *plugin, QWidget *parent=NULL);
		~InfoPluginSettingsPane ();

	public slots:
		virtual void readSettings ();
		virtual bool writeSettings ();

	protected:
		virtual void setSettingsWindow (SettingsWindow *settingsWindow);
		virtual void changeEvent (QEvent *event);

	private:
		Ui::InfoPluginSettingsPaneClass ui;

		InfoPlugin *plugin;
		PluginSettingsPane *infoPluginSettingsPane;
};

#endif
