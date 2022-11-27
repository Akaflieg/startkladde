/*
 * Improvements:
 *   - for the plugins, we should use a model and QTreeView instead of
 *     QTreeWidget and manual syncing between plugin list and view
 *   - pluginPathList: after dragging, select the dragged item in the new
 *     position
 *   - infoPluginList: enable internal dragging
 *   - pluginPathList/infoPluginList:
 *     - contextmenu
 *     - swapping with alt+up/alt+down
 *     - when double-clicking in the empty area, add an item
 *   - reset all to default
 *   - add an indication that the database name/port have been overridden by a
 *     command line argument and will not be saved
 */
#include "SettingsWindow.h"

#include <cassert>
#include <iostream>

#include <QDebug>
#include <QFileDialog>
#include <QInputDialog>
#include <QItemEditorFactory>
#include <QPushButton>
#include <QSettings>
#include <QStringList>
#include <QShowEvent>
#include <QSerialPortInfo>

#include "src/text.h"
#include "src/config/Settings.h"
#include "src/db/DatabaseInfo.h"
#include "src/gui/dialogs.h"
#include "src/gui/views/ReadOnlyItemDelegate.h"
#include "src/gui/views/SpinBoxCreator.h"
#include "src/gui/views/SpecialIntDelegate.h"
#include "src/i18n/notr.h"
#include "src/i18n/TranslationManager.h"
#include "src/plugin/info/InfoPlugin.h"
#include "src/plugin/factory/PluginFactory.h"
#include "src/plugin/info/InfoPluginSelectionDialog.h"
#include "src/plugin/settings/PluginSettingsDialog.h"
#include "src/plugins/weather/ExternalWeatherPlugin.h"
#include "src/util/fileSystem.h"
#include "src/util/qString.h"
#include "src/util/qList.h"

const int captionColumn=0;
const int    nameColumn=1;
const int enabledColumn=2;
const int  configColumn=3;


// ******************
// ** Construction **
// ******************

SettingsWindow::SettingsWindow (QWidget *parent):
	SkDialog<Ui::SettingsWindowClass> (parent),
	warned (false),
	databaseSettingsChanged (false)
{
	// TODO there should be a warning if the settings can't be saved without a password

	ui.setupUi (this);

	prepareText ();
	setupText ();

	ui.tabWidget->setCurrentIndex (0);

	ui.dbTypePane->setVisible (false);

	ui.languageInput->setSizeAdjustPolicy (QComboBox::AdjustToContents);
	ui.languageInput->setLanguageItems (TranslationManager::instance ().listLanguages ());

	// Populate the Flarm connection type list
	foreach (Flarm::ConnectionType type, Flarm::ConnectionType_list ())
	{
		QString text=Flarm::ConnectionType_text (type);
		ui.flarmConnectionTypeInput->addItem (text, type);
	}

	populateSerialPortList ();

	readSettings ();
	updateWidgets ();
}

SettingsWindow::~SettingsWindow()
{
	deleteList (infoPlugins);
}


// ***********
// ** Setup **
// ***********

void SettingsWindow::prepareText ()
{
	weatherPlugins=PluginFactory::getInstance ().getDescriptors<WeatherPlugin> ();
    std::sort (weatherPlugins.begin (), weatherPlugins.end (), WeatherPlugin::Descriptor::nameLessThanP);

	// Weather plugin lists
	ui.weatherPluginInput      ->addItem (notr ("-"), QString ());
	ui.weatherWindowPluginInput->addItem (notr ("-"), QString ());

	foreach (const WeatherPlugin::Descriptor *descriptor, weatherPlugins)
	{
        QString id  =descriptor->getId   ().toString();
		ui.weatherPluginInput      ->addItem ("", id);
		ui.weatherWindowPluginInput->addItem ("", id);
	}
}

void SettingsWindow::setupText ()
{
	// Note that this label does not use wordWrap because it causes the minimum
	// size of the label not to work properly.
	ui.passwordMessageLabel->setText (tr (
		"The password protection can be removed by deleting or editing"
		" the configuration file or registry key %1.")
		.arg (QSettings ().fileName ()));

	// Weather plugin lists
	foreach (const WeatherPlugin::Descriptor *descriptor, weatherPlugins)
	{
		QString name=descriptor->getName ();
        QString id  =descriptor->getId   ().toString();
		ui.weatherPluginInput      ->setItemTextByItemData (id, name);
		ui.weatherWindowPluginInput->setItemTextByItemData (id, name);
	}

	// Required because we have a label with word wrap
	adjustSize ();
}

/**
 * A very special sorting criterion that defaults to stringNumericLessThan, but
 * sorts all strings containing "USB" to the front of the list.
 *
 * Examples:
 *   - ttyUSB1 < ttyS0 (entries containing "USB" before others)
 *   - ttyUSB1 < ttyUSB10 (entries containing "USB": use stringNumericLessThan)
 *   - ttyS1 < ttyS10 (entries not containing "USB": use stringNumericLessThan)
 */
bool serialPortLessThan (const QSerialPortInfo &s1, const QSerialPortInfo &s2)
{
    bool usb1=s1.portName().contains ("USB", Qt::CaseInsensitive);
    bool usb2=s2.portName().contains ("USB", Qt::CaseInsensitive);

	if (usb1 == usb2)
        return stringNumericLessThan (s1.portName(), s2.portName());
	else
		return usb1;
}

void SettingsWindow::populateSerialPortList ()
{
	// The edit text will be replaced with the first entry added to the list, so
	// we have to preserve it.
	QString editText=ui.flarmSerialPortInput->currentText ();

	// TODO in the serial ports list, also indicate .isBusy (); however: (a)
	// don't indicate a port as busy if we're using it ourselves, and (b) update
	// the list when the status of a port changes.

	ui.flarmSerialPortInput->clear ();

    QList<QSerialPortInfo> serialPortList = QSerialPortInfo::availablePorts();
    std::sort (serialPortList.begin (), serialPortList.end (), serialPortLessThan);

    foreach (const QSerialPortInfo &portInfo, serialPortList)
	{
        QString deviceDescription= portInfo.description();

		QString text;
		if (isBlank (deviceDescription))
            text=tr ("%1").arg (portInfo.portName());
		else
            text=tr ("%1 (%2)").arg (portInfo.portName()).arg (deviceDescription);

        ui.flarmSerialPortInput->addItem (text, portInfo.portName());
	}

	// Make boolean columns and some other columns read-only
	// The title column is read-only because we would have to write back the
	// value to the plugin after editing it so the plugin settings dialog show
	// it correctly.
	ui.infoPluginList->setItemDelegateForColumn (   nameColumn, new ReadOnlyItemDelegate (ui.infoPluginList));
	ui.infoPluginList->setItemDelegateForColumn (captionColumn, new ReadOnlyItemDelegate (ui.infoPluginList));
	ui.infoPluginList->setItemDelegateForColumn (enabledColumn, new ReadOnlyItemDelegate (ui.infoPluginList));
	ui.infoPluginList->setItemDelegateForColumn ( configColumn, new ReadOnlyItemDelegate (ui.infoPluginList));

	// Restore the edit text. Note that the value will be restored even if the
	// corresponding entry in the list (if any) does not exist any more. This is
	// the correct behavior as the user may enter any port, even one that is not
	// in the list.
	// Note further that the cursor position and text selection may be changed.
	// This only seems to be the case if the port that is currently entered in
	// the text edit field is removed or added.
	ui.flarmSerialPortInput->setEditText (editText);
}


// **************
// ** Settings **
// **************

void SettingsWindow::readItem (QTreeWidgetItem *item, const InfoPlugin *plugin)
{
	item->setData       (captionColumn, Qt::DisplayRole, plugin->getCaption ());
	item->setData       (   nameColumn, Qt::DisplayRole, plugin->getName ());
	item->setCheckState (enabledColumn, plugin->isEnabled ()?Qt::Checked:Qt::Unchecked);
	item->setData       ( configColumn, Qt::DisplayRole, plugin->configText ());

	item->setFlags (item->flags () | Qt::ItemIsEditable | Qt::ItemIsUserCheckable);
}

void SettingsWindow::makeItemEditable (QListWidgetItem *item)
{
	item->setFlags (item->flags () | Qt::ItemIsEditable);
}

void SettingsWindow::readSettings ()
{
	Settings &s=Settings::instance ();
	DatabaseInfo &info=s.databaseInfo;

	// *** Database
	ui.mysqlServerInput        ->setText    (info.server);
	ui.mysqlDefaultPortCheckBox->setChecked (info.defaultPort);
	ui.mysqlPortInput          ->setValue   (info.port);
	ui.mysqlUserInput          ->setText    (info.username);
	ui.mysqlPasswordInput      ->setText    (info.password);
	ui.mysqlDatabaseInput      ->setText    (info.database);

	// *** Settings
	// UI
	ui.languageInput->setCurrentItem (s.languageConfiguration);
	// Data
	ui.locationInput         ->setText    (s.location);
	ui.recordTowpilotCheckbox->setChecked (s.recordTowpilot);
	ui.checkMedicalsCheckbox ->setChecked (s.checkMedicals);
    ui.anonymousCheckbox     ->setChecked (s.anonymousMode);
    // Vereinsflieger
    ui.vfUploadEnabled  ->setChecked    (s.vfUploadEnabled);
    ui.vfApiKeyInput    ->setText       (s.vfApiKey);
    ui.vfCIDInput       ->setText       (s.vfClubId);
	// Flarm
	ui.flarmGroupBox              ->setChecked  (s.flarmEnabled);
	ui.flarmConnectionTypeInput   ->setCurrentItemByItemData (
	                                             s.flarmConnectionType, 0);
	ui.flarmSerialPortInput       ->setEditText (s.flarmSerialPort);
	ui.flarmSerialBaudRateInput   ->setEditText (QString::number (
	                                             s.flarmSerialBaudRate));
	ui.flarmTcpHostInput          ->setText     (s.flarmTcpHost);
	ui.flarmTcpPortInput          ->setValue    (s.flarmTcpPort);
	ui.flarmFileNameInput         ->setText     (s.flarmFileName);
	ui.flarmFileDelayInput        ->setValue    (s.flarmFileDelayMs);
	ui.flarmAutoDeparturesCheckbox->setChecked  (s.flarmAutoDepartures);
    ui.flarmActivateRangeCheckbox ->setChecked  (s.flarmRange != 0);
    ui.flarmRangeInput            ->setValue    (s.flarmRange);
	ui.flarmDataViewableCheckbox  ->setChecked  (s.flarmDataViewable);
	ui.flarmMapKmlFileNameInput   ->setText     (s.flarmMapKmlFileName);
	// FlarmNet
	ui.flarmNetEnabledCheckbox    ->setChecked (s.flarmNetEnabled);
	// Permissions
	ui.protectSettingsCheckbox      ->setChecked (s.protectSettings);
	ui.protectLaunchMethodsCheckbox ->setChecked (s.protectLaunchMethods);
	ui.protectMergePeopleCheckbox   ->setChecked (s.protectMergePeople);
	ui.protectFlightDatabaseCheckbox->setChecked (s.protectFlightDatabase);
	ui.protectViewMedicalsCheckbox  ->setChecked (s.protectViewMedicals);
	ui.protectChangeMedicalsCheckbox->setChecked (s.protectChangeMedicals);
	// Diagnostics
	ui.enableDebugCheckbox->setChecked (s.enableDebug);
	ui.diagCommandInput   ->setText    (s.diagCommand);

	// *** Plugins - Info
	deleteList (infoPlugins);
	infoPlugins=s.readInfoPlugins ();

	ui.infoPluginList->clear ();
	foreach (InfoPlugin *plugin, infoPlugins)
	{
		QTreeWidgetItem *item=new QTreeWidgetItem (ui.infoPluginList);
		readItem (item, plugin);
	}
	for (int i=0; i<ui.infoPluginList->columnCount (); ++i)
		ui.infoPluginList->resizeColumnToContents (i);


	// *** Plugins - Weather

	// Weather plugin
	ui.weatherPluginBox          ->setChecked               (s.weatherPluginEnabled);
	ui.weatherPluginInput        ->setCurrentItemByItemData (s.weatherPluginId, 0);
	ui.weatherPluginCommandInput ->setText                  (s.weatherPluginCommand );
	ui.weatherPluginHeightInput  ->setValue                 (s.weatherPluginHeight  );
	ui.weatherPluginIntervalInput->setValue                 (s.weatherPluginInterval/60);
    on_weatherPluginInput_currentIndexChanged (ui.weatherPluginInput->currentIndex());

	// Weather dialog
	ui.weatherWindowBox          ->setChecked               (s.weatherWindowEnabled);
	ui.weatherWindowPluginInput  ->setCurrentItemByItemData (s.weatherWindowPluginId, 0);
	ui.weatherWindowCommandInput ->setText                  (s.weatherWindowCommand );
	ui.weatherWindowIntervalInput->setValue                 (s.weatherWindowInterval/60);
	ui.weatherWindowTitleInput   ->setText                  (s.weatherWindowTitle   );
    on_weatherWindowPluginInput_currentIndexChanged (ui.weatherPluginInput->currentIndex());

	// *** Plugins - Paths
	ui.pluginPathList->clear ();
	foreach (const QString &pluginPath, s.pluginPaths)
		ui.pluginPathList->addItem (pluginPath);

	int n=ui.pluginPathList->count ();
	for (int i=0; i<n; ++i)
		makeItemEditable (ui.pluginPathList->item (i));

	updateWidgets ();
}

void SettingsWindow::writeSettings ()
{
	Settings &s=Settings::instance ();
	DatabaseInfo &info=s.databaseInfo;

	DatabaseInfo oldInfo=info;

	// *** Database
	info.server     =ui.mysqlServerInput        ->text ();
	info.defaultPort=ui.mysqlDefaultPortCheckBox->isChecked ();
	info.port       =ui.mysqlPortInput          ->value ();
	info.username   =ui.mysqlUserInput          ->text ();
	info.password   =ui.mysqlPasswordInput      ->text ();
	info.database   =ui.mysqlDatabaseInput      ->text ();

    // *** Settings
	// Language
	s.languageConfiguration=ui.languageInput->getSelectedLanguageConfiguration ();
	// Data
	s.location      =ui.locationInput         ->text ();
	s.recordTowpilot=ui.recordTowpilotCheckbox->isChecked ();
	s.checkMedicals =ui.checkMedicalsCheckbox ->isChecked ();
    s.anonymousMode =ui.anonymousCheckbox     ->isChecked ();
    // Vereinsflieger
    s.vfUploadEnabled   =ui.vfUploadEnabled ->isChecked();
    s.vfApiKey          =ui.vfApiKeyInput   ->text();
    s.vfClubId          =ui.vfCIDInput      ->text();
	// Flarm
	s.flarmEnabled	     =ui.flarmGroupBox              ->isChecked ();
	s.flarmConnectionType=(Flarm::ConnectionType)
	                      ui.flarmConnectionTypeInput   ->currentItemData ().toInt ();
	s.flarmSerialPort    =ui.flarmSerialPortInput       ->currentText ();
	s.flarmSerialBaudRate=ui.flarmSerialBaudRateInput   ->currentText ().toInt ();
	s.flarmTcpHost       =ui.flarmTcpHostInput          ->text ();
    s.flarmTcpPort       = (uint16_t) ui.flarmTcpPortInput          ->value ();
	s.flarmFileName      =ui.flarmFileNameInput         ->text ();
	s.flarmFileDelayMs   =ui.flarmFileDelayInput        ->value ();
	s.flarmAutoDepartures=ui.flarmAutoDeparturesCheckbox->isChecked ();
    s.flarmRange         =ui.flarmActivateRangeCheckbox->isChecked() ? ui.flarmRangeInput->value() : 0;
	s.flarmDataViewable  =ui.flarmDataViewableCheckbox  ->isChecked ();
	s.flarmMapKmlFileName=ui.flarmMapKmlFileNameInput   ->text ();
	// FlarmNet
	s.flarmNetEnabled	=ui.flarmNetEnabledCheckbox     ->isChecked ();
	// Permissions
	s.protectSettings      =ui.protectSettingsCheckbox      ->isChecked ();
	s.protectLaunchMethods =ui.protectLaunchMethodsCheckbox ->isChecked ();
	s.protectMergePeople   =ui.protectMergePeopleCheckbox   ->isChecked ();
	s.protectFlightDatabase=ui.protectFlightDatabaseCheckbox->isChecked ();
	s.protectViewMedicals  =ui.protectViewMedicalsCheckbox  ->isChecked ();
	s.protectChangeMedicals=ui.protectChangeMedicalsCheckbox->isChecked ();
	// Diagnostics
	s.enableDebug=ui.enableDebugCheckbox->isChecked ();
	s.diagCommand=ui.diagCommandInput   ->text ();

	// *** Plugins - Info
	int numInfoPlugins=infoPlugins.size ();
	assert (numInfoPlugins==ui.infoPluginList->topLevelItemCount ());
	for (int i=0; i<numInfoPlugins; ++i)
	{
		QTreeWidgetItem &item=*ui.infoPluginList->topLevelItem (i);
		infoPlugins[i]->setCaption (item.data       (captionColumn, Qt::DisplayRole).toString ());
		infoPlugins[i]->setEnabled (item.checkState (enabledColumn                 )==Qt::Checked);
	}
	s.writeInfoPlugins (infoPlugins);


	// *** Plugins - Weather
	// Weather plugin
	s.weatherPluginEnabled =ui.weatherPluginBox          ->isChecked ();
	s.weatherPluginId      =ui.weatherPluginInput        ->currentItemData ().toString ();
	s.weatherPluginCommand =ui.weatherPluginCommandInput ->text ();
	s.weatherPluginHeight  =ui.weatherPluginHeightInput  ->value ();
	s.weatherPluginInterval=ui.weatherPluginIntervalInput->value ()*60;
	// Weather dialog
	s.weatherWindowEnabled =ui.weatherWindowBox          ->isChecked ();
	s.weatherWindowPluginId=ui.weatherWindowPluginInput  ->currentItemData ().toString ();
	s.weatherWindowCommand =ui.weatherWindowCommandInput ->text ();
	s.weatherWindowInterval=ui.weatherWindowIntervalInput->value ()*60;
	s.weatherWindowTitle   =ui.weatherWindowTitleInput   ->text ();

	// *** Plugins - Paths
	s.pluginPaths=getPluginPaths ();

	s.save ();

	databaseSettingsChanged=oldInfo.different (info);
}

QStringList SettingsWindow::getPluginPaths ()
{
	QStringList pluginPaths;

	int numPluginPaths=ui.pluginPathList->count ();

	for (int i=0; i<numPluginPaths; ++i)
		pluginPaths << ui.pluginPathList->item (i)->text ();

	return pluginPaths;
}


// ***********
// ** Flarm **
// ***********

void SettingsWindow::on_browseFlarmFileButton_clicked ()
{
	// TODO: code duplication - browse functionality
	QString currentFileName=ui.flarmFileNameInput->text ();

	QString fileName=QFileDialog::getOpenFileName (
		this,
		tr ("Select Flarm file"),
		existingParentDirectory (currentFileName, QDir ()).absolutePath (),
		notr ("*.txt;;*"),
		NULL,
		QFileDialog::ReadOnly
		);

	if (!fileName.isEmpty ())
		ui.flarmFileNameInput->setText (fileName);
}

// *****************
// ** Plugin path **
// *****************

void SettingsWindow::on_addPluginPathButton_clicked ()
{
	warnEdit ();
	QListWidget *list=ui.pluginPathList;

	list->addItem ("");
	makeItemEditable (list->item (list->count ()-1));
	list->setCurrentRow (list->count ()-1);
	list->editItem (list->item (list->count ()-1));
}

void SettingsWindow::on_removePluginPathButton_clicked ()
{
	warnEdit ();
	QListWidget *list=ui.pluginPathList;

	int row=list->currentRow ();
	if (row<0 || row>=list->count ()) return;
	delete list->takeItem (row);
	if (row>=list->count ()) --row;
	if (row>=0) list->setCurrentRow (row);
}

void SettingsWindow::on_pluginPathUpButton_clicked ()
{
	warnEdit ();
	QListWidget *list=ui.pluginPathList;

	int row=list->currentRow ();
	if (row<0 || row>=list->count ()) return;
	if (row==0) return;

	list->insertItem (row-1, list->takeItem (row));
	list->setCurrentRow (row-1);
}

void SettingsWindow::on_pluginPathDownButton_clicked ()
{
	warnEdit ();
	QListWidget *list=ui.pluginPathList;

	int row=list->currentRow ();
	if (row<0 || row>=list->count ()) return;
	if (row==list->count ()-1) return;

	list->insertItem (row+1, list->takeItem (row));
	list->setCurrentRow (row+1);
}


// ******************
// ** Info plugins **
// ******************

void SettingsWindow::on_addInfoPluginButton_clicked ()
{
	warnEdit ();
	QTreeWidget *list=ui.infoPluginList;

	QList<const InfoPlugin::Descriptor *> descriptors=PluginFactory::getInstance ().getDescriptors<InfoPlugin> ();
	const InfoPlugin::Descriptor *descriptor=InfoPluginSelectionDialog::select (descriptors, this);

	if (!descriptor) return;

	InfoPlugin *plugin=descriptor->create ();

	if (plugin)
	{
		plugin->setCaption (plugin->getName ()+notr (":"));
		int settingsDialogResult=PluginSettingsDialog::invoke (plugin, this, this);

		if (settingsDialogResult==QDialog::Accepted)
		{
			infoPlugins.append (plugin);

			QTreeWidgetItem *item=new QTreeWidgetItem (list);
			readItem (item, plugin);

			list->setCurrentItem (item);
		}
		else
		{
			delete plugin;
		}
	}
}

void SettingsWindow::on_removeInfoPluginButton_clicked ()
{
	warnEdit ();
 	QTreeWidget *list=ui.infoPluginList;

	int row=list->indexOfTopLevelItem (list->currentItem ());
	if (row<0 || row>=list->topLevelItemCount ()) return;

	delete list->takeTopLevelItem (row);
	delete infoPlugins.takeAt (row);

	if (row>=list->topLevelItemCount ()) --row;
	if (row>=0) list->setCurrentItem (list->topLevelItem (row));
}

void SettingsWindow::on_infoPluginUpButton_clicked ()
{
	warnEdit ();
	QTreeWidget *list=ui.infoPluginList;

	int row=list->indexOfTopLevelItem (list->currentItem ());
	if (row<0 || row>=list->topLevelItemCount ()) return;
	if (row==0) return;

	list->insertTopLevelItem (row-1, list->takeTopLevelItem (row));
	infoPlugins.insert (row-1, infoPlugins.takeAt (row));

	list->setCurrentItem (list->topLevelItem (row-1));
}

void SettingsWindow::on_infoPluginDownButton_clicked ()
{
	warnEdit ();
	QTreeWidget *list=ui.infoPluginList;

	int row=list->indexOfTopLevelItem (list->currentItem ());
	if (row<0 || row>=list->topLevelItemCount ()) return;
	if (row==list->topLevelItemCount ()-1) return;

	list->insertTopLevelItem (row+1, list->takeTopLevelItem (row));
	infoPlugins.insert (row+1, infoPlugins.takeAt (row));

	list->setCurrentItem (list->topLevelItem (row+1));
}

void SettingsWindow::on_infoPluginSettingsButton_clicked ()
{
	warnEdit ();
	QTreeWidget *list=ui.infoPluginList;

	int row=list->indexOfTopLevelItem (list->currentItem ());
	if (row<0 || row>=list->topLevelItemCount ()) return;

	PluginSettingsDialog::invoke (infoPlugins[row], this, this);
	readItem (ui.infoPluginList->topLevelItem (row), infoPlugins[row]);
}

void SettingsWindow::on_infoPluginList_itemDoubleClicked (QTreeWidgetItem *item, int column)
{
	(void)column;
	if (!item) return;

	ui.infoPluginList->setCurrentItem (item);
	on_infoPluginSettingsButton_clicked ();
}


// *********************
// ** Weather plugins **
// *********************

void SettingsWindow::on_weatherPluginInput_currentIndexChanged (int index)
{
    Q_UNUSED(index)

    bool external=(ui.weatherPluginInput->currentItemData ().toString ()==ExternalWeatherPlugin::_getId ().toString());
	ui.weatherPluginCommandLabel->setEnabled (external);
	ui.weatherPluginCommandInput->setEnabled (external);
	ui.browseWeatherPluginCommandButton->setEnabled (external);
}

void SettingsWindow::on_weatherWindowPluginInput_currentIndexChanged (int index)
{
    Q_UNUSED(index)

    bool external=(ui.weatherWindowPluginInput->currentItemData ().toString ()==ExternalWeatherPlugin::_getId ().toString());
	ui.weatherWindowCommandLabel->setEnabled (external);
	ui.weatherWindowCommandInput->setEnabled (external);
	ui.browseWeatherWindowCommandButton->setEnabled (external);
}

void SettingsWindow::on_browseWeatherPluginCommandButton_clicked ()
{
	QString filename=Plugin::browse (ui.weatherPluginCommandInput->text (), notr ("*"), getPluginPaths (), this);

	if (!filename.isEmpty ())
		ui.weatherPluginCommandInput->setText (filename);
}

void SettingsWindow::on_browseWeatherWindowCommandButton_clicked ()
{
	QString filename=Plugin::browse (ui.weatherWindowCommandInput->text (), notr ("*"), getPluginPaths (), this);

	if (!filename.isEmpty ())
		ui.weatherWindowCommandInput->setText (filename);
}

void SettingsWindow::on_browseKmlFileButton_clicked ()
{
	QString currentFileName=ui.flarmMapKmlFileNameInput->text ();


	QString fileName=QFileDialog::getOpenFileName (
		this,
		tr ("Select KML file"),
		existingParentDirectory (currentFileName, QDir ()).absolutePath (),
		notr ("*.kml"),
		NULL,
		QFileDialog::ReadOnly
		);

	if (!fileName.isEmpty ())
		ui.flarmMapKmlFileNameInput->setText (fileName);
}



// *************
// ** Closing **
// *************

void SettingsWindow::reject ()
{
	SkDialog<Ui::SettingsWindowClass>::reject ();
	// Load the original language. Nothing will happen if the language did not
	// change.
	TranslationManager::instance ().load (Settings::instance ().languageConfiguration);
}

void SettingsWindow::on_buttonBox_accepted ()
{
	if (allowEdit ())
	{
		writeSettings ();
		close ();
	}
}



// **********
// ** Misc **
// **********

/**
 * Determines whether the settings may be stored.
 *
 * Changing the settings may be protected, that is, require that the database
 * password is entered. However, it's not simply a matter of asking the password
 * in this case. This method determines whether the settings may be store, and
 * asks the user for a password if required.
 */
bool SettingsWindow::allowEdit ()
{
	// TODO should the use PasswordPermission?
	QString message;
	QString requiredPassword;

	QString oldPassword=Settings::instance ().databaseInfo.password;
	QString newPassword=ui.mysqlPasswordInput->text ();
	bool passwordChanged=(newPassword!=oldPassword);

	if (Settings::instance ().protectSettings)
	{
		// The password protection is (already was) enabled. The user must enter
		// the password.
		// If the password was also changed, clarify that the user must enter
		// the *old* password.
		if (passwordChanged)
			message=tr ("The (old) database password must be entered to save\nthe settings.");
		else
			message=tr ("The database password must be entered to save\nthe settings.");
		requiredPassword=oldPassword;
	}
	else if (ui.protectSettingsCheckbox->isChecked ())
	{
		// The password protection was disabled before, but has been enabled.
		// The user must enter the database password to make sure that he
		// actually knows the password, so he won't enable the protection
		// without having a way to disable it again.
		// If the password was also changed, clarify that the user must enter
		// the *new* password.
		if (passwordChanged)
			message=tr (
				"Password protection of the settings is being enabled. The\n"
				"(new) database password must be entered. If you don't want\n"
				"to enable the protection, you can cancel now and disable\n"
				"the corresponding option."
				);
		else
			message=tr (
				"Password protection of the settings is being enabled. The\n"
				"database password must be entered. If you don't want\n"
				"to enable the protection, you can cancel now and disable\n"
				"the corresponding option."
				);
		requiredPassword=newPassword;
	}
	else
	{
		// The password protection is not currently enabled, nor has it been
		// enabled. We can store the settings.
		return true;
	}

	// If we didn't return yet, we may only store the settings if the user
	// enters the correct password. Which one is the "correct" password has been
	// determined before.
	return verifyPassword (this, requiredPassword, message);
}

void SettingsWindow::warnEdit ()
{
	if (!Settings::instance ().protectSettings) return;
	if (warned) return;

	showWarning (tr ("Settings protected"), tr (
		"The settings are protected. The settings\n"
		"can be changed, but to save them, the database\n"
		"password must be entered."), this);

	warned=true;
}

void SettingsWindow::showEvent (QShowEvent *event)
{
	// When the dialog is made visible (showEvent), show a warning if the
	// settings are password protected.  This warning will only be shown once
	// during the lifetime of the SettingsWindow instance.
	// Note that when this is active, all other calls to warnEdit are
	// unnecessary (but harmless).
	if (!event->spontaneous ())
		warnEdit ();
}

void SettingsWindow::languageChanged ()
{
	SkDialog<Ui::SettingsWindowClass>::languageChanged ();
	setupText ();
	for (int i=0, n=ui.infoPluginList->columnCount (); i<n; ++i)
		ui.infoPluginList->resizeColumnToContents (i);
}

// Use the "activated" signal rather than the "currentIndexChanged" signal to
// avoid loading another language on startup, before the index has been set to
// the current setting. This does have the disadvantage of being called when the
// same setting as before is selected.
void SettingsWindow::on_languageInput_activated (int index)
{
	// Load the selected language
	LanguageConfiguration languageConfiguration=ui.languageInput->getLanguageConfiguration (index);
	TranslationManager::instance ().load (languageConfiguration);

}
void SettingsWindow::updateWidgets ()
{
	// MySQL
	ui.mysqlPortInput->setEnabled (!ui.mysqlDefaultPortCheckBox->isChecked ());

    // Vereinsflieger
    ui.vfConnectionGroupBox->setEnabled (ui.vfUploadEnabled->isChecked());

	// Flarm
	QVariant connectionTypeValue=ui.flarmConnectionTypeInput->currentItemData ();
	Flarm::ConnectionType connectionType=(Flarm::ConnectionType)connectionTypeValue.toInt ();

	ui.flarmSerialPortLabel    ->setVisible (connectionType==Flarm::serialConnection);
	ui.flarmSerialPortInput    ->setVisible (connectionType==Flarm::serialConnection);
	ui.flarmSerialBaudRateLabel->setVisible (connectionType==Flarm::serialConnection);
	ui.flarmSerialBaudRatePane ->setVisible (connectionType==Flarm::serialConnection);

	ui.flarmTcpHostLabel->setVisible (connectionType==Flarm::tcpConnection);
	ui.flarmTcpHostInput->setVisible (connectionType==Flarm::tcpConnection);
	ui.flarmTcpPortLabel->setVisible (connectionType==Flarm::tcpConnection);
	ui.flarmTcpPortPane ->setVisible (connectionType==Flarm::tcpConnection);

	ui.flarmFileNamePane  ->setVisible (connectionType==Flarm::fileConnection);
	ui.flarmFileNameLabel ->setVisible (connectionType==Flarm::fileConnection);
	ui.flarmFileDelayPane ->setVisible (connectionType==Flarm::fileConnection);
	ui.flarmFileDelayLabel->setVisible (connectionType==Flarm::fileConnection);

    ui.flarmRangePane->setVisible (ui.flarmActivateRangeCheckbox->isChecked());
    ui.flarmRangeLabel->setVisible (ui.flarmActivateRangeCheckbox->isChecked());
}


void SettingsWindow::on_flarmConnectionTypeInput_activated (int index)
{
	(void)index;
	updateWidgets ();
}

void SettingsWindow::on_flarmSerialPortInput_activated (int index)
{
	// An item was selected. This can either be one of the predefined items, or
	// a custom text.
	// A predefined item will have a text of "port name (description)". We don't
	// want the description part in the text field, just the port name. The port
	// name is also stored in the item data for the item. For a custom text, the
	// item data will be empty.

	// Retrieve the item data for the item that was activated.
	QString text=ui.flarmSerialPortInput->itemData (index).toString ();

	// If the item data is not empty (i. e. it is one of the predefined items),
	// set the text to the item data.
	if (!text.isEmpty ())
		ui.flarmSerialPortInput->setEditText (text);
}

void SettingsWindow::on_flarmActivateRangeCheckbox_toggled(bool checked)
{
    if (checked)
    {
        ui.flarmRangeInput->setValue(1000);
    }

    updateWidgets();
}
