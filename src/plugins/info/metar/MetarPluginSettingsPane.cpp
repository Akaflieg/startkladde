#include "MetarPluginSettingsPane.h"

#include "src/util/qString.h"
#include "src/gui/dialogs.h"
#include "src/plugins/info/metar/MetarPlugin.h"

/*
 * TODO:
 *   - when retranslating, the airport input may not be drawn correctly until it
 *     is focused. A spacer between the airport input and the label to the right
 *     of it does not help.
 */

MetarPluginSettingsPane::MetarPluginSettingsPane (MetarPlugin *plugin, QWidget *parent):
	PluginSettingsPane (parent),
	plugin (plugin)
{
	ui.setupUi (this);

	ui.airportInput->setMinimumSize (100, 0);
}

MetarPluginSettingsPane::~MetarPluginSettingsPane()
{

}

void MetarPluginSettingsPane::readSettings ()
{
	ui.airportInput         ->setText  (plugin->getAirport         ()   );
	ui.refreshIntervalInput ->setValue (plugin->getRefreshInterval ()/60);
}

bool MetarPluginSettingsPane::writeSettings ()
{
	plugin->setAirport         (ui.airportInput        ->text  ()   );
	plugin->setRefreshInterval (ui.refreshIntervalInput->value ()*60);
	return true;
}

void MetarPluginSettingsPane::changeEvent (QEvent *event)
{
	if (event->type () == QEvent::LanguageChange)
	{
		ui.retranslateUi (this);
		adjustSize ();
	}
	else
		PluginSettingsPane::changeEvent (event);
}
