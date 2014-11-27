#ifndef SunsetPluginSETTINGSPANE_H
#define SunsetPluginSETTINGSPANE_H

#include "src/plugin/settings/PluginSettingsPane.h"
#include "ui_SunsetPluginSettingsPane.h"

#include <QtGui/QWidget>

class SunsetPluginBase;

class SunsetPluginSettingsPane: public PluginSettingsPane
{
		Q_OBJECT

	public:
		SunsetPluginSettingsPane (SunsetPluginBase *plugin, QWidget *parent=NULL);
		~SunsetPluginSettingsPane();

	public slots:
		virtual void readSettings ();
		virtual bool writeSettings ();

	private slots:
		virtual void on_filenameInput_editingFinished ();
		virtual void on_findFileButton_clicked ();
		virtual void on_longitudeCorrectionCheckbox_toggled ();

		virtual void updateFilenameLabel ();
		virtual void updateSourceLabel ();
		virtual void updateReferenceLongitudeLabel ();
		virtual void updateReferenceLongitudeNoteLabel ();

	protected:
		virtual void changeEvent (QEvent *event);

	private:
		Ui::SunsetPluginSettingsPaneClass ui;

		SunsetPluginBase *plugin;

		bool fileSpecified;
		bool fileResolved;
		bool fileExists;
		bool fileOk;
		QString resolvedFilename;
		QString fileError;

		QString source;
		bool referenceLongitudeFound;
		Longitude referenceLongitude;
};

#endif
