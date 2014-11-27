#include "SunsetPluginSettingsPane.h"

#include <QDebug>
#include <QFileInfo>

#include "src/plugins/info/sunset/SunsetPluginBase.h"
#include "src/util/qString.h"
#include "src/util/file.h"
#include "src/text.h"
#include "src/gui/dialogs.h"
#include "src/plugins/info/sunset/SunsetTimePlugin.h"
#include "src/i18n/notr.h"

SunsetPluginSettingsPane::SunsetPluginSettingsPane (SunsetPluginBase *plugin, QWidget *parent):
	PluginSettingsPane (parent),
	plugin (plugin),
	fileSpecified (false), fileResolved (false), fileExists (false), fileOk (false),
	referenceLongitudeFound (false)
{
	ui.setupUi (this);

	SunsetTimePlugin *sunsetTimePlugin=dynamic_cast<SunsetTimePlugin *> (plugin);
	ui.timeZoneLabel->setVisible (sunsetTimePlugin!=NULL);
	ui.timeZoneInput->setVisible (sunsetTimePlugin!=NULL);
}

SunsetPluginSettingsPane::~SunsetPluginSettingsPane()
{

}

void SunsetPluginSettingsPane::readSettings ()
{
	ui.filenameInput->setText (plugin->getFilename ());
	ui.longitudeInput->setLongitude (plugin->getLongitude ());
	ui.longitudeCorrectionCheckbox->setChecked (plugin->getLongitudeCorrection ());

	SunsetTimePlugin *sunsetTimePlugin=dynamic_cast<SunsetTimePlugin *> (plugin);
	if (sunsetTimePlugin)
		ui.timeZoneInput->setCurrentIndex (sunsetTimePlugin->getDisplayUtc ()?0:1);

	on_filenameInput_editingFinished ();
}

bool SunsetPluginSettingsPane::writeSettings ()
{
	plugin->setFilename (ui.filenameInput->text ());
	plugin->setLongitude (ui.longitudeInput->getLongitude ());
	plugin->setLongitudeCorrection (ui.longitudeCorrectionCheckbox->isChecked ());

	SunsetTimePlugin *sunsetTimePlugin=dynamic_cast<SunsetTimePlugin *> (plugin);
	if (sunsetTimePlugin)
		sunsetTimePlugin->setDisplayUtc (ui.timeZoneInput->currentIndex ()==0);

	return true;
}

void SunsetPluginSettingsPane::on_longitudeCorrectionCheckbox_toggled ()
{
	updateReferenceLongitudeNoteLabel ();
}

void SunsetPluginSettingsPane::on_filenameInput_editingFinished ()
{
	QString filename=ui.filenameInput->text ().trimmed ();

	fileSpecified=false;
	fileResolved=false;
	fileExists=false;
	fileOk=false;

	referenceLongitude=Longitude ();

	if (!isBlank (filename))
	{
		fileSpecified=true;

		QString resolved=plugin->resolveFilename (filename, getEffectivePluginPaths ());

		if (!resolved.isEmpty ())
		{
			fileResolved=true;
			resolvedFilename=QFileInfo (resolved).absoluteFilePath ();

			if (QFile::exists (resolved))
			{
				fileExists=true;

				try
				{
					source=SunsetPluginBase::readSource (resolved);

					QString referenceLongitudeString=SunsetPluginBase::readReferenceLongitudeString (resolved);
					referenceLongitudeFound=!referenceLongitudeString.isEmpty ();
					referenceLongitude=Longitude::parse (referenceLongitudeString);

					fileOk=true;
				}
				catch (FileOpenError &ex)
				{
					fileError=ex.errorString;
				}
			}
		}
	}

	updateFilenameLabel ();
	updateSourceLabel ();
	updateReferenceLongitudeLabel ();
	updateReferenceLongitudeNoteLabel ();
}


void SunsetPluginSettingsPane::on_findFileButton_clicked ()
{
	QString filename=Plugin::browse (ui.filenameInput->text (), notr (".txt"), getEffectivePluginPaths (), this);

	if (!filename.isEmpty ())
	{
		ui.filenameInput->setText (filename);
		on_filenameInput_editingFinished ();
	}
}


// ************
// ** Labels **
// ************

void SunsetPluginSettingsPane::updateFilenameLabel ()
{
	if (!fileSpecified)
		ui.filenameLabel->setTextAndDefaultColor (notr ("-"));
	else if (!fileResolved)
		ui.filenameLabel->setTextAndColor (tr ("not found"), Qt::red);
	else if (!fileExists)
		ui.filenameLabel->setTextAndColor (tr ("does not exist"), Qt::red);
	else if (!fileOk)
		ui.filenameLabel->setTextAndColor (tr ("%1\nError: %2").arg (resolvedFilename, fileError), Qt::red);
	else
		ui.filenameLabel->setTextAndDefaultColor (resolvedFilename);
}

void SunsetPluginSettingsPane::updateSourceLabel ()
{
	if (!fileOk)
		ui.sourceLabel->setText (notr ("-"));
	else if (source.isEmpty ())
		ui.sourceLabel->setText (tr ("unknown"));
	else
		ui.sourceLabel->setText (source);
}

void SunsetPluginSettingsPane::updateReferenceLongitudeLabel ()
{
	if (!fileOk)
		ui.referenceLongitudeLabel->setText (notr ("-"));
	else if (!referenceLongitudeFound)
		ui.referenceLongitudeLabel->setText (tr ("unknown"));
	else if (!referenceLongitude.isValid ())
		ui.referenceLongitudeLabel->setText (tr ("invalid"));
	else
		ui.referenceLongitudeLabel->setText (referenceLongitude.format ());
}

void SunsetPluginSettingsPane::updateReferenceLongitudeNoteLabel ()
{
	const QString referenceLongitudeNoteRegular=
		tr ("Longitude correction is only possible if a\nreference longitude is specified in the data file.");
	const QString referenceLongitudeNoteError=
		tr ("Longitude correction is not possible because\nno reference longitude is specified in the data file.");

	if (fileOk && !referenceLongitude.isValid ())
	{
		if (ui.longitudeCorrectionCheckbox->isChecked ())
			// Longitude correction activated - this is an error
			ui.referenceLongitudeNoteLabel->setTextAndColor (referenceLongitudeNoteError, Qt::red);
		else
			// Longitude correction not activated - this is ok
			ui.referenceLongitudeNoteLabel->setTextAndDefaultColor (referenceLongitudeNoteError);
	}
	else
	{
		ui.referenceLongitudeNoteLabel->setTextAndDefaultColor (referenceLongitudeNoteRegular);
	}
}

void SunsetPluginSettingsPane::changeEvent (QEvent *event)
{
	if (event->type () == QEvent::LanguageChange)
	{
		ui.retranslateUi (this);
	}
	else
		PluginSettingsPane::changeEvent (event);
}
