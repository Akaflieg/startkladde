#ifndef TESTPLUGINSETTINGSPANE_H
#define TESTPLUGINSETTINGSPANE_H

#include "src/plugin/settings/PluginSettingsPane.h"
#include "ui_TestPluginSettingsPane.h"

#include <QWidget>

class TestPlugin;

class TestPluginSettingsPane: public PluginSettingsPane
{
		Q_OBJECT

	public:
		TestPluginSettingsPane (TestPlugin *plugin, QWidget *parent=NULL);
		~TestPluginSettingsPane();

	public slots:
		virtual void readSettings ();
		virtual bool writeSettings ();

	protected:
		virtual void changeEvent (QEvent *event);

	private:
		Ui::TestPluginSettingsPaneClass ui;

		TestPlugin *plugin;
};

#endif
