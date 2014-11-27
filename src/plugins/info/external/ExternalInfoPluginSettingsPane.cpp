#include "ExternalInfoPluginSettingsPane.h"

#include "src/util/qString.h"
#include "src/gui/dialogs.h"
#include "src/plugins/info/external/ExternalInfoPlugin.h"
#include "src/i18n/notr.h"

// TODO: file resolving sunset plugin style

ExternalInfoPluginSettingsPane::ExternalInfoPluginSettingsPane (ExternalInfoPlugin *plugin, QWidget *parent):
	PluginSettingsPane (parent),
	plugin (plugin)
{
	ui.setupUi (this);
}

ExternalInfoPluginSettingsPane::~ExternalInfoPluginSettingsPane()
{

}

void ExternalInfoPluginSettingsPane::readSettings ()
{
	ui.commandInput    ->setText    (plugin->getCommand  ());
	ui.richTextCheckbox->setChecked (plugin->getRichText ());
}

bool ExternalInfoPluginSettingsPane::writeSettings ()
{
	plugin->setCommand  (ui.commandInput    ->text      ());
	plugin->setRichText (ui.richTextCheckbox->isChecked ());
	return true;
}

void ExternalInfoPluginSettingsPane::on_browseButton_clicked ()
{
	QString command=Plugin::browse (ui.commandInput->text (), notr ("*"), getEffectivePluginPaths (), this);

	if (!command.isEmpty ())
		ui.commandInput->setText (command);
}

void ExternalInfoPluginSettingsPane::changeEvent (QEvent *event)
{
	if (event->type () == QEvent::LanguageChange)
	{
		ui.retranslateUi (this);
	}
	else
		PluginSettingsPane::changeEvent (event);
}
