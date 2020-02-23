#ifndef FLIGHTWINDOW_H_
#define FLIGHTWINDOW_H_

/*
 * Remake:
 *   - TODO Completer: complete to proper case; drop down and inline completion
 *     Failing complete-to-proper-case: set the proper case on editing
 *     finished.
 *   - TODO Invalid launch method preserved.
 *     - in flightToFields, create and set an "unknown" entry and store the
 *       original
 *     - in determineFlight, if the entry is unknown, use the original value
 *     - in updateErrors, addObject as non-error
 *     - in checkFlightPhase1, exclude from error check
 *   - TODO Make sure modeless dialogs work (and control returns immediately)
 *   - TODO Handle simultaneous flight editing and manipulation from main window
 *   - TODO Do we need to check the desktop size?
 *     (QApplication::desktop ())->availableGeometry ().height ()
 *   - TODO Leaving towflight with no end time should be OK
 *   - TODO Create flight on day other than today => no "now"/"later" buttons
 *   - TODO Towflight mode on non-airtow: set how? Different from stable.
 *   - TODO Partial names working? Especially: error display
 *   - TODO in updateErrors, set the focus to the uppermost error widget, not the
 *     first one found
 *   - TODO in updateErrors, the highlighted labels for person errors may be last
 *     name, first name, or person
 *   - TODO make time zone safe
 *   - TODO determinePerson: preselection based on the club of the plane or student
 *     (if person unique within club); display text "Club of the plane" instead
 *     of "Matching club"
 *   - TODO Clean up method declarations
 *   - TODO When adding a new plane/person on flight creation, immediately get it
 *     from the database and show the registration/correct name (it might have
 *     been changed), even if on a subsequent choice the user aborts
 *   - TODO When there is only one last name or only one first name in the
 *     database, it is always filled in when tabbing over the person fields,
 *     even if the other field is empty
 *
 *
 * Further improvements:
 *   - improve buttons. For example, when editing a flight and removing the
 *     "landed" flag, can there be a "land now" button? That way, there would
 *     be no difference between editing and creating a flight, as long as the
 *     fields are equal. Does it matter that the flight already has values for
 *     these fields when editing?
 *     One implication is that the mode is not sufficient to determine whether
 *     the button should be "depart now" or "land now", respectively later.
 *     Note that create mode *is* different from edit mode, for example the
 *     automatic selection of launch method/departure locations.
 *   - Add a calendar to the date input
 *   - Add a button to set the departure/landing time to the current time
 *   - Add option to allow, but not require the towpilot
 *   - The sizeHint for the timeEdits seems to be too low by 1 pixel. This is
 *     solved by a hack in showEvent. It worked in the old version of this
 *     dialog, where the row spacing was set instead of the label's height.
 *   - "Depart now" should set the departure time, even if there are errors.
 *   - On flight mode change, change the locations only if they haven't been
 *     manually edited.
 *   - Read-only mode (modeDisplay)
 *   - If the plane is unknown and the user aborts, set the focus to the
 *     appropriate registration input
 *   - For the error checking on accepting a flight, use the error checking
 *     method provided by the Flight class (probably not all errors can be
 *     checked that way).
 *   - On editing, addObject a currently-flying test, at least for the case where
 *     a departure time has been added
 *   - Move more conditions to the Plane, for example canDoSelfLaunch, ...
 *   - Launch method "Other": comment should not be empty (warning if it is)
 *   - Warning, when date changed
 *   - Allow new flights only for current date and displayed date
 *   - Preserve unknown plane/person (e. g. hide input field, addObject "Unknown"
 *     label and "Change" button)
 *
 * Fixed bugs:
 *   - Edit prepared coming flight, activate "Landed" => landing location set to
 *     departure value.
 *   - Edit flight, change plane to non-existent => error "no plane specified"
 *   - Wrong copilot set on repeating flights
 *
 *
 *
 * Action Mode    Times not | Buttons[1]             Time inputs   Comments
 *                automatic |
 * -------------------------+------------------------------------------------------------
 * New    local   -         | Depart now/later       Auto/Auto     Regular local
 * New    local   Departure | Land now/later or OK   Auto/Auto     Amend departed
 * New    local   Landing   | OK                     Auto/Auto     Error
 * New    local   Both      | OK                     Auto/Auto     Amend
 *                          |
 * New    coming  -         | Land now/later         -/Auto        Regular coming
 * New    coming  Landing   | OK                     -/Auto        Amending coming
 *                          |
 * New    leaving -         | Depart now/later       Auto/-        Regular leaving
 * New    leaving Departure | OK                     Auto/-        Amending leaving
 *
 * [2] Now/later only if date==today; determined by isNowActionPossible
 *
 * Action Mode    Flags     | Buttons                Time inputs     Comments
 * -------------------------+-------------------------------------------------------------
 * Edit   local   -         | OK                     Departed/Landed Prepared
 * Edit   local   Departed  | OK                     Departed/Landed Flying
 * Edit   local   Landed    | OK                     Departed/Landed Error
 * Edit   local   Both      | OK                     Departed/Landed Landed
 *                          |
 * Edit   coming  -         | OK                     Landed          Prepared coming
 * Edit   coming  Landed    | OK                     Landed          Landed coming
 *                          |
 * Edit   leaving -         | OK                     Departed        Prepared leaving
 * Edit   leaving Departed  | OK                     Departed        Departed leaving
 */

#include <QDate>
#include <QList>

#include "ui_FlightWindow.h"

#include "src/gui/SkDialog.h"
#include "src/db/dbId.h"
#include "src/model/LaunchMethod.h" // TODO remove dependency
#include "src/model/Flight.h" // Required for Flight::Mode and Flight::Type
#include "src/config/Settings.h" // TODO remove dependency
#include "src/db/DbManager.h" // Required for DbManager::State
#include "src/db/event/DbEvent.h"

class QPushButton;

class DbManager;
class Person;
class Cache;

// We want to use a switch so the compiler can warn us if we didn't handle a
// value. We still provide a default value (outside of the switch statement!)
// instead of throwing an assertion. We use a macro to be able to write in in
// on line and still use a switch.
#define EDITOR_MODE_RETURN(createValue, editValue, defaultValue) do { switch (mode) { \
	case modeCreate: return createValue; \
	case modeEdit: return editValue; \
	} return defaultValue; } while (0)

class FlightWindow: public SkDialog<Ui::FlightWindowClass>
{
    Q_OBJECT

	public:
		// *** Types
		enum Mode { modeCreate, modeEdit };

		class AbortedException: public std::exception {};


		// *** Construction
                FlightWindow (QWidget *parent, FlightWindow::Mode mode, DbManager &manager, Qt::WindowFlags flags=0);
		~FlightWindow ();

		// *** Setup
		void fillLists ();
		void fillData ();
		void updateLists ();

		// *** Invocation
		static FlightWindow *createFlight (QWidget *parent, DbManager &manager, QDate date, dbId preselectedLaunchMethod);
		static FlightWindow *repeatFlight (QWidget *parent, DbManager &manager, const Flight &original, QDate date, dbId preselectedLaunchMethod);
		static FlightWindow *editFlight   (QWidget *parent, DbManager &manager, Flight &flight);

		// *** Properties
		dbId getEditedId () { return originalFlightId; }

	signals:
		void flightDeparted (dbId id);
		void flightLanded   (dbId id);

	protected:
		void setupTitle ();

		// Input field data
		int fillNames (QStringList (Cache::*fullListMethod)(), QStringList (Cache::*partialListMethod)(const QString &), QComboBox *target, const QString &otherName, bool preserveTarget);
		dbId fillLastNames  (bool active, QComboBox *target, const QString &firstName, bool preserveTarget);
		dbId fillFirstNames  (bool active, QComboBox *target, const QString &lastName, bool preserveTarget);


		// *** Input values
		// The values returned by this function are only meaningful if the
		// corresponding field is active. This can be determined by the
		// isXActive methods.
		QString    getCurrentRegistration                 () { return ui.registrationInput->currentText (); }
		Flight::Type getCurrentFlightType                 () { return (Flight::Type)ui.flightTypeInput->currentItemData ().toInt (); }
		//
		QString    getCurrentPilotLastName                () { return ui.pilotLastNameInput->currentText (); }
		QString    getCurrentPilotFirstName               () { return ui.pilotFirstNameInput->currentText (); }
		QString    getCurrentCopilotLastName              () { return ui.copilotLastNameInput->currentText (); }
		QString    getCurrentCopilotFirstName             () { return ui.copilotFirstNameInput->currentText (); }
        //
        int        getCurrentNumCrew                      () { return ui.numCrewInput->value(); }
        int        getCurrentNumPax                       () { return ui.numPaxInput->value(); }
		//
		Flight::Mode getCurrentFlightMode                 () { return (Flight::Mode)ui.flightModeInput->currentItemData ().toInt(); }
		dbId      getCurrentLaunchMethodId                () { return ui.launchMethodInput->currentItemData ().toInt(); }
		//
		QString    getCurrentTowplaneRegistration         () { return ui.towplaneRegistrationInput->currentText (); }
		QString    getCurrentTowpilotLastName             () { return ui.towpilotLastNameInput->currentText(); }
		QString    getCurrentTowpilotFirstName            () { return ui.towpilotFirstNameInput->currentText (); }
		Flight::Mode getCurrentTowflightMode              () { return (Flight::Mode)ui.towflightModeInput->currentItemData ().toInt(); }
		//
		QTime      getCurrentDepartureTime                () { return ui.departureTimeInput->time (); }
		QTime      getCurrentLandingTime                  () { return ui.landingTimeInput->time (); }
		QTime      getCurrentTowflightLandingTime         () { return ui.towflightLandingTimeInput->time (); }
		//
		QString    getCurrentDepartureLocation            () { return ui.departureLocationInput->currentText(); }
		QString    getCurrentLandingLocation              () { return ui.landingLocationInput->currentText (); }
		QString    getCurrentTowflightLandingLocation     () { return ui.towflightLandingLocationInput->currentText(); }
		int        getCurrentNumLandings                  () { return ui.numLandingsInput->value (); }
		//
		QString    getCurrentComment                      () { return ui.commentInput->text(); }
		QString    getCurrentAccountingNote               () { return ui.accountingNoteInput->currentText(); }
		QDate      getCurrentDate                         () { return ui.dateInput->date (); }

		// *** Input value frontends
		/*
		 * The conditions for whether a field is active or not have to be
		 * checked in several places, at least for field visiblilty and for
		 * accepting the flight. In order to avoid code duplication, there are
		 * methods to determine wheter the fields are active for the current
		 * input values.
		 * As several field visiblities depend on the same input value (e. g.
		 * departure time and towplane depend on the launch method), values will be
		 * read from the fields and/or data storage several times. This should
		 * not be a problem, but could be solved by allowing the caller to pass
		 * a parameter which will be used instead of calling getCurrentX.
		 * Note that the copilot input is active even for single seated planes
		 * because that information may be unreliable. There is a warning for
		 * that, though.
		 */

		/**
		 * Gets the currently selected launch method from the database.
		 *
		 * @return the currently selected launch method
		 * @throw Cache::NotFoundException if there is no such launch
		 *        method, or none is selected
		 */
		LaunchMethod getCurrentLaunchMethod ();
		bool isCurrentLaunchMethodValid () { return idValid (getCurrentLaunchMethodId ()); }

		bool currentDepartsHere  () {       return isFlightModeActive    () && Flight::departsHere (getCurrentFlightMode ()); }
		bool currentLandsHere    () {       return isFlightModeActive    () && Flight::landsHere (getCurrentFlightMode ()); }
		bool currentTowLandsHere () {       return isTowflightModeActive () && Flight::landsHere (getCurrentTowflightMode ()); }
		bool currentIsAirtow     ();



		// *** Field active-ness
		bool isRegistrationActive                 () { return true; }
		bool isPlaneTypeActive                    () { return true; }
		bool isFlightTypeActive                   () { return true; }
		//
        bool isPilotActive                        () { return !Settings::instance().anonymousMode; }
        bool isCopilotActive                      () { return !Settings::instance().anonymousMode && Flight::typeCopilotRecorded (getCurrentFlightType ()); } // Does not depend on plane, see comments above
		//
		bool isFlightModeActive                   () { return true; }
		bool isLaunchMethodActive                 () { return currentDepartsHere (); }
		//
		// No exception thrown because if the launch method is not valid,
		bool isTowplaneRegistrationActive         ();
		bool isTowplaneTypeActive                 () { return currentIsAirtow (); }
		bool isTowpilotActive                     () { return currentIsAirtow () && Settings::instance ().recordTowpilot; }
		bool isTowflightModeActive                () { return currentIsAirtow (); }
		//
		bool isDepartureActive                    () { return currentDepartsHere (); }
		bool isDepartureTimeActive                () { return isDepartureActive () && getTimeFieldActive (ui.departureTimeCheckbox->isChecked ()); }
		bool isLandingActive                      () { return currentLandsHere (); }
		bool isLandingTimeActive                  () { return isLandingActive () && getTimeFieldActive (ui.landingTimeCheckbox->isChecked ()); }
		bool isTowflightLandingActive             () { return currentIsAirtow (); } // Even if mode==leaving - it's the tow end time in that case.
		bool isTowflightLandingTimeActive         () { return isTowflightLandingActive () && getTimeFieldActive (ui.towflightLandingTimeCheckbox->isChecked ()); }
		//
		bool isDepartureLocationActive            () { return true; }
		bool isLandingLocationActive              () { return true; }
		bool isTowflightLandingLocationActive     () { return currentIsAirtow (); }
		bool isNumLandingsActive                  () { return true; } // (touch'n'gos are possible before leaving)
		//
		bool isCommentActive                      () { return true; }
		bool isAccountingNodeActive               () { return true; }
		bool isDateActive                         () { return true; }
		//
		// Error list is not active in create mode because there are invalid intermediate states during flight input
		bool isErrorListActive                    () { EDITOR_MODE_RETURN (false, true, false); }


		// Flight reading/writing
		void personToFields (dbId id, SkComboBox *lastNameInput, SkComboBox *firstNameInput, QString incompleteLastName, QString incompleteFirstName);
		void planeToFields (dbId id, SkComboBox *registrationInput, SkLabel *typeLabel);
		void flightToFields (const Flight &flight, bool repeat, dbId preselectedLaunchMethod=invalidId);

                Flight determineFlight (bool departNow) noexcept(false);
                Flight determineFlightBasic ();
                void determineFlightPlanes (Flight &flight) noexcept(false);
		void determineFlightPeople (Flight &flight, const LaunchMethod *launchMethod) throw (AbortedException);
		dbId determinePlane (QString registration, QString description, QWidget *widget) throw (AbortedException);
		dbId determineAndEnterPlane (QString registration, QString description, SkComboBox *registrationInput, SkLabel *typeLabel) throw (AbortedException);
		dbId determinePerson (bool active, QString lastName, QString firstName, QString description, bool required, QString &incompleteLastName, QString &incompleteFirstName, dbId originalId, QWidget *widget) throw (AbortedException);
		dbId determineAndEnterPerson (bool active, QString lastName, QString firstName, QString description, bool required, QString &incompleteLastName, QString &incompleteFirstName, dbId originalId, SkComboBox *lastNameWidget, SkComboBox *firstNameWidget) throw (AbortedException);
		dbId createNewPerson (QString lastName, QString firstName) throw (AbortedException);
		void checkFlightPhase1 (const Flight &flight, bool departNow) throw (AbortedException);
		void checkFlightPhase2 (const Flight &flight, bool departNow, const Plane *plane, const Plane *towplane, const LaunchMethod *launchMethod) throw (AbortedException);
		void checkFlightPhase3 (const Flight &flight, bool departNow, const Plane *plane, const Person *pilot, const Person *copilot, const Person *towpilot) throw (AbortedException);
		void checkMedical (const Person *person, const QString &ofThePersonText);
		void errorCheck (const QString &problem, QWidget *widget) throw (AbortedException);

		bool checkBuffer ();

		bool writeToDatabase (Flight &flight);
		bool updateFlarmId (const Flight & flight);

		// *** Input field setup
		void  enableWidget (QWidget *widget, bool  enabled);
		void disableWidget (QWidget *widget, bool disabled);
		void  enableWidgets (QWidget *widget0, QWidget *widget1, bool  enabled);
		void disableWidgets (QWidget *widget0, QWidget *widget1, bool disabled);

		void updateSetupVisibility ();
		void updateSetupLabels ();
		void updateSetupButtons ();
		void updateSetup ();

		virtual void showEvent (QShowEvent *event);

	protected slots:
		virtual void settingsChanged ();
		virtual void databaseStateChanged (DbManager::State state);
		virtual void cacheChanged (DbEvent event);

	private slots:
		/*
		 * Notes on change events:
		 *   - Things to be done on a value change event:
		 *       - Potentially change other fields' values
		 *       - Update field states
		 *       - Error checks
		 *   - Errors may depend on multiple inputs and multiple errors may depend on
		 *     the same input, so all errors are checked after each change
		 *   - No data that requires user interaction (e. g. person selection if there
		 *     are multiple people with the same name) is used in error checks (this
		 *     does not apply to the error checks done after accepting).
		 *   - All state updates are performed for all fields which may lead to any
		 *     state change (see "Notes on field state updates")
		 *   - Sometimes, a plane or person may be retrieved from the cache by
		 *     these function and then again by updateErrors. As cache does not
		 *     access the database, this should not be a performance problem.
		 *   - A change can lead to several other fields being changed (e. g. mode ->
		 *     departure, landing location)
		 *   - We don't want error checks/updates after every change
		 *   - The user change slot calls xChanged and updateSetup and updateErrors
		 */

		// *** Input field value change events

		// Use slots that are only called on user input if available (see above)

		void on_registrationInput_editingFinished (const QString &text) { registrationChanged (text);  updateSetup (); updateErrors (); }
		void on_flightTypeInput_activated         (int index)           { flightTypeChanged   (index); updateSetup (); updateErrors (); }
		//
		void on_pilotLastNameInput_editingFinished    (const QString &text) { pilotLastNameChanged    (text); updateSetup (); updateErrors (); }
		void on_pilotFirstNameInput_editingFinished   (const QString &text) { pilotFirstNameChanged   (text); updateSetup (); updateErrors (); }
		void on_copilotLastNameInput_editingFinished  (const QString &text) { copilotLastNameChanged  (text); updateSetup (); updateErrors (); }
		void on_copilotFirstNameInput_editingFinished (const QString &text) { copilotFirstNameChanged (text); updateSetup (); updateErrors (); }
		//
		void on_flightModeInput_activated   (int index) { flightModeChanged   (index); updateSetup (); updateErrors (); }
		void on_launchMethodInput_activated (int index) { launchMethodChanged (index); updateSetup (); updateErrors (); }
		//
		void on_towplaneRegistrationInput_editingFinished (const QString &text) { towplaneRegistrationChanged (text);  updateSetup (); updateErrors (); }
		void on_towpilotLastNameInput_editingFinished     (const QString &text) { towpilotLastNameChanged     (text);  updateSetup (); updateErrors (); }
		void on_towpilotFirstNameInput_editingFinished    (const QString &text) { towpilotFirstNameChanged    (text);  updateSetup (); updateErrors (); }
		void on_towflightModeInput_activated              (int index)           { towflightModeChanged        (index); updateSetup (); updateErrors (); }
		//
		void on_departureTimeCheckbox_clicked         (bool checked)      { departureTimeCheckboxChanged           (checked); updateSetup (); updateErrors (); ui.departureTimeInput->setFocus (); }
		void on_departureTimeInput_timeChanged        (const QTime &time) { departureTimeChanged                   (time);    updateSetup (); updateErrors (); } // This widget does not have a user-edit-only signal
		void on_landingTimeCheckbox_clicked           (bool checked)      { landingTimeCheckboxChanged          (checked); updateSetup (); updateErrors (); ui.landingTimeInput->setFocus (); }
		void on_landingTimeInput_timeChanged          (const QTime &time) { landingTimeChanged                  (time);    updateSetup (); updateErrors (); } // This widget does not have a user-edit-only signal
		void on_towflightLandingTimeCheckbox_clicked  (bool checked)      { towflightLandingTimeCheckboxChanged (checked); updateSetup (); updateErrors (); ui.towflightLandingTimeInput->setFocus ();}
		void on_towflightLandingTimeInput_timeChanged (const QTime &time) { towflightLandingTimeChanged         (time);    updateSetup (); updateErrors (); } // This widget does not have a user-edit-only signal
		//
		void on_departureLocationInput_editingFinished        (const QString &text) { departureLocationChanged        (text ); updateSetup (); updateErrors (); }
		void on_landingLocationInput_editingFinished          (const QString &text) { landingLocationChanged          (text ); updateSetup (); updateErrors (); }
		void on_towflightLandingLocationInput_editingFinished (const QString &text) { towflightLandingLocationChanged (text ); updateSetup (); updateErrors (); }
		void on_numLandingsInput_valueChanged                 (int value)           { numLandingsChanged              (value); updateSetup (); updateErrors (); } // This widget does not have a user-edit-only signal
		//
		void on_commentInput_editingFinished        ()                    { commentChanged        ();     updateSetup (); updateErrors (); }
		void on_accountingNoteInput_editingFinished (const QString &text) { accountingNoteChanged (text); updateSetup (); updateErrors (); }
		void on_dateInput_dateChanged               (const QDate   &date) { dateChanged           (date); updateSetup (); updateErrors (); }


		// *** Value change handlers
		void registrationChanged (const QString &text);
		void flightTypeChanged (int index) { (void)index; }
		//
		void pilotLastNameChanged    (const QString &text) { selectedPilot  =fillFirstNames (isPilotActive (), ui.pilotFirstNameInput  , text, false); }
		void pilotFirstNameChanged   (const QString &text) { selectedPilot  =fillLastNames  (isPilotActive (), ui.pilotLastNameInput   , text, false); }
		void copilotLastNameChanged  (const QString &text) { selectedCopilot=fillFirstNames (isCopilotActive (), ui.copilotFirstNameInput, text, false); }
		void copilotFirstNameChanged (const QString &text) { selectedCopilot=fillLastNames  (isCopilotActive (), ui.copilotLastNameInput , text, false); }
		//
		void flightModeChanged   (int index);
		void launchMethodChanged (int index);
		//
		void towplaneRegistrationChanged (const QString &text);
		void towpilotLastNameChanged     (const QString &text) { selectedTowpilot=fillFirstNames (isTowpilotActive (), ui.towpilotFirstNameInput, text, false); }
		void towpilotFirstNameChanged    (const QString &text) { selectedTowpilot=fillLastNames  (isTowpilotActive (), ui.towpilotLastNameInput , text, false); }
		void towflightModeChanged        (int index) { (void)index; }
		//
		void departureTimeCheckboxChanged        (bool checked) { (void)checked; }
		void departureTimeChanged                (const QTime &time) { (void)time; }
		void landingTimeCheckboxChanged          (bool checked);
		void landingTimeChanged                  (const QTime &time) { (void)time; }
		void towflightLandingTimeCheckboxChanged (bool checked);
		void towflightLandingTimeChanged         (const QTime &time) { (void)time; }
		//
		void departureLocationChanged         (const QString &text) { (void)text; }
		void landingLocationChanged           (const QString &text) { (void)text; }
		void towflightLandingLocationChanged  (const QString &text) { (void)text; }
		void numLandingsChanged               (int value)           { (void)value; }
		//
		void commentChanged        ()                    { }
		void accountingNoteChanged (const QString &text) { (void)text; }
		void dateChanged           (const QDate   &date) { (void)date; }


		void languageChanged ();

		// *** Button events
		void okButton_clicked    (); // not automatically connected
		void nowButton_clicked   (); // not automatically connected
		void laterButton_clicked (); // not automatically connected


	private:
		QMultiMap<QWidget *, SkLabel *> widgetLabelMap;

		QPushButton *nowButton;
		QPushButton *laterButton;

		DbManager &manager;
		Cache &cache;
		const FlightWindow::Mode mode;

		void updateErrors (bool setFocus=false);
		QWidget *getErrorWidget (Flight::Error error);

		// Flarm
		void identifyPlane (const Flight &flight);

		// In create mode, we have "Automatic" checkboxes which deativate the
		// input fields, so they are active if the checkbox is not checked. In edit
		// mode, we have "Departed"/"Landed" checkboxes which activate the input
		// field.
		bool getTimeFieldActive (bool checkboxValue) { EDITOR_MODE_RETURN (!checkboxValue, checkboxValue, true); }
		bool getTimeFieldCheckboxValue (bool active) { EDITOR_MODE_RETURN (!active, active, true); }

		bool isNowActionPossible ();

		bool labelHeightsSet;

		// In addition to the data in the fields, we need to store some more
		// values:
		//   - the ID of the flight
		//   - the ID of the planes
		//   - the ID of the people
		// The flight ID is required for updating the flight in the database
		// after editing.
		// The ID of the planes and people is required for error checks. They
		// are updated on every change of the corresponding input fields. If a
		// person is non-unique, the corresponding selectedX is set to invalid.
		// Methods reading this value may want to look at the input fields to
		// distinguish between unknown and unspecified people.
		// Anyway, on accepting the flight, the correct values have to be
		// determined, querying the user if necessary.
		// The ID of the people is also required for preslection if there are
		// multiple people with the same name.
		// Note that a simpler implementation might be to use these values for
		// preselection only (originalX instead of selectedX) and have
		// determineFlightBasic read the values from the database if they are
		// uniqe.
		Flight originalFlight; // Only meaningful after flightToFields has been called, e. g. when editing a flight
		dbId originalFlightId;
		dbId selectedPlane, selectedTowplane;
		dbId selectedPilot, selectedCopilot, selectedTowpilot;
};

#endif

