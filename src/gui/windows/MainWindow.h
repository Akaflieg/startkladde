#ifndef MAINWINDOW_H
#define MAINWINDOW_H

/*
 * Remake:
 *   - TODO incoming motor flight without pilot is error in table, not in
 *     editor
 *   - TODO towplane type and registration not shown correctly
 *   - TODO the weather widget is still made smaller if the window is resized.
 *          Note that it will not get small than the fixed size set in the UI.
 *   - TODO change the display when the database does not reply to ping
 *   - TODO task error handling
 *   - TODO warn on edit if database not alive (recently responded)
 *   - TODO on startup, display only one progress dialog with multiple items
 *     which are checked after they are completed
 *
 * Tests:
 *   - TODO context menu: correct flight used
 *
 * Further improvements:
 *   - on repeating a towflight, ask if the towed flight should be repeated
 *   - change menu entries for "jump to towflight/towed flight" and "land/
 *     /end airtow" depending on the selected flight.
 *   - update action state if the virtual keyboard is closed/killed externally
 *   - reload weather plugin from context menu
 *   - add the weather plugin in designer (need different initialization)
 *   - add a menu entry for opening the weather dialog
 *   - focus the weather dialog instead of hiding and showing it
 *   - on date wrap, the display date label's color is not updated
 *   - double click in free table space: create new flight
 *   - make a script for date changing (set date and store to hardware clock)
 *   - when performing a touchngo with a towflight, land and depart it
 *   - allow repeating of towflights
 */

#include <QAction>
#include <QPointer>

#include "ui_MainWindow.h"

#include "src/db/DbManager.h"
#include "src/flarm/Flarm.h"
#include "src/io/dataStream/DataStream.h" // For DataStream::State
#include "src/gui/SkMainWindow.h"
#include "src/model/FlightBase.h"
#include "src/flarm/algorithms/FlightLookup.h" // For FlightLookup::Result
#include "src/FlightReference.h"
#include "src/time/EventTimeTracker.h"
#include "src/db/vfsync/vereinsfliegersyncworker.h"
#include "src/gui/windows/LoginDialog.h"

class QWidget;
template<class T> class QList;
class QModelIndex;

class InfoPlugin;
class WeatherPlugin;
class WeatherWidget;
class WeatherDialog;
class FlightWindow;
class FlightWizard;
class TrainingsBarometerDialog;
class Flarm;
class DbSync;

/*
 * Notes:
 *   - We don't enable/disable the flight manipulation menu entries depending
 *     on whether the manipulation can be performed. It has to be checked when
 *     it's performed anyway, and that way the user can be told why someting
 *     is not possible.
 */
class MainWindow: public SkMainWindow<Ui::MainWindowClass>
{
		Q_OBJECT

	public:
		MainWindow (QWidget *parent, DbManager &dbManager, Flarm &flarm);
		~MainWindow ();

	protected:
		// Startup/Shutdown
		bool confirmAndExit (int returnCode, QString title, QString text);

		// Setup
		void setupText ();
		void setupPlugins ();
		void setupLabels ();
		void setupLayout ();

		// Data
		QDate getNewFlightDate ();
		void setDisplayDate (QDate displayDate, bool force);
		void setDisplayDateCurrent (bool force) { setDisplayDate (QDate::currentDate (), force); }
		bool updateFlight (const Flight &flight);

		// Settings
		void writeSettings ();
		void readSettings ();
		void readColumnWidths ();

		// Events
		void closeEvent (QCloseEvent *event);
		void keyPressEvent (QKeyEvent *);

		// Database connection management
		void closeDatabase ();
		void setDatabaseActionsEnabled (bool enabled);

		// Plugins
		void setupPlugin (InfoPlugin *plugin, QGridLayout *pluginLayout);
		void terminatePlugins ();

		// Translation
		virtual void languageChanged ();

		void updateDisplayDateLabel (const QDate &today=QDate::currentDate ());
		void updateTimeLabels (const QDateTime &now=QDateTime::currentDateTime ());
		void updateDatabaseStateLabel (DbManager::State state);

		// Flarm
		void interactiveUpdateFlarmId (dbId flightId);

	signals:
		void minuteChanged ();

	protected slots:
		void databaseError (int number, QString message);
		void databaseStateChanged (DbManager::State state);
		void flarm_stateChanged (Flarm::State::Type state);
		void flarm_streamStateChanged (ManagedDataStream::State::Type state);

		void updateFlarmStateEnabled (ManagedDataStream::State::Type state);
		void updateFlarmStateDisabled ();
		void updateFlarmState (Flarm::State::Type state);
		void updateFlarmState ();

		void readTimeout ();
		void readResumed ();

		// Actions
		void refreshFlights ();

		// Flight changes
		void flightDeparted (dbId id);
		void flightLanded (dbId id);
		void towflightLanded (dbId id);
		void touchAndGoPerformed (dbId id);

		void migrationStarted () { oldLogVisible=ui.logDockWidget->isVisible (); ui.logDockWidget->setVisible (true); }
		void migrationEnded () { ui.logDockWidget->setVisible (oldLogVisible); }

		void settingsChanged ();

		void flarmStreamLinkActivated (const QString &link);

	private slots:
		// Menu: Program
		void on_actionSettings_triggered ();
		void on_actionSetTime_triggered ();
		void on_actionSetGPSTime_triggered ();
		void on_actionQuit_triggered ();
		void on_actionShutdown_triggered ();

		// Menu: Flight
		void on_actionNew_triggered ();
        void on_actionNewWizard_triggered ();
		void on_actionLaunchMethodPreselection_triggered ();
		void on_actionDepart_triggered ();
		void on_actionLand_triggered ();
		void on_actionTouchngo_triggered ();
		void on_actionEdit_triggered ();
		void on_actionRepeat_triggered ();
		void on_actionDelete_triggered ();
		void on_identifyPlaneAction_triggered ();
		void on_updateFlarmIdAction_triggered ();
		void on_actionDisplayError_triggered ();

		// Menu: View
		//        void on_actionFont_triggered ();
		void on_actionShowVirtualKeyboard_triggered (bool checked);

		// Menu: View - Font
		void on_actionSelectFont_triggered ();
		void on_actionIncrementFontSize_triggered ();
		void on_actionDecrementFontSize_triggered ();

		// Menu: View - Flights
		// on_actionHideFinished_triggered (bool checked) - connected to proxyModel
		// on_actionAlwaysShowExternal_triggered (bool checked) - connected to proxyModel
		// on_actionAlwaysShowErroneous_triggered (bool checked) - connected to proxyModel
		void on_actionResizeColumns_triggered () { ui.flightTable->resizeColumnsToContents (); }


		// Menu: View - Date
		void on_actionSetDisplayDate_triggered ();
		//		void on_actionResetDisplayDateOnNewFlight_triggered ();
		//		void on_actionUseCurrentDateForNewFlights_triggered ();

		// Menu: Statistics
		void on_actionPlaneLogs_triggered ();
		void on_actionPilotLogs_triggered ();
		void on_actionLaunchMethodStatistics_triggered ();
		
		//Menu: Flarm
		void on_actionFlarmPlaneList_triggered ();
		void on_actionFlarmRadar_triggered ();
		void on_flarmNetImportFileAction_triggered ();
		void on_flarmNetImportWebAction_triggered ();
		void on_actionFlarmNetWindow_triggered ();

		// Menu: Database
		void on_actionConnect_triggered ();
		void on_actionDisconnect_triggered ();
		void on_actionEditPlanes_triggered ();
		void on_actionEditPeople_triggered ();
		void on_actionEditLaunchMethods_triggered ();
		void on_actionShowFlights_triggered ();
		void on_actionRefreshAll_triggered ();
        void on_actionSync_triggered();

		// Menu: Debug
		void on_actionSegfault_triggered () { *(int *)NULL = 0; } // For testing the automatic restart mechanism
		//		void on_actionPingServer_triggered ();
		void on_actionTest_triggered ();
		void on_injectFlarmDepartureAction_triggered ();
		void on_injectFlarmLandingAction_triggered ();
		void on_injectFlarmTouchAndGoAction_triggered ();
		void on_lookupPlaneAction_triggered ();
		void on_lookupFlightAction_triggered ();
		void on_showNotificationAction_triggered ();
		void on_decodeFlarmNetFileAction_triggered ();
		void on_encodeFlarmNetFileAction_triggered ();

		// Menu: Help
		void on_actionInfo_triggered ();
		void on_actionNetworkDiagnostics_triggered ();

		// Flight Table
		void on_flightTable_customContextMenuRequested (const QPoint &pos);
		void on_flightTable_doubleClicked (const QModelIndex &index);
		void on_flightTable_departButtonClicked (FlightReference flight);
		void on_flightTable_landButtonClicked (FlightReference flight);

		// Not connected pane
		void on_connectButton_clicked () { on_actionConnect_triggered (); }

		// Flight manipulation
		bool checkPlaneFlying (dbId id, const QString &description);
		bool checkPersonFlying (dbId id, const QString &description);
		void interactiveDepartFlight (dbId id);
		void interactiveLandFlight (dbId id);
		void interactiveLandTowflight (dbId id);
		void interactiveTouchAndGo (dbId id);
		bool nonInteractiveDepartFlight (dbId flightId);
		bool nonInteractiveLandFlight (dbId flightId);
		bool nonInteractiveLandTowflight (dbId flightId);
		bool nonInteractiveTouchAndGo (dbId flightId);
		void departOrLand ();

		// Plugins
		void restartPlugins ();
		void weatherWidget_doubleClicked ();

		// Timers
		void timeTimer_timeout ();

		// Database
		void cacheChanged (DbEvent event);
		void executingQuery (Query);

		// Translation
		void on_changeLanguageAction_triggered ();
		void on_timerBasedLanguageChangeAction_triggered ();

		void logMessage (QString message);

		void flightListChanged ();
		
		// Flarm
		void flarmList_departureDetected  (const QString &flarmId);
		void flarmList_landingDetected    (const QString &flarmId);
		void flarmList_touchAndGoDetected (const QString &flarmId);
		Flight createFlarmFlight (const FlightLookup::Result &lookupResult, const QString &flarmId);
		QString determineFlarmId (dbId flightId, bool ofTowflight);

        //Audio
        void playDepartedSound ();
        void playLandedSound ();

	private:
		DbManager &dbManager;
		Cache &cache;
		Flarm &flarm;

		// TODO move to translation manager?
		QTimer *translationTimer;

		QAction *logAction;

		bool oldLogVisible;


		QDate displayDate;
		
		QPointer<FlightWindow> createFlightWindow;
        QPointer<FlightWizard> createFlightWizard;
		QPointer<FlightWindow> editFlightWindow;

		QList<InfoPlugin *> infoPlugins;
		WeatherWidget *weatherWidget;
		WeatherPlugin *weatherPlugin;
		QPointer<WeatherDialog> weatherDialog;

		// The models involved in displaying the flight list
		EntityList<Flight> *flightList;

		// The context menu is a property of the class rather than a local
		// variable because it has to persist after the method opening it
		// returns.
		QMenu *contextMenu;

		bool databaseActionsEnabled;
		QString databaseOkText;

		/** Whether the font was set explicitly */
		bool fontSet;

		// Flarm
//		TcpDataStream *flarmStream;
//		NmeaDecoder *nmeaDecoder;
//		GpsTracker *gpsTracker;
//		FlarmList *flarmList;
		QString debugFlarmId;
//		bool flarmStreamValid;

		// FIXME remove?
//		Flarm::ConnectionState flarmConnectionState;
		QString flarmConnectionError;

		// TODO: This is only used for Flarm and should be integrated with the
		// Flarm handling (e. g. receive signals from the flight controller).
		// TODO this must be called whenever the respective action is
		// performed, this stinks.
		// TODO should also handle manual edits
		EventTimeTracker departureTracker;
		EventTimeTracker landingTracker;
		EventTimeTracker touchAndGoTracker;

        //Sync
        DbSync* sync;

};

#endif
