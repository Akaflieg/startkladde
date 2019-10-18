#ifndef PLUGINSETTINGSPANE_H_
#define PLUGINSETTINGSPANE_H_

#include <QWidget>

#include "src/accessor.h"

class SettingsWindow;

// Note that we cannot inherit the plugin settings pane from an SkDialog-like
// SkWidget because they inherit from PluginSettingsPane, and PluginSettingsPane
// cannot inherit from SkWidget because SkWidget would have to be instantiated
// with the Ui class which is different for different plugin settings panes.

/**
 * A QWidget with fields for configuring a plugin instance
 *
 * A PluginSettingPane contains widgets for editing properties of a Plugin.
 * Generally, there will be a PluginSettingsPane implementation for each Plugin
 * implementation.
 *
 * A PluginSettingsPane instance is created for a specific Plugin instance.
 * Implementations will generally store a link to the plugin to be configured.
 */
class PluginSettingsPane: public QWidget
{
		Q_OBJECT

	public:
		PluginSettingsPane (QWidget *parent=NULL);
		virtual ~PluginSettingsPane ();

	public slots:
		/**
		 * Reads the settings from the plugin and sets up the input fields
		 * accordingly
		 */
		virtual void readSettings ()=0;

		/**
		 * Takes the settings from the input fields and writes them to the
		 * plugin
		 *
		 * Consistency checks may be performed here. This method may return
		 * false in order to indicate canceling by the user or an error. It is
		 * responsible for notifying the user when false is returned, unless
		 * false is returned in response to user interaction.
		 *
		 * If false is returned, so settings of the plugin may have been
		 * changed.
		 *
		 * @return true on success, false if canceled
		 */
		virtual bool writeSettings ()=0;

	public:
		// TODO virtual attribute accessors
		virtual void setSettingsWindow (SettingsWindow *settingsWindow) { this->settingsWindow=settingsWindow; }

	protected:
		QStringList getEffectivePluginPaths ();

		// TODO: it would be better to pass a temporary Settings instance
		SettingsWindow *settingsWindow;
};

#endif
