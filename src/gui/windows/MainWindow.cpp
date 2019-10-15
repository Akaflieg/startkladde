/*
 * Improvements:
 *   - the log should also display queries from other database interfaces that
 *     may be created during connect, such as the root interface
 *   - when double-clicking in the empty area of the flight table, create a new
 *     flight
 *   - when double-clicking the displayed date label, change the displayed date
 *   - [non]interactive[Depart|Land] should probably accept a FlightReference
 *     instead of a flight ID, so we can get rid of
 *     [non]interactiveLandTowflight (really? Note that departing a towflight
 *     is the same as departing the flight).
 */
//	assert (isGuiThread ());

#include "MainWindow.h"

#include <iostream>

#include <QAction>
#include <QSettings>
#include <QEvent>
#include <QFontDialog>
#include <QTimer>
#include <QGridLayout>
#include <QProgressDialog>
#include <QInputDialog>
#include <QList>
#include <QStatusBar>
#include <QCloseEvent>
#include <QScrollBar>
//#include <phonon>
#include <QWidget> // remove

// TODO many dependencies - split
#include "src/concurrent/threadUtil.h"
#include "src/config/Settings.h"
#include "src/i18n/notr.h"
#include "src/gui/widgets/WeatherWidget.h"
#include "src/gui/windows/AboutDialog.h"
#include "src/gui/windows/input/DateInputDialog.h"
#include "src/gui/windows/input/DateTimeInputDialog.h"
#include "src/gui/windows/FlightWindow.h"
#include "src/gui/windows/FlightWizard.h"
#include "src/gui/windows/LaunchMethodSelectionWindow.h"
#include "src/gui/windows/objectList/ObjectListWindow.h"
#include "src/gui/windows/objectList/FlightListWindow.h"
#include "src/gui/windows/StatisticsWindow.h"
#include "src/gui/windows/WeatherDialog.h"
#include "src/gui/windows/TrainingsBarometerDialog.h"
#include "src/gui/windows/SettingsWindow.h"
#include "src/model/Plane.h"
#include "src/model/Flight.h"
#include "src/model/Person.h"
#include "src/model/objectList/AutomaticEntityList.h" // TODO remove some?
#include "src/model/objectList/EntityList.h"
#include "src/model/objectList/ObjectListModel.h"
#include "src/plugin/info/InfoPlugin.h"
#include "src/plugin/weather/WeatherPlugin.h"
#include "src/plugin/factory/PluginFactory.h"
#include "src/statistics/LaunchMethodStatistics.h"
#include "src/statistics/PilotLog.h"
#include "src/statistics/PlaneLog.h"
#include "src/flarm/FlarmList.h"
#include "src/nmea/GpsTracker.h"
#include "src/flarm/FlarmWindow.h"
#include "src/flarm/flarmNet/FlarmNetHandler.h"
#include "src/flarm/flarmNet/FlarmNetWindow.h"
#include "src/gui/dialogs.h"
#include "src/logging/messages.h"
#include "src/util/qString.h"
#include "src/util/qList.h"
#include "src/util/qDate.h"
#include "src/concurrent/monitor/OperationCanceledException.h"
#include "src/db/cache/Cache.h"
#include "src/text.h"
#include "src/i18n/TranslationManager.h"
#include "src/flarm/algorithms/PlaneLookup.h"
#include "src/flarm/algorithms/FlightLookup.h"
#include "src/flarm/algorithms/PlaneIdentification.h"
#include "src/flarm/algorithms/FlarmIdUpdate.h"
#include "src/gui/windows/input/ChoiceDialog.h"
#include "src/flarm/Flarm.h"
#include "src/flarm/flarmNet/FlarmNetHandler.h"
#include "SyncDialog.h"

template <class T> class MutableObjectList;

const int notificationDisplayTime=4000; // Milliseconds
const QTime ignoreDuplicateFlarmEventInterval (0, 0, 10);

// ******************
// ** Construction **
// ******************

MainWindow::MainWindow (QWidget *parent, DbManager &dbManager, Flarm &flarm):
	SkMainWindow<Ui::MainWindowClass> (parent),
	dbManager (dbManager),
	cache (dbManager.getCache ()),
	flarm (flarm),
	oldLogVisible (false),
	createFlightWindow (NULL), editFlightWindow (NULL),
	weatherWidget (NULL), weatherPlugin (NULL),
	weatherDialog (NULL),
	flightList (new EntityList<Flight> (this)),
	contextMenu (new QMenu (this)),
	databaseActionsEnabled (false),
	fontSet (false),
	debugFlarmId ("ABC")
{
	ui.setupUi (this);

	ui.flightTable->init (&dbManager);

	// ***** Flarm
	connect (
		&flarm, SIGNAL (streamStateChanged       (ManagedDataStream::State::Type)),
		this  , SLOT   (flarm_streamStateChanged (ManagedDataStream::State::Type)));
	connect (
		&flarm, SIGNAL (stateChanged       (Flarm::State::Type)),
		this  , SLOT   (flarm_stateChanged (Flarm::State::Type)));
	// This will cause a Flarm connection state change
	flarm.open ();
	ui.openFlarmAction->setChecked (flarm.isOpen ());

	// TODO this action should only be enabled if the Flarm connection type is
	// not "none"
	connect (ui.openFlarmAction, SIGNAL (toggled (bool)), &flarm, SLOT (setOpen (bool)));

	connect (flarm.flarmList (), SIGNAL (departureDetected  (const QString &)), this, SLOT (flarmList_departureDetected  (const QString &)));
	connect (flarm.flarmList (), SIGNAL (landingDetected    (const QString &)), this, SLOT (flarmList_landingDetected    (const QString &)));
	connect (flarm.flarmList (), SIGNAL (touchAndGoDetected (const QString &)), this, SLOT (flarmList_touchAndGoDetected (const QString &)));

	connect (ui.flarmStateLabel, SIGNAL (linkActivated (const QString &)), this, SLOT (flarmStreamLinkActivated (const QString &)));

	// TODO should not call any non-trivial methods from the constructor
	updateFlarmState ();

	// Menu bar
	logAction = ui.logDockWidget->toggleViewAction ();
	ui.menuDatabase->addSeparator ();
	ui.menuDatabase->addAction (logAction);

	connect (this, SIGNAL (minuteChanged ()), ui.flightTable, SLOT (minuteChanged ()));

	connect (ui.actionSort, SIGNAL (triggered ()), ui.flightTable, SLOT (setCustomSorting ()));

	connect (&Settings::instance (), SIGNAL (changed ()), this, SLOT (settingsChanged ()));
	readSettings ();
	// This also calls setupText
	settingsChanged ();

	setupLabels ();

	// Change the language every second so we can verify even for modal windows
	// that they are correctly retranslated.
	QTimer *timeTimer = new QTimer (this);
	connect (timeTimer, SIGNAL (timeout ()), this, SLOT (timeTimer_timeout ()));
	timeTimer->start (1000);

	timeTimer_timeout ();

	translationTimer=new QTimer (this);
	connect (translationTimer, SIGNAL (timeout ()), this, SLOT (on_changeLanguageAction_triggered ()));


	setupLayout ();

	// Do this before calling connect
	QObject::connect (&dbManager.getCache (), SIGNAL (changed (DbEvent)), this, SLOT (cacheChanged (DbEvent)));

	QObject::connect (&dbManager, SIGNAL (migrationStarted ()), this, SLOT (migrationStarted ()));
	QObject::connect (&dbManager, SIGNAL (migrationEnded   ()), this, SLOT (migrationEnded   ()));

	// TODO to showEvent?
	QTimer::singleShot (0, this, SLOT (on_actionConnect_triggered ()));

	ui.logDockWidget->setVisible (false);

	ui.actionShutdown->setVisible (Settings::instance ().enableShutdown);

#if defined (Q_OS_WIN32) || defined (Q_OS_WIN64)
	ui.actionSetTime->setVisible (false);
	ui.actionSetGPSTime->setVisible (false);
	bool virtualKeyboardEnabled=false;
#else
	bool virtualKeyboardEnabled = (
		system (notr ("which kvkbd >/dev/null")) == 0 &&
		system (notr ("which dbus-send >/dev/null")) == 0);
		//system ("which dcop >/dev/null") == 0);
#endif

	ui.actionShowVirtualKeyboard->setVisible (virtualKeyboardEnabled);
    ui.actionShowVirtualKeyboard->setIcon (QIcon (notr (":/graphics/keyboard.svg")));

	// Log
	ui.logWidget->document ()->setMaximumBlockCount (100);

	// Signals
	connect (flightList, SIGNAL (rowsInserted (const QModelIndex &, int, int)), this, SLOT (flightListChanged ()));
	connect (flightList, SIGNAL (rowsRemoved (const QModelIndex &, int, int)), this, SLOT (flightListChanged ()));
	connect (flightList, SIGNAL (dataChanged (const QModelIndex &, const QModelIndex &)), this, SLOT (flightListChanged ()));
	connect (flightList, SIGNAL (modelReset ()), this, SLOT (flightListChanged ()));

	// Flight table
	ui.flightTable->setAutoResizeRows (true);
	ui.flightTable->setColoredSelectionEnabled (true);
	ui.flightTable->setModel (flightList);
	ui.flightTable->resizeColumnsToContents (); // Default sizes

	readColumnWidths (); // Stored sizes

	connect (ui.actionHideFinished, SIGNAL (toggled (bool)), ui.flightTable, SLOT (setHideFinishedFlights (bool)));
	connect (ui.actionAlwaysShowExternal, SIGNAL (toggled (bool)), ui.flightTable, SLOT (setAlwaysShowExternalFlights (bool)));
	connect (ui.actionAlwaysShowErroneous, SIGNAL (toggled (bool)), ui.flightTable, SLOT (setAlwaysShowErroneousFlights (bool)));
	connect (ui.actionRefreshTable, SIGNAL (triggered ()), this, SLOT (refreshFlights ()));

	connect (ui.actionRefreshTable, SIGNAL (triggered ()), this, SLOT (refreshFlights ()));
	connect (ui.actionJumpToTow, SIGNAL (triggered ()), ui.flightTable, SLOT (interactiveJumpToTowflight ()));
	connect (ui.actionRestartPlugins, SIGNAL (triggered ()), this, SLOT (restartPlugins ()));

	// Initialize all properties of the filter proxy model
	ui.flightTable->setHideFinishedFlights (ui.actionHideFinished->isChecked ());
	ui.flightTable->setAlwaysShowExternalFlights (ui.actionAlwaysShowExternal->isChecked ());
	ui.flightTable->setAlwaysShowErroneousFlights (ui.actionAlwaysShowErroneous->isChecked ());
	ui.flightTable->setFocus ();

	setDisplayDateCurrent (true);


	// Database
	connect (&dbManager.getInterface (), SIGNAL (databaseError (int, QString)), this, SLOT (databaseError (int, QString)));
	connect (&dbManager.getInterface (), SIGNAL (executingQuery (Query)), this, SLOT (executingQuery (Query)));

	connect (&dbManager, SIGNAL (readTimeout ()), this, SLOT (readTimeout ()));
	connect (&dbManager, SIGNAL (readResumed ()), this, SLOT (readResumed ()));

	connect (&dbManager, SIGNAL (stateChanged (DbManager::State)), this, SLOT (databaseStateChanged (DbManager::State)));
	databaseStateChanged (dbManager.getState ());

}

MainWindow::~MainWindow ()
{
	// Hack: Hide the window to avoid trouble[tm].
	// If we don't make the window invisible here, it will be done in the
	// QWidget destructor. Then the flight table will access its model, which
	// will in turn access the cache, which has already been deleted.
	// In one case, this has been known to lead to a "QMutex::lock: mutex lock
	// failure", but worse things can happen.
	setVisible (false);
	// QObjects will be deleted automatically
	// TODO only if this is their parent
	// TODO make sure this also applies to flightList

	terminatePlugins ();
}

void MainWindow::setupLabels ()
{
	if (Settings::instance ().coloredLabels)
	{
		QObjectList labels = ui.infoPane->children ();

		foreach (QObject *object, labels)
		{
			SkLabel *label = dynamic_cast<SkLabel *> (object);
			if (label)
			{
				if (label->objectName ().contains (notr ("Caption"), Qt::CaseSensitive))
					label->setPaletteBackgroundColor (QColor (0, 255, 127));
				else
					label->setPaletteBackgroundColor (QColor (0, 127, 255));

				label->setAutoFillBackground (true);
			}
		}
	}
}

void MainWindow::setupLayout ()
{
	// QT 4.3.4 uic ignores the stretch factors (4.5.0 uses it), so set it here.

	QVBoxLayout *centralLayout = (QVBoxLayout *)centralWidget () -> layout ();
	centralLayout->setStretchFactor (ui.topPane, 0);
	centralLayout->setStretchFactor (ui.flightTable, 1);

	//	QHBoxLayout *topPaneLayout     = (QHBoxLayout *) ui.infoPane    -> layout ();
	QHBoxLayout *infoFrameLayout = (QHBoxLayout *)ui.infoFrame -> layout ();
	QGridLayout *infoPaneLayout = (QGridLayout *)ui.infoPane -> layout ();
	QGridLayout *pluginPaneLayout = (QGridLayout *)ui.pluginPane -> layout ();

	/*
	 * This setting gives the plugins more space while still leaving some non-
	 * minimum space between the info and the plugins.
	 * The best behavior would be:
	 *   - normally, info and plugins take up the same amount of space
	 *   - if the plugin values get too large, the info area is made smaller
	 *     in favor of the plugin area
	 *   - if the info area cannot shrink any more, but the plugin area is
	 *     still to small the plugin values are wrapped
	 *   - the plugin values are not wrapped as long as space can be freed by
	 *     shrinking the info area
	 *   - never is the window resized to make space for the plugins if
	 *     something else is possible
	 *   - small changes in the info pane width do not lead to the plugin area
	 *     being moved around
	 */

	// The plugin pane gets twice the space of the info pane
	infoFrameLayout->setStretchFactor (ui.infoPane, 1);
	infoFrameLayout->setStretchFactor (ui.pluginPane, 2);

	// For both the info pane and the plugin pane: all available space goes to
	// the value; the caption is kept at minimum size.
	infoPaneLayout->setColumnStretch (0, 0);
	infoPaneLayout->setColumnStretch (1, 1);
	pluginPaneLayout->setColumnStretch (0, 0);
	pluginPaneLayout->setColumnStretch (1, 1);
}

void MainWindow::setupPlugin (InfoPlugin *plugin, QGridLayout *pluginLayout)
{
	connect (this, SIGNAL (minuteChanged ()), plugin, SLOT (minuteChanged ()));

	SkLabel *captionLabel = new SkLabel ("", ui.pluginPane);
	SkLabel *valueLabel = new SkLabel (notr ("..."), ui.pluginPane);

	captionLabel->setText (plugin->getCaption ());

	valueLabel->setWordWrap (true);
	QString toolTip=tr ("%1\nConfiguration: %2").arg (plugin->getDescription (), plugin->configText ());
	valueLabel->setToolTip (toolTip);
	captionLabel->setToolTip (toolTip);

	int row = pluginLayout->rowCount ();
	pluginLayout->addWidget (captionLabel, row, 0, Qt::AlignTop);
	pluginLayout->addWidget (valueLabel, row, 1, Qt::AlignTop);

	if (Settings::instance ().coloredLabels)
	{
		captionLabel->setPaletteBackgroundColor (QColor (255, 63, 127));
		valueLabel ->setPaletteBackgroundColor (QColor (255, 255, 127));

		captionLabel->setAutoFillBackground (true);
		valueLabel ->setAutoFillBackground (true);
	}

	connect (plugin, SIGNAL (textOutput (QString, Qt::TextFormat)), valueLabel, SLOT (setText (QString, Qt::TextFormat)));

	QObject::connect (captionLabel, SIGNAL (doubleClicked (QMouseEvent *)), plugin, SLOT (restart ()));
	QObject::connect (valueLabel, SIGNAL (doubleClicked (QMouseEvent *)), plugin, SLOT (restart ()));

	plugin->start ();
}

void MainWindow::setupPlugins ()
{
	Settings &s=Settings::instance ();

	// First, terminate the plugins to make sure they won't access the labels
	// any more.
	terminatePlugins ();

	// Remove the old labels from the plugin pane
	foreach (QObject *child, ui.pluginPane->children ())
		delete child;

	// Delete the old layout manager and create a new one, or the layout
	// will be wrong: pluginLayout->rowCount will continue to grow, even
	// though pluginLayout->count will return the correct value.
	delete ui.pluginPane->layout ();
	QGridLayout *pluginLayout=new QGridLayout (ui.pluginPane);
	pluginLayout->setMargin (4);
	pluginLayout->setVerticalSpacing (4);

	deleteList (infoPlugins);

	infoPlugins=s.readInfoPlugins ();

	ui.pluginPane->setVisible (!infoPlugins.isEmpty ());

	foreach (InfoPlugin *plugin, infoPlugins)
		if (plugin->isEnabled ())
			setupPlugin (plugin, pluginLayout);

	pluginLayout->setColumnStretch (0, 0);
	pluginLayout->setColumnStretch (1, 1);
	pluginLayout->setRowStretch (pluginLayout->rowCount (), 1);



	delete weatherWidget;
	weatherWidget=NULL;


	weatherPlugin=NULL; // Deleted in terminatePlugins
	if (s.weatherPluginEnabled && !isBlank (s.weatherPluginId))
		weatherPlugin=PluginFactory::getInstance ().createWeatherPlugin (s.weatherPluginId, s.weatherPluginCommand);

	ui.weatherFrame->setVisible (weatherPlugin!=NULL);
	if (weatherPlugin)
	{
		// Create and setup the weather widget. The weather widget is located to
		// the right of the info frame.
		weatherWidget = new WeatherWidget (ui.weatherFrame);
		ui.weatherFrame->layout ()->addWidget (weatherWidget);
		weatherWidget->setFixedSize (s.weatherPluginHeight, s.weatherPluginHeight);
		weatherWidget->setText (tr ("Weather"));

		weatherPlugin->enableRefresh (s.weatherPluginInterval);
		connect (weatherPlugin, SIGNAL (textOutput (const QString &, Qt::TextFormat)), weatherWidget, SLOT (setText (const QString &, Qt::TextFormat)));
		connect (weatherPlugin, SIGNAL (imageOutput (const QImage &)), weatherWidget, SLOT (setImage (const QImage &)));
		connect (weatherPlugin, SIGNAL (movieOutput (SkMovie &)), weatherWidget, SLOT (setMovie (SkMovie &)));
		connect (weatherWidget, SIGNAL (doubleClicked ()), this, SLOT (weatherWidget_doubleClicked ()));
		weatherPlugin->start ();
	}

}

void MainWindow::terminatePlugins ()
{
	foreach (InfoPlugin *plugin, infoPlugins)
	{
		//std::cout << "Terminating plugin " << plugin->get_caption () << std::endl;
		plugin->terminate ();
		QThread::yieldCurrentThread ();
	}

	while (!infoPlugins.empty ())
		delete infoPlugins.takeLast ();

	if (weatherPlugin)
	{
		weatherPlugin->terminate ();
		delete weatherPlugin;
		weatherPlugin=NULL;
	}
}

void MainWindow::restartPlugins ()
{
	if (weatherPlugin) weatherPlugin->restart ();
	if (weatherDialog) weatherDialog->restartPlugin ();

	foreach (InfoPlugin *plugin, infoPlugins)
	{
		plugin->terminate ();
		plugin->start ();
	}
}


// *************
// ** Closing **
// *************

bool MainWindow::confirmAndExit (int returnCode, QString title, QString text)
{
	(void)title;
	(void)text;
//	if (yesNoQuestion (this, title, text))
//	{
		closeDatabase ();
		writeSettings ();
		qApp->exit (returnCode);
		return true;
//	}
//	else
//	{
//		return false;
//	}
}

void MainWindow::closeEvent (QCloseEvent *event)
{
	if (!confirmAndExit (0, tr ("Really exit?"), tr ("Really exit the program?")))
		event->ignore ();
}

void MainWindow::on_actionQuit_triggered ()
{
	confirmAndExit (0, tr ("Really exit?"), tr ("Really exit the program?"));
}

void MainWindow::on_actionShutdown_triggered ()
{
	confirmAndExit (69, tr ("Really shut down?"), tr ("Really shut down the computer?"));
}

// **************
// ** Settings **
// **************

void MainWindow::writeSettings ()
{
	QSettings settings;

	settings.beginGroup (notr ("gui"));

	if (fontSet)
	{
		settings.beginGroup (notr ("fonts"));
		QFont font = QApplication::font ();
		settings.setValue (notr ("font"), font.toString ());
		settings.endGroup ();
	}

	settings.beginGroup (notr ("flightTable"));
	ui.flightTable->writeColumnWidths (settings);
	settings.endGroup ();

	settings.endGroup ();
}

void MainWindow::readColumnWidths ()
{
	QSettings settings;

	settings.beginGroup (notr ("gui"));
    settings.beginGroup (notr ("flightTable"));
    ui.flightTable->readColumnWidths (settings);
    settings.endGroup ();
    settings.endGroup ();
}

void MainWindow::readSettings ()
{
	QSettings settings;

	settings.beginGroup (notr ("gui"));
	settings.beginGroup (notr ("fonts"));

	if (settings.contains (notr ("font")))
	{
		QString fontDescription = settings.value (notr ("font")).toString ();
		QFont font;
		if (font.fromString (fontDescription))
		{
			QApplication::setFont (font);
			fontSet=true;
		}
	}

	settings.endGroup ();
	settings.endGroup ();

}

/** Setup translated texts */
void MainWindow::setupText ()
{
	Settings &s=Settings::instance ();

	logAction->setText (tr ("Show &log"));

	if (isBlank (s.location))
		setWindowTitle (tr ("Startkladde"));
	else
		setWindowTitle (tr ("Flight log %1 - Startkladde").arg (s.location));

}

void MainWindow::settingsChanged ()
{
	Settings &s=Settings::instance ();

	setupText ();

	ui.menuDebug->menuAction ()->setVisible (s.enableDebug);
	// Even though the menu is invisible, we still need to disable the menu
	// entries to disable their shortcuts.
	ui.changeLanguageAction          ->setEnabled (s.enableDebug);
	ui.timerBasedLanguageChangeAction->setEnabled (s.enableDebug);

	ui.actionNetworkDiagnostics     ->setVisible (!isBlank (s.diagCommand));

    // Vereinsflieger
    ui.actionSync->setVisible(s.vfUploadEnabled);
	
	// Flarm
	// Note that we enable the flarmPlaneList and flarmRadar actions even if
	// s.flarmDataViewable is off, so we can show a message to the user as to
	// why it's disabled.
	ui.actionFlarmPlaneList  ->setEnabled (s.flarmEnabled);
	ui.actionFlarmRadar	     ->setEnabled (s.flarmEnabled);
	ui.flarmStateCaptionLabel->setEnabled (s.flarmEnabled);
	ui.flarmStateLabel       ->setEnabled (s.flarmEnabled);
	// Also disabled the FlarmNet menu entries to indicate to the user that
	// FlarmNet is not used.
	ui.actionFlarmNetWindow    ->setEnabled (s.flarmNetEnabled);
	ui.flarmNetImportFileAction->setEnabled (s.flarmNetEnabled);
	ui.flarmNetImportWebAction ->setEnabled (s.flarmNetEnabled);

	// Plugins
	setupPlugins ();
}

// *************
// ** Flights **
// *************

void MainWindow::updateDisplayDateLabel (const QDate &today)
{
	if (!databaseActionsEnabled)
	{
		ui.displayDateLabel->resetDefaultForegroundColor ();
		ui.displayDateLabel->setText (notr ("-"));
	}
	else if (displayDate==today)
	{
		ui.displayDateLabel->resetDefaultForegroundColor ();
		ui.displayDateLabel->setText (tr ("Today (%1)").arg (today.toString (defaultNumericDateFormat ())));
	}
	else
	{
		ui.displayDateLabel->setPaletteForegroundColor (Qt::red);
		ui.displayDateLabel->setText (dbManager.getCache ().getOtherDate ().toString (tr ("dddd, M/d/yyyy")));
	}
}

/**
 * Refreshes both the flight table and the corresponding info labels, from the
 * cache, including the displayed date label (text and color). Does not access
 * the database.
 */
void MainWindow::refreshFlights ()
{
	// Fetch the current date to avoid it changing during the operation
	// TODO time zone safety: should be local today
	QDate today=QDate::currentDate ();

	QList<Flight> flights;
	if (databaseActionsEnabled)
	{
		if (displayDate==today)
		{
			// The displayed date is today's date - display today's flights and
			// prepared flights
			flights  = dbManager.getCache ().getFlightsToday    (false).getList ();
			flights += dbManager.getCache ().getPreparedFlights (false).getList ();

			ui.flightTable->setShowPreparedFlights (true);
		}
		else
		{
			// The displayed date is not today's date - display the flights from the
			// cache's "other" date
			flights=dbManager.getCache ().getFlightsOther ().getList ();

			ui.flightTable->setShowPreparedFlights (false);
		}
	}

	updateDisplayDateLabel (today);

	FlightReference selectedFlight=ui.flightTable->selectedFlightReference ();
	int column=ui.flightTable->currentIndex ().column ();

	flightList->replaceList (flights);
	ui.flightTable->setCustomSorting ();

	ui.flightTable->selectFlight (selectedFlight, column);

	// TODO should be done automatically
	// ui.flightTable->resizeColumnsToContents ();
	// ui.flightTable->resizeRowsToContents ();

	// TODO: set the cursor to last row, same column as before (this is
	// usually called after a date change, so the previous row is
	// meaningless)
	//int oldColumn = ui.flightTable->currentColumn ();
	//int newRow = ui.flightTable->rowCount () - 1;
	//ui.flightTable->setCurrentCell (newRow, oldColumn);
	// TODO make sure it's visible

	// TODO
	//updateInfo ();
}

/**
 * Called when the flight list changes (flights are inserted or removed or data
 * changes). Updates the info labels.
 */
void MainWindow::flightListChanged ()
{
	QList<Flight> flights=flightList->getList ();

	// Note that there is a race condition if "refresh" is called before
	// midnight and this function is called after midnight.
	// TODO store the todayness somewhere instead.

	if (databaseActionsEnabled && displayDate == QDate::currentDate ())
		ui.activeFlightsLabel->setNumber (Flight::countFlying (flights));
	else
		ui.activeFlightsLabel->setText (notr ("-"));

	if (databaseActionsEnabled)
		ui.totalFlightsLabel->setNumber (Flight::countHappened (flights));
	else
		ui.totalFlightsLabel->setText (notr ("-"));
}

// *************************
// ** Flight manipulation **
// *************************

/*
 * Notes:
 *   - for create, repeat and edit: the flight editor may be modeless
 *     and control may return immediately (even for modal dialogs).
 *     The table entry will be updated when the flight editor updates the
 *     database.
 *
 * TODO:
 *   - hier den ganzen Kram wie displayDate==heute und flug schon
 *     gelandet prüfen, damit man die Menüdeaktivierung weglassen kann.
 *     Außerdem kann man dann hier melden, warum das nicht geht.
 *
 */

bool MainWindow::updateFlight (const Flight &flight)
{
	try
	{
		dbManager.updateObject (flight, this);
		return true;
	}
	catch (OperationCanceledException &e)
	{
		// TODO the cache may now be inconsistent

		return false;
	}

	return false;
}

bool MainWindow::checkPlaneFlying (dbId id, const QString &description)
{
	if (idValid (id) && cache.planeFlying (id))
	{
		Plane plane=cache.getObject<Plane> (id);
		QString text=tr ("According to the database, the %1 %2 is still flying. Depart anyway?")
				.arg (description, plane.registration);
		if (!yesNoQuestion (this, tr ("Plane still flying"), text))
			return false;
	}

	return true;
}

bool MainWindow::checkPersonFlying (dbId id, const QString &description)
{
	if (idValid (id) && cache.personFlying (id))
	{
		Person person=cache.getObject<Person> (id);
		QString text=tr ("According to the database, the %1 %2 is still flying. Start anyway?")
				.arg (description, person.fullName ());
		if (!yesNoQuestion (this, tr ("Person still flying"), text)) return false;
	}

	return true;
}

void MainWindow::interactiveDepartFlight (dbId id)
{
	// TODO display message
	if (idInvalid (id)) return;

	try
	{
		Flight flight=dbManager.getCache ().getObject<Flight> (id);
		QString reason;

		if (flight.canDepart (&reason))
		{
			bool isAirtow=flight.isAirtow (cache);

			// *** Check for planes flying
			// Plane
			if (!checkPlaneFlying (flight.getPlaneId (), tr ("plane"))) return;
			if (isAirtow)
				if (!checkPlaneFlying (flight.effectiveTowplaneId (cache), tr ("towplane"))) return;

			// *** Check for people flying
			// Pilot
			if (!checkPersonFlying (flight.getPilotId (), flight.pilotDescription ())) return;
			// Copilot (if recorded for this flight)
			if (flight.copilotRecorded ())
				if (!checkPersonFlying (flight.getCopilotId (), flight.copilotDescription ())) return;
			// Towpilot (if airtow)
			if (isAirtow && Settings::instance ().recordTowpilot)
				if (!checkPersonFlying (flight.getTowpilotId (), flight.towpilotDescription ())) return;

			flight.departNow ();
			if (updateFlight (flight))
				flightDeparted (id);
		}
		else
		{
			showWarning (tr ("Departing not possible"), reason, this);
		}
	}
	catch (Cache::NotFoundException &ex)
	{
		log_error (qnotr ("Flight %1 not found in MainWindow::departFlight").arg (ex.id));
	}
}

void MainWindow::interactiveLandFlight (dbId id)
{
	// TODO display message
	if (idInvalid (id)) return;

	try
	{
		Flight flight = dbManager.getCache ().getObject<Flight> (id);
		QString reason;

		if (flight.canLand (&reason))
		{
			flight.landNow ();
			if (updateFlight (flight))
				flightLanded (id);
		}
		else
		{
			showWarning (tr ("Landing not possible"), reason, this);
		}
	}
	catch (Cache::NotFoundException &ex)
	{
		log_error (qnotr ("Flight not found in MainWindow::landFlight").arg (ex.id));
	}
}

void MainWindow::interactiveTouchAndGo (dbId id)
{
	// TODO display message
	if (idInvalid (id)) return;

	try
	{
		// TODO warning if the plane is specified and a glider and the
		// launch method is specified and not an unended airtow

		Flight flight = dbManager.getCache ().getObject<Flight> (id);
		QString reason;

		if (flight.canTouchngo (&reason))
		{
			flight.performTouchngo ();
			if (updateFlight (flight))
				touchAndGoPerformed (id);
		}
		else
		{
			showWarning (tr ("Touch-and-go not possible"), reason, this);
		}
	}
	catch (Cache::NotFoundException &ex)
	{
		log_error (qnotr ("Flight %1 not found in MainWindow::on_actionTouchngo_triggered").arg (ex.id));
	}
}

void MainWindow::interactiveLandTowflight (dbId id)
{
	// TODO display message
	if (idInvalid (id)) return;

	try
	{
		Flight flight = dbManager.getCache ().getObject<Flight> (id);
		QString reason;

		if (flight.canTowflightLand (&reason))
		{
			flight.landTowflightNow ();
			if (updateFlight (flight))
				towflightLanded (id);
		}
		else
		{
			showWarning (tr ("Landing not possible"), reason, this);
		}
	}
	catch (Cache::NotFoundException &ex)
	{
		log_error (qnotr ("Flight %1 not found in MainWindow::landTowFlight").arg (ex.id));
	}
}

// TODO for all nonInteractiveXxx methods, there should be a way to report
// errors (departure not possible...) or failed plausibility checks (person
// still flying...)
bool MainWindow::nonInteractiveDepartFlight (dbId flightId)
{
	try
	{
		Flight flight = dbManager.getCache ().getObject<Flight> (flightId);
		QString reason;
		if (flight.canDepart (&reason))
		{
			flight.departNow ();
			if (updateFlight (flight))
				flightDeparted (flightId);
			return true;
		}
		else
		{
			std::cout << qnotr ("Departure not possible: %1").arg (reason) << std::endl;
			return false;
		}
	}
	catch (Cache::NotFoundException &ex)
	{
		log_error (qnotr ("Flight %1 not found in MainWindow::nonInteractiveDepartFlight").arg (ex.id));
	}

	return false;
}

bool MainWindow::nonInteractiveLandFlight (dbId flightId)
{
	try
	{
		Flight flight = dbManager.getCache ().getObject<Flight> (flightId);
		QString reason;
		if (flight.canLand (&reason))
		{
			flight.landNow ();
			if (updateFlight (flight))
			{
				flightLanded (flightId);
				return true;
			}
			else
			{
				return false;
			}
		}
		else
		{
			std::cout << qnotr ("Landing not possible: %1").arg (reason) << std::endl;
			return false;
		}
	}
	catch (Cache::NotFoundException &ex)
	{
		log_error (qnotr ("Flight %1 not found in MainWindow::nonInteractiveLandFlight").arg (ex.id));
	}

	return false;
}

// Note that we can't call nonInteractiveLandFlight on the towflight
bool MainWindow::nonInteractiveLandTowflight (dbId flightId)
{
	try
	{
		Flight flight = dbManager.getCache ().getObject<Flight> (flightId);
		QString reason;
		if (flight.canTowflightLand (&reason))
		{
			flight.landTowflightNow ();
			if (updateFlight (flight))
			{
				towflightLanded (flightId);
				return true;
			}
			else
			{
				return false;
			}
		}
		else
		{
			std::cout << qnotr ("Landing towflight not possible: %1").arg (reason) << std::endl;
			return false;
		}
	}
	catch (Cache::NotFoundException &ex)
	{
		log_error (qnotr ("Flight %1 not found in MainWindow::nonInteractiveLandTowflight").arg (ex.id));
	}

	return false;
}

bool MainWindow::nonInteractiveTouchAndGo (dbId flightId)
{
	try
	{
		Flight flight = dbManager.getCache ().getObject<Flight> (flightId);
		QString reason;
		if (flight.canTouchngo (&reason))
		{
			flight.performTouchngo ();
			if (updateFlight (flight))
			{
				touchAndGoPerformed (flightId);
				return true;
			}
			else
			{
				return false;
			}
		}
		else
		{
			std::cout << qnotr ("Touch and go not possible: ") << reason << std::endl;
			return false;
		}
	}
	catch (Cache::NotFoundException &ex)
	{
		log_error (qnotr ("Flight %1 not found in MainWindow::nonInteractiveTouchAndGo").arg (ex.id));
	}

	return false;
}

void MainWindow::on_actionNew_triggered ()
{
	// Delete the old flight window (if it exists)
    createFlightWindow=FlightWindow::createFlight (this, dbManager, getNewFlightDate (), Settings::instance().preselectedLaunchMethod);

	connect (createFlightWindow, SIGNAL (flightDeparted (dbId)), this, SLOT (flightDeparted (dbId)));
	connect (createFlightWindow, SIGNAL (flightLanded   (dbId)), this, SLOT (flightLanded   (dbId)));
	createFlightWindow->setAttribute (Qt::WA_DeleteOnClose, true);

}

void MainWindow::on_actionNewWizard_triggered ()
{
    createFlightWizard = new FlightWizard(this, dbManager);
    createFlightWizard->setAttribute(Qt::WA_DeleteOnClose, true);
    createFlightWizard->exec();
}

void MainWindow::on_actionLaunchMethodPreselection_triggered ()
{
    Settings &settings = Settings::instance();
    LaunchMethodSelectionWindow::select (cache, settings.preselectedLaunchMethod, settings.loadPreselectedLM);
    settings.save();
}

void MainWindow::on_actionDepart_triggered ()
{
	// This will check canDepart
	interactiveDepartFlight (ui.flightTable->selectedFlightReference ().id ());
}

void MainWindow::on_actionLand_triggered ()
{
	FlightReference flight=ui.flightTable->selectedFlightReference ();

	if (flight.towflight ())
		// This will check canLand
		interactiveLandTowflight (flight.id ());
	else
		// This will check canLand
		interactiveLandFlight (flight.id ());
}

void MainWindow::on_actionTouchngo_triggered ()
{
	FlightReference flight=ui.flightTable->selectedFlightReference ();

	if (flight.towflight ())
	{
		showWarning (
			tr ("Touch-and-go not possible"),
			tr ("The selected flight is a towflight. Towflights cannot perform a touch-and-go."),
			this);
	}
	else
		interactiveTouchAndGo (flight.id ());
}

void MainWindow::departOrLand ()
{
	FlightReference flightRef=ui.flightTable->selectedFlightReference ();

	if (!flightRef.isValid ())
		return;

	try
	{
		Flight flight = dbManager.getCache ().getObject<Flight> (flightRef.id ());

		if (flight.canDepart ())
		{
			flight.departNow ();
			if (updateFlight (flight))
				flightDeparted (flightRef.id ());
		}
		else if (flightRef.towflight () && flight.canTowflightLand ())
		{
			flight.landTowflightNow ();
			if (updateFlight (flight))
				towflightLanded (flightRef.id ());
		}
		else if (!flightRef.towflight () && flight.canLand ())
		{
			flight.landNow ();
			if (updateFlight (flight))
				flightLanded (flightRef.id ());
		}

	}
	catch (Cache::NotFoundException &ex)
	{
		log_error (qnotr ("Flight %1 not found in MainWindow::departOrLand").arg (ex.id));
	}
}

void MainWindow::on_actionEdit_triggered ()
{
	FlightReference flightRef=ui.flightTable->selectedFlightReference ();

	if (!flightRef.isValid ())
		return;

	try
	{
		Flight flight = dbManager.getCache ().getObject<Flight> (flightRef.id ());

		if (editFlightWindow && editFlightWindow->getEditedId ()==flightRef.id ())
		{
			// The flight is already being edited

			// How to raise a QDialog?
			// How to move to center? editFlightWindow->move does not seem to
			// have any effect (regardless of what - show or move - is done
			// first). Currently, it's done in FlightWindow#showEvent which
			// causes it to be shown in the top-left position first and then
			// moved.
			editFlightWindow->hide ();
			editFlightWindow->show ();
		}
		else
		{
			// Another flight may be being edited
			delete editFlightWindow; // noop if NULL
			editFlightWindow=FlightWindow::editFlight (this, dbManager, flight);
			editFlightWindow->setAttribute (Qt::WA_DeleteOnClose, true);
		}
	}
	catch (Cache::NotFoundException &ex)
	{
		log_error (qnotr ("Flight not found in MainWindow::on_actionEdit_triggered").arg (ex.id));
	}
}

void MainWindow::on_actionRepeat_triggered ()
{
	FlightReference flightRef=ui.flightTable->selectedFlightReference ();

	// TODO display message
	if (!flightRef.isValid ())
		return;

	else if (flightRef.towflight ())
	{
		showWarning (tr ("Replicating not possible"),
			tr ("The selected flight is a towflight. Towflights cannot be replicated."),
			this);
		return;
	}

	try
	{
		Flight flight = dbManager.getCache ().getObject<Flight> (flightRef.id ());
		delete createFlightWindow; // noop if NULL
        createFlightWindow=FlightWindow::repeatFlight (this, dbManager, flight, getNewFlightDate (), Settings::instance().preselectedLaunchMethod);
		createFlightWindow->setAttribute (Qt::WA_DeleteOnClose, true);
	}
	catch (Cache::NotFoundException &ex)
	{
		log_error (qnotr ("Flight %1 not found in MainWindow::on_actionRepeat_triggered").arg (flightRef.id ()));
	}
}

void MainWindow::on_actionDelete_triggered ()
{
	FlightReference flight=ui.flightTable->selectedFlightReference ();

	if (!flight.isValid ())
		// TODO display message
		return;

	if (!yesNoQuestion (this, tr ("Delete flight?"), tr ("Really delete flight?"))) return;

	if (flight.towflight ())
		if (!yesNoQuestion (this, tr ("Delete glider flight?"),
			tr ("The selected flight is a towflight. Really delete the corresponding glider flight?")))
			return;

	try
	{
		// Get the current index
		QModelIndex previousIndex=ui.flightTable->currentIndex ();
		dbManager.deleteObject<Flight> (flight.id (), this);
		ui.flightTable->setCurrentIndex (previousIndex); // Handles deletion of last item correctly
	}
	catch (OperationCanceledException &)
	{
		// TODO the cache may now be inconsistent
	}
}

void MainWindow::on_identifyPlaneAction_triggered ()
{
	try
	{
		// Retrieve the flight from the database
		FlightReference flightRef=ui.flightTable->selectedFlightReference ();
		if (!flightRef.isValid ())
			return;

		Flight flight=cache.getObject<Flight> (flightRef.id ());

		// Try to identify the plane, letting the user choose or create the
		// plane if necessary.
		PlaneIdentification planeIdentification (dbManager, this);
		dbId newPlaneId=planeIdentification.interactiveIdentifyPlane (flight, true);

		// If a plane was found and is different, update the flight
		if (idValid (newPlaneId) && newPlaneId!=flight.getPlaneId ())
		{
			flight.setPlaneId (newPlaneId);
			updateFlight (flight);
		}
	}
	catch (Cache::NotFoundException &ex)
	{
	}
}

void MainWindow::on_updateFlarmIdAction_triggered ()
{
	try
	{
		FlightReference flightRef=ui.flightTable->selectedFlightReference ();
		if (!flightRef.isValid ())
			return;

		if (flightRef.towflight ())
		{
			showWarning ("Cannot update Flarm ID", "The selected flight is a "
				"towflight. The Flarm ID cannot be updated for towflights.", this);
			return;
		}

		Flight flight=dbManager.getCache ().getObject<Flight> (flightRef.id ());

		FlarmIdUpdate flarmIdUpdate (dbManager, this);
		flarmIdUpdate.interactiveUpdateFlarmId (flight, true, flight.getPlaneId ());
	}
	catch (Cache::NotFoundException &)
	{
	}
}

void MainWindow::on_actionDisplayError_triggered ()
{
	// Note: only the first error is displayed

	// TODO: this method is quite complex and duplicates code found
	// elsewhere - the towplane generation should be simplified

	FlightReference flightRef=ui.flightTable->selectedFlightReference ();

	if (!flightRef.isValid ())
	{
		showWarning (tr ("No flight selected"), tr ("No flight is selected."), this);
		return;
	}

	try
	{
		Flight flight = dbManager.getCache ().getObject<Flight> (flightRef.id ());

		Plane *plane=dbManager.getCache ().getNewObject<Plane> (flight.getPlaneId ());
		LaunchMethod *launchMethod=dbManager.getCache ().getNewObject<LaunchMethod> (flight.getLaunchMethodId ());
		Plane *towplane=NULL;

		dbId towplaneId=invalidId;
		if (launchMethod && launchMethod->isAirtow ())
		{
			if (launchMethod->towplaneKnown ())
				towplaneId=dbManager.getCache ().getPlaneIdByRegistration (launchMethod->towplaneRegistration);
			else
				towplaneId=flight.getTowplaneId ();

			if (idValid (towplaneId))
				towplane=dbManager.getCache ().getNewObject<Plane> (towplaneId);
		}

		if (flightRef.towflight ())
		{
			dbId towLaunchMethod=dbManager.getCache ().getLaunchMethodByType (LaunchMethod::typeSelf);

			// TODO should use Flight::makeTowflight (cache), probably won't
			// need the stuff above
			flight=flight.makeTowflight (towplaneId, towLaunchMethod);

			delete launchMethod;
			launchMethod=dbManager.getCache ().getNewObject<LaunchMethod> (towLaunchMethod);

			delete plane;
			plane=towplane;
			towplane=NULL;
		}

		QString errorText;
		bool error=flight.isErroneous (cache, &errorText);

		delete plane;
		delete towplane;
		delete launchMethod;

		if (error)
		{
			if (flightRef.towflight ())
				showWarning (tr ("Towflight has errors"), tr ("First error of the towflight: %1").arg (errorText), this);
			else
				showWarning (tr ("Flight has errors"), tr ("First error of the flight: %1").arg (errorText), this);
		}
		else
		{
			if (flightRef.towflight ())
				showWarning (tr ("Towflight has no errors"), tr ("The towflight has no errors."), this);
			else
				showWarning (tr ("Flight has no errors"), tr ("The flight has no errors."), this);
		}
	}
	catch (Cache::NotFoundException &ex)
	{
		log_error (qnotr ("Flight %1 not found in MainWindow::on_actionDisplayError_triggered").arg (ex.id));
	}
}


// **********
// ** Font **
// **********

void MainWindow::on_actionSelectFont_triggered ()
{
	bool ok;
	QFont font = QApplication::font ();
	font = QFontDialog::getFont (&ok, font, this);

	if (ok)
	{
		// The user pressed OK and font is set to the font the user selected
		QApplication::setFont (font);
		fontSet=true;
	}
}

void MainWindow::on_actionIncrementFontSize_triggered ()
{
	QFont font = QApplication::font ();
	int size = font.pointSize ();
	font.setPointSize (size + 1);
	QApplication::setFont (font);
	fontSet=true;
}

void MainWindow::on_actionDecrementFontSize_triggered ()
{
	QFont font = QApplication::font ();
	int size = font.pointSize ();
	if (size>5)
	{
		font.setPointSize (size - 1);
		QApplication::setFont (font);
		fontSet=true;
	}
}

// **********
// ** View **
// **********

void MainWindow::on_actionShowVirtualKeyboard_triggered (bool checked)
{
	if (checked)
	{
		// This call may fail (when the progran is not running), don't display
		// stderr. If it fails, it will be run again.
		// Note that without --print-reply, it doesn't seem to work
		//int result = system ("dcop kvkbd kvkbd show >/dev/null 2>/dev/null");
		int result=system (notr ("dbus-send --print-reply --dest='org.kde.kvkbd' /Kvkbd org.freedesktop.DBus.Properties.Set string:org.kde.kvkbd.Kvkbd string:visible variant:boolean:true >/dev/null"));
		if (result != 0)
		{
			// failed to show; try launch
			if (system (notr ("kvkbd >/dev/null"))==0)
			{
				//system ("dcop kvkbd kvkbd show >/dev/null");
				if (system (notr ("dbus-send --print-reply --dest='org.kde.kvkbd' /Kvkbd org.freedesktop.DBus.Properties.Set string:org.kde.kvkbd.Kvkbd string:visible variant:boolean:true  >/dev/null"))!=0)
					showWarning (tr ("DBus call failed"),
						tr ("The call to dbus-send for displaying the virtual keyboard failed."),
						this);
			}
			else
			{
				showWarning (tr ("Unable to display virtual keyboard"),
					tr ("The virtual keyboard could not be displayed. Maybe kvkbd is not installed."),
					this);
			}

		}
	}
	else
	{
		//system ("/usr/bin/dcop kvkbd kvkbd hide >/dev/null");
		if (system (notr ("dbus-send --print-reply --dest='org.kde.kvkbd' /Kvkbd org.freedesktop.DBus.Properties.Set string:org.kde.kvkbd.Kvkbd string:visible variant:boolean:false >/dev/null"))!=0)
			showWarning (tr ("DBus call failed"),
				tr ("The call to dbus-send for hiding the virtual keyboard failed."),
				this);

	}
}

void MainWindow::on_actionRefreshAll_triggered ()
{
	try
	{
		dbManager.refreshCache (this);
	}
	catch (OperationCanceledException &ex) {}

	refreshFlights ();
}

void MainWindow::on_actionSync_triggered() {
    LoginDialog* loginDialog = new LoginDialog(this);
    loginDialog->exec();

    if (loginDialog->result() == QDialog::Accepted)
    {
        QString user = loginDialog->getUsername();
        QString pass = loginDialog->getPassword();
        delete loginDialog;

        SyncDialog* syncDialog = new SyncDialog(this);
        syncDialog->setCancelable(true);

        VereinsfliegerSyncWorker* worker = new VereinsfliegerSyncWorker(&dbManager, user, pass, this);
        syncDialog->open();

        connect(worker, SIGNAL(finished(bool,QString,QList<QTreeWidgetItem*>)), syncDialog, SLOT(completed(bool,QString,QList<QTreeWidgetItem*>)));
        connect(worker, SIGNAL(progress(int,QString)), syncDialog, SLOT(setProgress(int,QString)));
        connect(syncDialog, SIGNAL(cancelled()), worker, SLOT(cancel()));

        worker->sync();
        worker->deleteLater();
    }
}

// **********
// ** Help **
// **********

void MainWindow::on_actionInfo_triggered ()
{
	AboutDialog aboutDialog (this);
	aboutDialog.setModal (true);
	aboutDialog.exec ();
}

void MainWindow::on_actionNetworkDiagnostics_triggered ()
{
	QString command=Settings::instance ().diagCommand;
	if (isBlank (command)) return;

	// TODO: use QProcess and make sure it's in the background
	if (system (command.toUtf8 ().constData ())!=0)
		showWarning (tr ("Error"),
			tr ("An error occured while executing the network diagnostics command."), this);
}

// ************
// ** Events **
// ************

void MainWindow::keyPressEvent (QKeyEvent *e)
{
//	std::cout << "key " << e->key () << "/" << e->modifiers () << " pressed in MainWindow" << std::endl;

	switch (e->key ())
	{
		// The function keys trigger actions
		// ATTENTION: any changes here should be reflected in the menu entries' text.
        case Qt::Key_F2:  if (databaseActionsEnabled) ui.actionNewWizard     ->trigger (); break;
		case Qt::Key_F3:  if (databaseActionsEnabled) ui.actionRepeat        ->trigger (); break;
		case Qt::Key_F4:  if (databaseActionsEnabled) ui.actionEdit          ->trigger (); break;
		case Qt::Key_F5:  if (databaseActionsEnabled) ui.actionDepart        ->trigger (); break;
		case Qt::Key_F6:  if (databaseActionsEnabled) ui.actionLand          ->trigger (); break;
		case Qt::Key_F7:  if (databaseActionsEnabled) ui.actionTouchngo      ->trigger (); break;
		case Qt::Key_F8:  if (databaseActionsEnabled) ui.actionDelete        ->trigger (); break;
		case Qt::Key_F9:  if (databaseActionsEnabled) ui.actionSort          ->trigger (); break;
		case Qt::Key_F10: if (databaseActionsEnabled) ui.actionSetDisplayDate->trigger (); break;
		case Qt::Key_F11: if (databaseActionsEnabled) ui.actionHideFinished  ->trigger (); break;
		case Qt::Key_F12: if (databaseActionsEnabled) ui.actionRefreshAll    ->trigger (); break;

		// Flight manipulation
		// Note that we used to check for ui.flightTable->hasFocus() here, but this prevents
		// the actions from working when a table button is focused.
		case Qt::Key_Insert: if (databaseActionsEnabled) ui.actionNew    ->trigger (); break;
		case Qt::Key_Delete: if (databaseActionsEnabled) ui.actionDelete ->trigger (); break;

		case Qt::Key_Return:
		case Qt::Key_Enter:
			if (databaseActionsEnabled)
			{
				if (e->modifiers () & Qt::ControlModifier)
					ui.actionRepeat ->trigger ();
				else
					ui.actionEdit   ->trigger ();
			}
			break;
		case Qt::Key_Space:
			if (databaseActionsEnabled)
				departOrLand ();

			break;

		default: e->ignore (); break;
	}

	QMainWindow::keyPressEvent (e);
}

/**
 * Not using the activated signal because it may be emitted on single click,
 * depending on the desktop settings.
 */
void MainWindow::on_flightTable_doubleClicked (const QModelIndex &index)
{
	if (index.isValid ())
		ui.actionEdit->trigger ();
	else
		ui.actionNew->trigger ();
}

void MainWindow::on_flightTable_customContextMenuRequested (const QPoint &pos)
{
	contextMenu->clear ();

	if (ui.flightTable->indexAt (pos).isValid ())
	{
		contextMenu->addAction (ui.actionNew);
		contextMenu->addSeparator ();
		contextMenu->addAction (ui.actionDepart);
		contextMenu->addAction (ui.actionLand);
		contextMenu->addAction (ui.actionTouchngo);
		contextMenu->addSeparator ();
		contextMenu->addAction (ui.actionEdit);
		contextMenu->addAction (ui.actionRepeat);
		contextMenu->addAction (ui.actionDelete);
		contextMenu->addSeparator ();
		contextMenu->addAction (ui.identifyPlaneAction);
		contextMenu->addAction (ui.updateFlarmIdAction);
		contextMenu->addSeparator ();
		contextMenu->addAction (ui.actionJumpToTow);
	}
	else
	{
		contextMenu->addAction (ui.actionNew);
	}

	contextMenu->popup (ui.flightTable->mapToGlobal (pos), 0);
}

void MainWindow::on_flightTable_departButtonClicked (FlightReference flight)
{
	if (flight.isValid ())
		interactiveDepartFlight (flight.id ());
}

void MainWindow::on_flightTable_landButtonClicked (FlightReference flight)
{
	if (flight.towflight ())
		interactiveLandTowflight (flight.id ());
	else
		interactiveLandFlight (flight.id ());
}

void MainWindow::updateTimeLabels (const QDateTime &now)
{
	ui.utcTimeLabel  ->setText (now.toUTC       ().toString (defaultNumericDateTimeFormat ()));
	ui.localTimeLabel->setText (now.toLocalTime ().toString (defaultNumericDateTimeFormat ()));
}

void MainWindow::timeTimer_timeout ()
{
	QDateTime now=QDateTime::currentDateTime ();

	updateTimeLabels (now);

	static int lastSecond=0;
	int second=QTime::currentTime ().second ();

	// Some things are done on the beginning of a new minute.
	if (second<lastSecond)
		emit minuteChanged ();

	lastSecond=second;
}

void MainWindow::weatherWidget_doubleClicked ()
{
	Settings &s=Settings::instance ();

	if (weatherDialog)
	{
		// How to raise a QDialog?
		weatherDialog->hide ();
		weatherDialog->show ();
	}
	else
	{
		if (s.weatherWindowEnabled && !isBlank (s.weatherWindowPluginId))
		{
			// The plugin will be deleted by the weather dialog
			WeatherPlugin *weatherDialogPlugin=PluginFactory::getInstance ().createWeatherPlugin (s.weatherWindowPluginId, s.weatherWindowCommand);

			if (weatherDialogPlugin)
			{
				weatherDialogPlugin->enableRefresh (s.weatherWindowInterval);

				// The weather dialog will be deleted when it's closed, and
				// weatherDialog is a QPointer, so it will be set to NULL.
				weatherDialog = new WeatherDialog (weatherDialogPlugin, this);
				weatherDialog->setAttribute (Qt::WA_DeleteOnClose, true);
				weatherDialog->setWindowTitle (s.weatherWindowTitle);
				weatherDialog->show ();
			}
		}
	}
}

// **********
// ** Date **
// **********

/**
 * Determines the date to use for new flights
 *
 * This is, depending on the current setting, either the current date (today)
 * or the current displayed date.
 *
 * @return the date to use for new flights
 */
QDate MainWindow::getNewFlightDate ()
{
	if (ui.actionUseCurrentDateForNewFlights->isChecked ())
		return QDate::currentDate ();
	else
		return displayDate;
}


/**
 *
 * @param displayDate null means current
 * @param force true means even if already that date
 */
void MainWindow::setDisplayDate (QDate newDisplayDate, bool force)
{
	// TODO this should be correct now, but it still sucks. The better solution
	// would probably be just to have a flag "displayToday" here, and use the
	// cache's "other" date as displayed date. This would avoid date==today
	// comparisons, prevent inconsistencies and make handling date changes
	// easier (prevent race conditions on date change).

	// Fetch the current date to avoid it changing during the operation
	QDate today=QDate::currentDate ();

	// If the displayed date is null, use the current date
	if (newDisplayDate.isNull ()) newDisplayDate = today;

	// If the displayed date is already current, don't do anything (unless force
	// is true)
	if (newDisplayDate==displayDate && !force) return;

	if (newDisplayDate==today)
	{
		// Setting today's displayed date
		// Since today's flights are always cached, we can do this
		// unconditionally.
		displayDate=newDisplayDate;
	}
	else
	{
		// Setting another date

		try
		{
			// If the new displayed date is not in the cache, fetch it.
			// TODO move that to fetchFlights() (with force flag)
			if (newDisplayDate!=dbManager.getCache ().getOtherDate ())
				dbManager.fetchFlights (newDisplayDate, this);

			// Now the displayed date is the one in the cache (which should be
			// newDisplayDate).
			displayDate=dbManager.getCache ().getOtherDate ();
		}
		catch (OperationCanceledException &ex)
		{
			// The fetching was canceled. Don't change the displayed date.
		}
	}

	// Update the display
	refreshFlights ();
}

void MainWindow::on_actionSetDisplayDate_triggered ()
{
	QDate newDisplayDate = displayDate;
	if (DateInputDialog::editDate (&newDisplayDate, tr ("Set displayed date"), tr ("Displayed date:"), this))
		setDisplayDate (newDisplayDate, true);
}

// ****************
// ** Statistics **
// ****************

// Note that these strings must be defined ouside of the functions because the
// window may have to access them (for retranslation) after the function
// returns.
const char *ntr_planeLogBooksTitle       =QT_TRANSLATE_NOOP ("StatisticsWindow", "Plane logbooks");
const char *ntr_pilotLogBooksTitle       =QT_TRANSLATE_NOOP ("StatisticsWindow", "Pilot logbooks");
const char *ntr_launchMethodOverviewTitle=QT_TRANSLATE_NOOP ("StatisticsWindow", "Launch method overview");
const char *ntr_flarmPlaneListTitle      =QT_TRANSLATE_NOOP ("StatisticsWindow", "Flarm overview");

void MainWindow::on_actionPlaneLogs_triggered ()
{
	// Get the list of flights and add the towflights
	QList<Flight> flights=flightList->getList ();
	flights+=Flight::makeTowflights (flights, cache);

	PlaneLog *planeLog = PlaneLog::createNew (flights, cache);
	StatisticsWindow::display (planeLog, true, ntr_planeLogBooksTitle, this);
}

void MainWindow::on_actionPilotLogs_triggered ()
{
	// Get the list of flights and add the towflights
	QList<Flight> flights=flightList->getList ();
	flights+=Flight::makeTowflights (flights, cache);

	// Create the pilots' log
	PilotLog *pilotLog = PilotLog::createNew (flights, cache);

	// Display the pilots' log
	StatisticsWindow::display (pilotLog, true, ntr_pilotLogBooksTitle, this);
}

void MainWindow::on_actionLaunchMethodStatistics_triggered ()
{
	// Get the list of flights and add the towflights
	QList<Flight> flights=flightList->getList ();
	flights+=Flight::makeTowflights (flights, cache);

	// Create the launch method statistics
	LaunchMethodStatistics *stats=LaunchMethodStatistics::createNew (flights, cache);

	// Display the launch method statistics
	StatisticsWindow::display (stats, true, ntr_launchMethodOverviewTitle, this);
}


// ***********
// ** Flarm **
// ***********

void MainWindow::on_actionFlarmPlaneList_triggered ()
{
	if (!Settings::instance ().flarmDataViewable)
	{
		QString text=tr ("Viewing Flarm data is disabled. It can be enabled in"
			" the configuration.");
		showWarning ("Flarm plane list disabled", text, this);
		return;
	}

	FlarmWindow* dialog = new FlarmWindow (this);
	dialog->setGpsTracker (flarm.gpsTracker ());
	dialog->setFlarmList (flarm.flarmList ());
	dialog->setAttribute (Qt::WA_DeleteOnClose, true);
	dialog->showPlaneList ();
}

void MainWindow::on_actionFlarmRadar_triggered ()
{
	if (!Settings::instance ().flarmDataViewable)
	{
		QString text=tr ("Viewing Flarm data is disabled. It can be enabled in"
			" the configuration.");
		showWarning ("Flarm radar disabled", text, this);
		return;
	}

	FlarmWindow* dialog = new FlarmWindow (this);
	dialog->setGpsTracker (flarm.gpsTracker ());
	dialog->setFlarmList (flarm.flarmList ());
	dialog->setAttribute (Qt::WA_DeleteOnClose, true);
	dialog->showFlarmMap ();
}

void MainWindow::on_actionFlarmNetWindow_triggered ()
{
	FlarmNetWindow *dialog = new FlarmNetWindow (dbManager, this);
	dialog->setAttribute (Qt::WA_DeleteOnClose, true);
	dialog->show ();
}

void MainWindow::on_flarmNetImportFileAction_triggered ()
{
	FlarmNetHandler handler (dbManager, this);
	handler.interactiveImportFromFile ();
}

void MainWindow::on_flarmNetImportWebAction_triggered ()
{
	FlarmNetHandler handler (dbManager, this);
	handler.interactiveImportFromWeb ();
}


// **************
// ** Database **
// **************

void MainWindow::on_actionConnect_triggered ()
{
	// Does not throw OperationCanceledException, ConnectionFailedException, ConnectCanceledException, SqlException
	dbManager.connect (this);
}

void MainWindow::on_actionDisconnect_triggered ()
{
	// Does not throw OperationCanceledException, ConnectionFailedException, ConnectCanceledException, SqlException
	dbManager.disconnect ();
}

void MainWindow::on_actionEditPlanes_triggered ()
{
	ObjectListWindow<Plane>::show (dbManager, this);
}

void MainWindow::on_actionEditPeople_triggered ()
{
	ObjectListWindow<Person>::show (dbManager, this);
}

void MainWindow::on_actionEditLaunchMethods_triggered ()
{
	ObjectListWindow<LaunchMethod>::show (dbManager,
		Settings::instance ().protectLaunchMethods,
		this);
}

void MainWindow::on_actionShowFlights_triggered ()
{
	if (Settings::instance ().protectFlightDatabase)
	{
		QString password=Settings::instance ().databaseInfo.password;
		QString message=tr ("The database password must be entered to display the flight database.");

		if (!verifyPassword (this, password, message))
			return;
	}

	FlightListWindow::show (dbManager, this);
}


// **************
// ** Database **
// **************

void MainWindow::databaseError (int number, QString message)
{
	if (number==0)
		statusBar ()->clearMessage ();
	else
	{
		logMessage (message);
		statusBar ()->showMessage (tr ("Database: %2 (%1)").arg (number).arg (message), 2000);
	}
}

void MainWindow::executingQuery (Query query)
{
	logMessage (query.toString ());
}

void MainWindow::cacheChanged (DbEvent event)
{
	assert (isGuiThread ());

	std::cout << notr ("MainWindow: ") << event.toString () << std::endl;

	try
	{
		// TODO when a plane, person or launch method is changed, the flight list
		// has to be updated, too. But that's a feature of the FlightListModel (?).
		if (event.hasTable<Flight> ())
		{
			switch (event.getType ())
			{
				case DbEvent::typeAdd:
				{
					Flight flight=event.getValue<Flight> ();
					if (flight.isPrepared () || flight.effdatum ()==displayDate)
						flightList->append (flight);

					// TODO: set the cursor position to the flight

					// TODO introduce Flight::hasDate (timeZone)
					if (ui.actionResetDisplayDateOnNewFlight->isChecked ())
					{
						if (flight.isPrepared ())
							setDisplayDateCurrent (false);
						else
							setDisplayDate (flight.effdatum (), false);
					}
				} break;
				case DbEvent::typeChange:
				{
					Flight flight=event.getValue<Flight> ();

					if (flight.isPrepared () || flight.effdatum ()==displayDate)
						flightList->replaceOrAdd (flight.getId (), flight);
					else
						flightList->removeById (flight.getId ());
				} break;
				case DbEvent::typeDelete:
					flightList->removeById (event.getId ());
					break;
			}
		}
	}
	catch (Cache::NotFoundException &)
	{
		// TODO log error
	}

	// Cannot use foreach because flightList can only return a copy of its list
	int numFlights=flightList->size ();
	for (int i=0; i<numFlights; ++i)
		flightList->at (i).databaseChanged (event);
}

void MainWindow::updateDatabaseStateLabel (DbManager::State state)
{
	switch (state)
	{
		case DbManager::stateDisconnected:
			ui.databaseStateLabel->resetDefaultForegroundColor ();
			ui.databaseStateLabel->setText (tr ("Not connected"));
			break;
		case DbManager::stateConnecting:
			ui.databaseStateLabel->resetDefaultForegroundColor ();
			ui.databaseStateLabel->setText (tr ("Connecting..."));
			break;
		case DbManager::stateConnected:
			ui.databaseStateLabel->resetDefaultForegroundColor ();
			ui.databaseStateLabel->setText (tr ("OK"));
			break;
		// no default
	}
}

void MainWindow::databaseStateChanged (DbManager::State state)
{
	updateDatabaseStateLabel (state);

	switch (state)
	{
		case DbManager::stateDisconnected:
			ui.flightTable->setVisible (false);
			ui.notConnectedPane->setVisible (true);
			setDatabaseActionsEnabled (false);

			refreshFlights (); // Also sets the labels

			break;
		case DbManager::stateConnecting:
			ui.flightTable->setVisible (true);
			ui.notConnectedPane->setVisible (false);
			setDatabaseActionsEnabled (false);

			refreshFlights (); // Also sets the labels

			break;
		case DbManager::stateConnected:
			ui.flightTable->setVisible (true);
			ui.notConnectedPane->setVisible (false);
			setDatabaseActionsEnabled (true);

			setDisplayDateCurrent (true); // Will also call refreshFlights

			ui.flightTable->setFocus ();

			break;
		// no default
	}
}

QString linkTo (const QString &target, const QString &text)
{
	return qnotr ("<a href=\"%1\">%2</a>").arg (target).arg (text);
}

void MainWindow::updateFlarmStateEnabled (ManagedDataStream::State::Type state)
{
	QString text;

	QString linkTarget=notr ("flarmStreamErrorDetails");

	switch (state)
	{
		case ManagedDataStream::State::noConnection:
			text=tr ("No connection");
			break;
		case ManagedDataStream::State::closed:
			text=tr ("Closed");
			break;
		case ManagedDataStream::State::opening:
			// TODO distinguish:
			//   * connection failed - reconnecting (with link)
			//   * connection lost - reconnecting (with link)
			//   * connecting
			text=tr ("Connecting");
			break;
		case ManagedDataStream::State::open:
			text=tr ("Connected");
			break;
		case ManagedDataStream::State::ok:
			text=tr ("OK");
			break;
		case ManagedDataStream::State::timeout:
			text=tr ("No data");
			break;
		case ManagedDataStream::State::error:
		{
			// TODO distinguish between "connection failed" and "connection lost"
			text=linkTo (linkTarget, tr ("Error"));

			QTime reconnectTime=flarm.getManagedStream ()->getReconnectTime ();
			if (reconnectTime.isValid ())
			{
				QTime now=QTime::currentTime ();
				int msToReconnect=now.msecsTo (reconnectTime);
				int sToReconnect=(msToReconnect/1000)+1;
				text+=", "+tr ("reconnect in %1 s").arg (sToReconnect);

				// Update the status again in 200 ms to update the remaining
				// time. This will continue until the state is no longer
				// ManagedDataStream::State::error, or until the reconnect time
				// reported by the managed data stream is invalid.
				QTimer::singleShot (200, this, SLOT (updateFlarmStreamState ()));
			}
			flarmConnectionError=flarm.getManagedStream ()->getErrorMessage ();
		} break;
		// no default
	}

	ui.flarmStateLabel->setText (text);
}

void MainWindow::updateFlarmStateDisabled ()
{
	ui.flarmStateLabel->setText (tr ("Disabled"));
}

void MainWindow::updateFlarmState (Flarm::State::Type state)
{
	switch (state)
	{
		case Flarm::State::active:
			updateFlarmStateEnabled (flarm.getManagedStream ()->getState ());
			break;
		case Flarm::State::disabled:
			updateFlarmStateDisabled ();
			break;
	}
}

void MainWindow::updateFlarmState ()
{
	updateFlarmState (flarm.state ());
}

void MainWindow::flarm_stateChanged (Flarm::State::Type state)
{
	updateFlarmState (state);
}

void MainWindow::flarm_streamStateChanged (ManagedDataStream::State::Type state)
{
	// FIXME state mix-up, there should only be a single Flarm state
	updateFlarmStateEnabled (state);
}

void MainWindow::flarmStreamLinkActivated (const QString &link)
{
	(void)link;

	QString text;

	// TODO distinguish between "connection failed" and "connection lost":
	//   - "the connection was terminated"
	//   - "the connection could not be established"

	if (flarmConnectionError.isEmpty ())
		text=tr ("The connection encountered an error and will be reopened automatically.");
	else
		text=tr ("The connection encountered an error: %1. The connection will be reopened automatically.").arg (flarmConnectionError);

	QString title=tr ("Flarm connection error");
	QMessageBox::information (this, title, text);
}


// ***************************
// ** Connection monitoring **
// ***************************

void MainWindow::readTimeout ()
{
	ui.databaseStateLabel->setPaletteForegroundColor (Qt::red);
	ui.databaseStateLabel->setText (tr ("No reply"));
}

void MainWindow::readResumed ()
{
	ui.databaseStateLabel->resetDefaultForegroundColor ();
	ui.databaseStateLabel->setText (databaseOkText);
}




// ************************************
// ** Database connection management **
// ************************************

void MainWindow::setDatabaseActionsEnabled (bool enabled)
{
	databaseActionsEnabled=enabled;

	ui.actionDelete                  ->setEnabled (enabled);
	ui.actionEdit                    ->setEnabled (enabled);
	ui.actionEditPeople              ->setEnabled (enabled);
	ui.actionEditPlanes              ->setEnabled (enabled);
	ui.actionEditLaunchMethods       ->setEnabled (enabled);
	ui.actionJumpToTow               ->setEnabled (enabled);
	ui.actionLand                    ->setEnabled (enabled);
	ui.actionLaunchMethodStatistics  ->setEnabled (enabled);
	ui.actionNew                     ->setEnabled (enabled);
	ui.actionLaunchMethodPreselection->setEnabled (enabled);
	ui.actionPilotLogs               ->setEnabled (enabled);
	ui.actionPingServer              ->setEnabled (enabled);
	ui.actionPlaneLogs               ->setEnabled (enabled);
	ui.actionShowFlights             ->setEnabled (enabled);
	ui.actionRefreshAll              ->setEnabled (enabled);
	ui.actionRefreshTable            ->setEnabled (enabled);
	ui.actionRepeat                  ->setEnabled (enabled);
	ui.actionSetDisplayDate          ->setEnabled (enabled);
	ui.actionDepart                  ->setEnabled (enabled);
	ui.actionTouchngo                ->setEnabled (enabled);
	ui.actionDisplayError            ->setEnabled (enabled);

	ui.flightTable->setEnabled (enabled);

	// Connect/disconnect are special
	ui.actionConnect    ->setEnabled (!enabled);
	ui.actionDisconnect ->setEnabled ( enabled);
}

void MainWindow::closeDatabase ()
{
	// No need, will be closed on destruction
	//dbInterface.close ();
}


// **********
// ** Misc **
// **********

void MainWindow::on_actionSettings_triggered ()
{
	SettingsWindow w (this);
	w.setModal (true); // TODO non-modal and auto delete
	w.exec ();


	// Check if the database changed
	if (dbManager.getState ()==DbManager::stateConnected && w.databaseSettingsChanged)
	{
		QString title=tr ("Database settings changed");

		QString text=tr (
			"The database settings were changed."
			" The changes will be effective on the next reconnect."
			" Do you want to reconnect now?");

		if (yesNoQuestion (this, title, text))
		{
			on_actionDisconnect_triggered ();
			on_actionConnect_triggered ();
		}
	}
}

void MainWindow::on_actionSetTime_triggered ()
{
	// Store the current time to avoid a midnight race condition
	QDateTime oldDateTime = QDateTime::currentDateTime ();

	QDate date = oldDateTime.date ();
	QTime time = oldDateTime.time ();


	if (DateTimeInputDialog::editDateTime (this, &date, &time, tr ("Set system time")))
	{
                QString timeString (QDateTime(date, time).toString (notr("yyyy-MM-dd hh:mm:ss")));

		// sudo -n: non-interactive (don't prompt for password)
		// sudoers entry: deffi ALL=NOPASSWD: /bin/date

		int result=QProcess::execute (notr ("sudo"), QStringList () << notr ("-n") << notr ("date") << notr ("-s") << timeString);

		if (result==0)
		{
			showWarning (tr ("System time changed"),
				tr ("The system time was changed. The setting"
				" may only be stored permanently when the system is shut down."), this);
		}
		else
		{
			showWarning (tr ("Error"),
				tr ("Changing the system time failed."
				" Maybe the user has insufficient permissions."),
				this);
		}
	}
}

void MainWindow::on_actionSetGPSTime_triggered ()
{
        qDebug () << "MainWindow::on_actionSetGPSTime_triggered";
        // FIXME
//        if (!flarm.isDataValid ())
//        {
//        	QMessageBox::warning (this, tr ("No GPS signal"),
//        			tr ("Flarm does not send data"));
//        	return;
//        }
        QDateTime current (QDateTime::currentDateTimeUtc ());
        QDateTime currentGPSdateTime (flarm.getGpsTime ());
        qDebug () << "slot_setGPSdateTime: " << currentGPSdateTime.toString (notr("hh:mm:ss dd.MM.yyyy"));
        qDebug () << "currentTime: " << current.toString (notr("hh:mm:ss dd.MM.yyyy"));
        int diff = currentGPSdateTime.secsTo(current);
        if (abs (diff) > 0) {
                if (QMessageBox::question(this, tr("Time difference"), 
                        tr("<p>System time: %1</p>"
                        "<p>GPS time: %2</p>"
                        "<p>The system time differs by %3 seconds from the GPS time.</p>"
                        "<p>Correction?</p>")
                        .arg(current.toString (notr("hh:mm:ss dd.MM.yyyy")))
                        .arg(currentGPSdateTime.toString (notr("hh:mm:ss dd.MM.yyyy")))
                        .arg(diff), QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
                {
                        // Get the current GPS time again, the messagebox can stay open a long time
                		QDateTime gpsTime=flarm.getGpsTime ();
                        QString timeString (gpsTime.toString (notr("yyyy-MM-dd hh:mm:ss")));

                        // sudo -n: non-interactive (don't prompt for password)
                        // sudoers entry: deffi ALL=NOPASSWD: /bin/date

                        int result=QProcess::execute (notr ("sudo"), QStringList () << notr ("-n") << notr ("date") << notr ("-s") << timeString);
                        
                        if (result==0)
                        {
			        showWarning (tr ("System time changed"),
				        tr ("The system time was changed. The setting"
				        " may only be stored permanently when the system is shut down."), this);
                        }
                        else
                        {
			        showWarning (tr ("Error"),
				        tr ("Changing the system time failed."
				        " Maybe the user has insufficient permissions."),
				        this);
                        }
                }
        }
        else
                QMessageBox::information (this, tr("System time"), tr ("The system time is correct"));

}

void MainWindow::on_actionTest_triggered ()
{
	// Perform a sleep task in the background
}

void MainWindow::logMessage (QString message)
{
	QString timeString = QTime::currentTime ().toString ();

	ui.logWidget->append (qnotr ("[%1] %2").arg (timeString).arg (message));

	// Scroll the log widget to the bottom. Note that ensurecursorVisible
	// may scroll to the bottom right, which is undesirable.
	QScrollBar *scrollBar=ui.logWidget->verticalScrollBar ();
	scrollBar->setValue (scrollBar->maximum ());
}

void MainWindow::on_changeLanguageAction_triggered ()
{
	TranslationManager::instance ().toggleLanguage ();
}

void MainWindow::on_timerBasedLanguageChangeAction_triggered ()
{
	if (ui.timerBasedLanguageChangeAction->isChecked ())
		translationTimer->start (1000);
	else
		translationTimer->stop ();

}

void MainWindow::languageChanged ()
{
	SkMainWindow<Ui::MainWindowClass>::languageChanged ();
	setupText ();

	updateDisplayDateLabel ();
	updateTimeLabels ();
	updateDatabaseStateLabel (dbManager.getState ());

	ui.flightTable->languageChanged ();

	restartPlugins ();

	// Do not call this, we use stored column widths which would be overwritten.
	//ui.flightTable->resizeColumnsToContents ();
}


// ***********
// ** Flarm **
// ***********

void MainWindow::on_injectFlarmDepartureAction_triggered ()
{
	QString flarmId=QInputDialog::getText (this, "Inject Flarm departure", "Flarm ID:", QLineEdit::Normal, debugFlarmId);
	if (!flarmId.isNull ())
	{
		flarmList_departureDetected (flarmId);
		debugFlarmId=flarmId;
	}
}

void MainWindow::on_injectFlarmLandingAction_triggered ()
{
	QString flarmId=QInputDialog::getText (this, "Inject Flarm landing", "Flarm ID:", QLineEdit::Normal, debugFlarmId);
	if (!flarmId.isNull ())
	{
		flarmList_landingDetected (flarmId);
		debugFlarmId=flarmId;
	}
}

void MainWindow::on_injectFlarmTouchAndGoAction_triggered ()
{
	QString flarmId=QInputDialog::getText (this, "Inject Flarm touch and go", "Flarm ID:", QLineEdit::Normal, debugFlarmId);
	if (!flarmId.isNull ())
	{
		flarmList_touchAndGoDetected (flarmId);
		debugFlarmId=flarmId;
	}
}

void MainWindow::on_lookupPlaneAction_triggered ()
{
	// Query the user for the Flarm ID
	QString flarmId=QInputDialog::getText (this, "Lookup plane", "Flarm ID:", QLineEdit::Normal, debugFlarmId);
	if (flarmId.isNull ())
		return;
	debugFlarmId=flarmId;

	// Do the lookup
	PlaneLookup::Result result=PlaneLookup (cache).lookupPlane (flarmId);

	// Display the result
	QString text="Not handled";
	if (result.plane.isValid ())
	{
		if (result.flarmNetRecord.isValid ())
			text=qnotr ("Plane %1 via FlarmNet record %2")
				.arg (result.plane->registration)
				.arg (result.flarmNetRecord->getId ());
		else
			text=qnotr ("Plane %1 directly").arg (result.plane->registration);
	}
	else if (result.flarmNetRecord.isValid ())
		text=qnotr ("FlarmNet record %1, registration %2")
			.arg (result.flarmNetRecord->getId ()).arg (result.flarmNetRecord->registration);
	else
		text=qnotr ("Not found");

	QString title=qnotr ("Lookup plane %1").arg (flarmId);
	QMessageBox::information (this, title, text);
}

void MainWindow::on_lookupFlightAction_triggered ()
{
	// Query the user for the Flarm ID
	QString flarmId=QInputDialog::getText (this, "Lookup flight", "Flarm ID:", QLineEdit::Normal, debugFlarmId);
	if (flarmId.isNull ())
		return;
	debugFlarmId=flarmId;

	// Query the user for the candidate flights
	// Used mnemonics: o c - p r f l t d
	ChoiceDialog choiceDialog (this);
	choiceDialog.setWindowTitle ("Choose candidate flights");
	choiceDialog.setText ("Choose the candidate flights for the flight lookup:");
	choiceDialog.addOption ("&Prepared flights");
	choiceDialog.addOption ("P&repared flights (with towflights)");
	choiceDialog.addOption ("&Flying flights");
	choiceDialog.addOption ("F&lying flights (with towflights)");
	choiceDialog.addOption ("All flights of &today");
	choiceDialog.addOption ("All flights of to&day (with towflights)");
	choiceDialog.setSelectedOption (0);
	if (choiceDialog.exec ()!=QDialog::Accepted)
		return;

	// Decode the choice to a basic set of candidates an and "include
	// towflights" flag
	int choice=0;
	bool includeTowflights=false;
	switch (choiceDialog.getSelectedOption ())
	{
		// Prepared flights
		case 0: choice=0; includeTowflights=false; break;
		case 1: choice=0; includeTowflights=true ; break;
		// Flying flights
		case 2: choice=1; includeTowflights=false; break;
		case 3: choice=1; includeTowflights=true ; break;
		// Flights of today
		case 4: choice=2; includeTowflights=false; break;
		case 5: choice=2; includeTowflights=true ; break;
		default:
			QMessageBox::warning (this, qnotr ("Unhandled choice"), qnotr ("Unhandled choice"));
			return;
	}

	// Fetch the flights
	QList<Flight> candidates;
	if (choice==0)
		candidates=cache.getPreparedFlights (includeTowflights).getList ();
	else if (choice==1)
		candidates=cache.getFlyingFlights (includeTowflights).getList ();
	else if (choice==2)
		candidates=cache.getFlightsToday (includeTowflights).getList ();
	else
	{
		QMessageBox::warning (this, qnotr ("Unhandled choice"), qnotr ("Unhandled choice"));
		return;
	}

	// Do the lookup
	FlightLookup::Result result=FlightLookup (cache).lookupFlight (candidates, flarmId);

	// Display the result
	QString text="Not handled";
	if (result.flightReference.isValid ())
	{
		if (result.plane.isValid ())
		{
			if (result.flarmNetRecord.isValid ())
				text=qnotr ("%1 via plane %2 via FlarmNet record %3")
					.arg (result.flightReference.toString ("Flight", "Towflight"))
					.arg (result.plane->registration)
					.arg (result.flarmNetRecord->getId ());
			else
				text=qnotr ("%1 via plane %2 directly")
					.arg (result.flightReference.toString ("Flight", "Towflight"))
					.arg (result.plane->registration);
		}
		else
		{
			text=qnotr ("%1 directly").arg (result.flightReference.toString ("Flight", "Towflight"));
		}
	}
	else if (result.plane.isValid ())
	{
		if (result.flarmNetRecord.isValid ())
			text=qnotr ("Plane %1 via FlarmNet record %2")
				.arg (result.plane->registration)
				.arg (result.flarmNetRecord->getId ());
		else
			text=qnotr ("Plane %1 directly").arg (result.plane->registration);
	}
	else if (result.flarmNetRecord.isValid ())
		text=qnotr ("FlarmNet record %1, registration %2")
			.arg (result.flarmNetRecord->getId ()).arg (result.flarmNetRecord->registration);
	else
		text=qnotr ("Not found");

	QString title=qnotr ("Lookup flight %1").arg (flarmId);
	QMessageBox::information (this, title, text);
}


Flight MainWindow::createFlarmFlight (const FlightLookup::Result &lookupResult, const QString &flarmId)
{
	Flight flight;

	if (lookupResult.plane.isValid ())
		flight.setPlaneId (lookupResult.plane->getId ());

	flight.setFlarmId (flarmId);

	if (lookupResult.flarmNetRecord.isValid ())
	{
		flight.setComments (tr ("Registration from FlarmNet: %1")
			.arg (lookupResult.flarmNetRecord->registration));
	}

	return flight;
}

/**
 * We use the non-interactive flight update methods (nonInteractiveDepartFlight
 * etc.) because we do not want popups (we still get the status window, but we
 * might be able to use a different status indicator for that). This means that
 * we cannot perform plausibility checks like "person is still flying". There
 * should be a way to report failed plausibility checks in an unobstrusive way.
 */
void MainWindow::flarmList_departureDetected (const QString &flarmId)
{
	// Ignore the event if handling of Flarm events is disabled
	Settings &s=Settings::instance ();
	if (!(s.flarmEnabled && s.flarmAutoDepartures))
		return;

	if (departureTracker.eventWithin (flarmId, ignoreDuplicateFlarmEventInterval))
	{
		std::cout <<
			qnotr ("Ignored departure of %1 because it departed %2 minutes ago")
			.arg (flarmId)
			.arg (departureTracker.timeSinceEvent (flarmId).toString ("m:ss"))
			<< std::endl;

		return;
	}

	std::cout << "Detected departure of " << flarmId << std::endl;

	// Make a list of candidate flights. Start with the prepared flights,
	// including towflights (so we can identify the flights when the departure
	// of the towplane is detected).
	QList<Flight> flights=cache.getPreparedFlights (true).getList ();

	// Also add the flying flights along with their towflights to the list of
	// candidates. That way, we can handle the case that the flight has already
	// departed (this can happen when both a plane and the towplane have a
	// Flarm, or if a departure is misdetected).
	flights+=cache.getFlyingFlights (true).getList ();

	// Find the flight
	FlightLookup flightLookup (cache);
	FlightLookup::Result lookupResult=flightLookup.lookupFlight (flights, flarmId);

	if (lookupResult.flightReference.isValid ())
	{
		// We found the flight. Depart it, unless it's already flying. This is
		// the same for flights and towflights.
		try
		{
			Flight flight=cache.getObject<Flight> (lookupResult.flightReference.id ());
			if (flight.isFlying ())
			{
				std::cout << qnotr ("Departure of Flarm ID %1 ignored because "
					"flight %2 is already flying").arg (flarmId).arg (flight.getId ())
					<< std::endl;
			}
			else
			{
				bool departed=nonInteractiveDepartFlight (lookupResult.flightReference.id ());

                if (departed)
                {
					ui.flightTable->showNotification (
						lookupResult.flightReference,
						tr ("The flight was departed automatically"),
						notificationDisplayTime);

                    playDepartedSound();
                }
			}
		}
		catch (Cache::NotFoundException &ex) {}
	}
	else
	{
		// We did not find the flight. Create it. The data will be incomplete
		// and the flight will be shown in red.
		Flight flight=createFlarmFlight (lookupResult, flarmId);
		flight.setMode (FlightBase::modeLocal);
        flight.setLaunchMethodId (Settings::instance().preselectedLaunchMethod);
		flight.departNow (Settings::instance ().location);
		dbId flightId=dbManager.createObject (flight, this);
		// Since the flight is newly created, it is not updated and we can't
		// detect the change automatically.
		flightDeparted (flightId);

		if (idValid (flightId))
        {
			ui.flightTable->showNotification (
				FlightReference::flight (flightId),
				tr ("The flight was created automatically"),
				notificationDisplayTime);

            playDepartedSound();
        }
	}
}

/**
 * See flarmList_departureDetected
 */
void MainWindow::flarmList_landingDetected (const QString &flarmId)
{
	// Ignore the event if handling of Flarm events is disabled
	Settings &s=Settings::instance ();
	if (!(s.flarmEnabled && s.flarmAutoDepartures))
		return;

	if (landingTracker.eventWithin (flarmId, ignoreDuplicateFlarmEventInterval))
	{
		std::cout <<
			qnotr ("Ignored landing of %1 because it landed %2 minutes ago")
			.arg (flarmId)
			.arg (landingTracker.timeSinceEvent (flarmId).toString ("m:ss"))
			<< std::endl;

		return;
	}


	std::cout << "Detected landing of " << flarmId << std::endl;

	// Get a list of flying flights, including towflights. Note that, contrary
	// to the prepared flights (needed for departures), a flying towflight does
	// not necessarily correspond to a flying flight, nor vice versa. Therefore,
	// we need to base the list of prepared towflights on the list of all
	// relevant flights, whether flying or not.
	QList<Flight> flights=dbManager.getCache ().getFlyingFlights (true).getList ();

	// Find out if one of the flights (or towflights) can be matched to the
	// Flarm ID
	FlightLookup flightLookup (cache);
	FlightLookup::Result lookupResult=flightLookup.lookupFlight (flights, flarmId);

	if (lookupResult.flightReference.isValid ())
	{
		bool landed;
		// We found the flight. Land it.
		if (lookupResult.flightReference.towflight ())
			landed=nonInteractiveLandTowflight (lookupResult.flightReference.id ());
		else
			landed=nonInteractiveLandFlight (lookupResult.flightReference.id ());

		if (landed)
        {
			ui.flightTable->showNotification (
				lookupResult.flightReference,
				tr ("The flight was landed automatically"),
				notificationDisplayTime);

           playLandedSound();
        }
	}
	else
	{
		// We did not find the flight. Create it. The data will be incomplete
		// and the flight will be shown in red.
		Flight flight=createFlarmFlight (lookupResult, flarmId);
		flight.setMode (FlightBase::modeComing);
		flight.landNow (Settings::instance ().location);
		dbId flightId=dbManager.createObject (flight, this);
		// Since the flight is newly created, it is not updated and we can't
		// detect the change automatically.
		flightLanded (flightId);

		if (idValid(flightId))
			ui.flightTable->showNotification (
				FlightReference::flight (flightId),
				tr ("The flight was created automatically"),
				notificationDisplayTime);
	}
}

/**
 * See flarmList_departureDetected
 */
void MainWindow::flarmList_touchAndGoDetected (const QString &flarmId)
{
	// Ignore the event if handling of Flarm events is disabled
	Settings &s=Settings::instance ();
	if (!(s.flarmEnabled && s.flarmAutoDepartures))
		return;

	if (touchAndGoTracker.eventWithin (flarmId, ignoreDuplicateFlarmEventInterval))
	{
		std::cout <<
			qnotr ("Ignored touch-and-go of %1 because it performed a touch-and-go %2 minutes ago")
			.arg (flarmId)
			.arg (touchAndGoTracker.timeSinceEvent (flarmId).toString ("m:ss"))
			<< std::endl;

		return;
	}

	std::cout << "Detected touch-and-go of " << flarmId << std::endl;

	// The candidates for the towflights are flying flights, including
	// towflights. The touch-and-go will be ignored for towflights, but we still
	// include them in the list so no new flight is created.
	QList<Flight> flights=dbManager.getCache ().getFlyingFlights (true).getList ();
	FlightLookup flightLookup (cache);
	FlightLookup::Result lookupResult=flightLookup.lookupFlight (flights, flarmId);

	if (lookupResult.flightReference.isValid ())
	{
		// We found the flight. Perform a touch and go (if possible). Ignore the
		// event for towflights, they can't perform a touch-and-go.
		if (!lookupResult.flightReference.towflight ())
		{
			bool performed=nonInteractiveTouchAndGo (lookupResult.flightReference.id ());

			if (performed)
				ui.flightTable->showNotification (
					lookupResult.flightReference,
					tr ("The flight performed a touch-and-go automatically"),
					notificationDisplayTime);
		}
	}
	else
	{
		// We did not find the flight. Create it. The data will be incomplete
		// and the flight will be shown in red.
		Flight flight=createFlarmFlight (lookupResult, flarmId);
		flight.setMode (FlightBase::modeComing);
		flight.performTouchngo ();
		dbId flightId=dbManager.createObject (flight, this);
		// Since the flight is newly created, it is not updated and we can't
		// detect the change automatically.
		touchAndGoPerformed (flightId);

		if (idValid(flightId))
			ui.flightTable->showNotification (
				FlightReference::flight (flightId),
				tr ("The flight was created automatically"),
				notificationDisplayTime);
	}
}

void MainWindow::on_showNotificationAction_triggered ()
{
	FlightReference flight=ui.flightTable->selectedFlightReference ();

	ui.flightTable->showNotification (
		flight,
		QString ("NotificationWidget test for flight %1").arg (flight.id ()),
		5000);
}

void MainWindow::flightDeparted (dbId id)
{
	// Departing a flight departs both the plane and the towplane
	QString flarmId;

	flarmId=determineFlarmId (id, false);
	if (!flarmId.isEmpty ())
		departureTracker.eventNow (flarmId);

	flarmId=determineFlarmId (id, true);
	if (!flarmId.isEmpty ())
		departureTracker.eventNow (flarmId);
}

void MainWindow::flightLanded (dbId id)
{
	QString flarmId=determineFlarmId (id, false);
	if (!flarmId.isEmpty ())
		landingTracker.eventNow (flarmId);
}

void MainWindow::towflightLanded (dbId id)
{
	QString flarmId=determineFlarmId (id, true);
	if (!flarmId.isEmpty ())
		landingTracker.eventNow (flarmId); // Note that we have the towplane's Flarm ID
}

void MainWindow::touchAndGoPerformed (dbId id)
{
	QString flarmId=determineFlarmId (id, false);
	if (!flarmId.isEmpty ())
		touchAndGoTracker.eventNow (flarmId);
}

// Meh, this stinks
/**
 * Returns the Flarm ID of the flight, if it has one, or of the plane (if it has
 * one, but the flight doesn't), or an empty QString otherwise.
 *
 * If ofTowflight is true, the towflight is considered instead of the flight,
 * and the Flarm ID of the flight is not used.
 *
 * @param flightId
 * @param ofTowflight
 * @return
 */
QString MainWindow::determineFlarmId (dbId flightId, bool ofTowflight)
{
	try
	{
		Flight flight=cache.getObject<Flight> (flightId);

		if (ofTowflight)
			flight=flight.makeTowflight (cache);

		// Try the Flarm ID of the flight (a towflight won't have one)
		if (!flight.getFlarmId ().isEmpty ())
			return flight.getFlarmId ();

		// Try the Flarm ID of the plane
		if (idValid (flight.getPlaneId ()))
		{
			Plane plane=cache.getObject<Plane> (flight.getPlaneId ());
			if (!plane.flarmId.isEmpty ())
				return plane.flarmId;
		}

		// No result
	}
	catch (Cache::NotFoundException &ex) {}

	return QString ();
}

void MainWindow::on_decodeFlarmNetFileAction_triggered ()
{
	FlarmNetHandler (dbManager, this).interactiveDecodeFile ();
}

void MainWindow::on_encodeFlarmNetFileAction_triggered ()
{
	FlarmNetHandler (dbManager, this).interactiveEncodeFile ();
}

void MainWindow::playDepartedSound ()
{
    //Phonon::MediaObject *music =
    //         Phonon::createPlayer(Phonon::MusicCategory,QString(":/snd_takeoff"));

    //music->play();
}

void MainWindow::playLandedSound ()
{
    //Phonon::MediaObject *music =
    //         Phonon::createPlayer(Phonon::MusicCategory,QString(":/snd_landing"));

    //music->play();
}
