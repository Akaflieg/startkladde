#ifndef PLUGINSETTINGSDIALOG_H
#define PLUGINSETTINGSDIALOG_H

#include "src/accessor.h"
#include "src/gui/SkDialog.h"
#include "src/plugin/info/InfoPlugin.h"

#include "ui_PluginSettingsDialog.h"

class Plugin;
class QWidget;
class PluginSettingsPane;
class SettingsWindow;

/**
 * A dialog which allows editing the settings of a plugin by means of its
 * PluginSettingsPane
 */
class PluginSettingsDialog: public SkDialog<Ui::PluginSettingsDialogClass>
{
		Q_OBJECT

	public:
		PluginSettingsDialog (Plugin *plugin, QWidget *parent=NULL, SettingsWindow *settingsWindow=NULL);
		~PluginSettingsDialog ();

		static int invoke (Plugin *plugin, QWidget *parent=NULL, SettingsWindow *settingsWindow=NULL);

	protected:
		void setupText ();

		virtual void languageChanged ();

	private slots:
		void on_buttonBox_accepted ();

	private:
		Plugin *plugin;
		PluginSettingsPane *settingsPane;
};

#endif
