#include "TestPluginSettingsPane.h"

#include "src/util/qString.h"
#include "src/gui/dialogs.h"
#include "src/plugins/info/test/TestPlugin.h"

TestPluginSettingsPane::TestPluginSettingsPane (TestPlugin *plugin, QWidget *parent):
	PluginSettingsPane (parent),
	plugin (plugin)
{
	ui.setupUi (this);
}

TestPluginSettingsPane::~TestPluginSettingsPane()
{

}

void TestPluginSettingsPane::readSettings ()
{
	ui.greetingNameInput->setText    (plugin->getGreetingName ());
	ui.richTextCheckbox ->setChecked (plugin->getRichText     ());
}

bool TestPluginSettingsPane::writeSettings ()
{
	if (ui.failCheckbox->isChecked ())
	{
		showWarning (tr ("Error"),
			tr ("Test error (\"Error on save\" is enabled)"),
			this);

		return false;
	}

	plugin->setGreetingName (ui.greetingNameInput->text      ());
	plugin->setRichText     (ui.richTextCheckbox ->isChecked ());
	return true;
}

void TestPluginSettingsPane::changeEvent (QEvent *event)
{
	if (event->type () == QEvent::LanguageChange)
	{
		ui.retranslateUi (this);
	}
	else
		PluginSettingsPane::changeEvent (event);
}
