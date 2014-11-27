#include "PlaneEditorPane.h"

#include <QMessageBox>

#include "src/text.h"
#include "src/db/cache/Cache.h"
#include "src/model/Plane.h"
#include "src/util/qString.h"
#include "src/i18n/notr.h"
#include "src/gui/windows/input/ChoiceDialog.h"
#include "src/flarm/algorithms/FlarmIdCheck.h"

/*
 * Improvements:
 *   - cursor to end of registration input on create
 */

/*
 * TODO remake:
 *   - autocompletion type consistent in all editors
 *   - disallow plane registration changes
 */



// ******************
// ** Construction **
// ******************

PlaneEditorPane::PlaneEditorPane (ObjectEditorWindowBase::Mode mode, DbManager &dbManager, QWidget *parent, PlaneEditorPaneData *paneData):
	ObjectEditorPane<Plane> (mode, dbManager, parent),
	paneData (paneData)
{
	ui.setupUi(this);

	prepareText ();
	setupText ();
	loadData ();

	ui.categoryInput->setCurrentItemByItemData (Plane::categoryNone);
	ui.typeInput->setEditText ("");
	ui.clubInput->setEditText ("");


	ui.registrationInput->setText (Plane::defaultRegistrationPrefix ());
	ui.registrationInput->setFocus ();
//	ui.registrationInput->setCursorPosition (ui.registrationInput->text ().length ());
//	ui.registrationInput->end (false);

	if (paneData)
	{
		ui.flarmIdInput     ->setEnabled (!paneData->flarmIdReadOnly     );
		ui.registrationInput->setEnabled (!paneData->registrationReadOnly);
	}
	else
	{
		ui.flarmIdInput->setEnabled (true);
	}
}

PlaneEditorPane::~PlaneEditorPane()
{

}

template<> ObjectEditorPane<Plane> *ObjectEditorPane<Plane>::create (ObjectEditorWindowBase::Mode mode, DbManager &dbManager, QWidget *parent, ObjectEditorPaneData *paneData)
{
	return new PlaneEditorPane (mode, dbManager, parent, dynamic_cast<PlaneEditorPaneData *> (paneData));
}


// ***********
// ** Setup **
// ***********

void PlaneEditorPane::prepareText ()
{
	// Categories
	ui.categoryInput->addItem ("", Plane::categoryNone);
	const QList<Plane::Category> categories=Plane::listCategories (false);
	for (int i=0, n=categories.size (); i<n; ++i)
		ui.categoryInput->addItem ("", categories.at (i));
}

void PlaneEditorPane::setupText ()
{
	// Categories
	// The text for category none, at index 0, is left at "".
	for (int i=1, n=ui.categoryInput->count (); i<n; ++i)
	{
		Plane::Category category=(Plane::Category)ui.categoryInput->itemData (i).toInt ();
		QString text=firstToUpper (Plane::categoryText (category));
		ui.categoryInput->setItemText (i, text);
	}
}

void PlaneEditorPane::loadData ()
{
	// Types
	ui.typeInput->addItem ("");
	ui.typeInput->addItems (cache.getPlaneTypes ());

	// Clubs
	ui.clubInput->addItem ("");
	ui.clubInput->addItems (cache.getClubs ());
}

void PlaneEditorPane::setNameObject (const Plane &nameObject)
{
	if (!isBlank (nameObject.registration))
	{
		ui.registrationInput->setText (nameObject.registration);
		ui.registrationInput->setEnabled (false);
		on_registrationInput_editingFinished ();
	}
}

// ****************
// ** GUI events **
// ****************

void PlaneEditorPane::on_registrationInput_editingFinished ()
{
	if (ui.categoryInput->currentItemData ()==Plane::categoryNone)
	{
		Plane::Category category=Plane::categoryFromRegistration (ui.registrationInput->text ());
		if (category!=Plane::categoryNone)
			ui.categoryInput->setCurrentItemByItemData (category);
	}
}


// ******************************
// ** ObjectEditorPane methods **
// ******************************

void PlaneEditorPane::objectToFields (const Plane &plane)
{
	ui.registrationInput->setText (plane.registration);
	ui.callsignInput->setText (plane.callsign);
	ui.categoryInput->setCurrentItemByItemData (plane.category);
	ui.typeInput->setEditText (plane.type);
	ui.clubInput->setEditText (plane.club);
	ui.seatsInput->setValue (plane.numSeats);
	ui.flarmIdInput->setText (plane.flarmId);
	ui.commentsInput->setText (plane.comments);
}

void PlaneEditorPane::fieldsToObject (Plane &plane, bool performChecks)
{
	if (!performChecks) return;

	// TODO: checks go here; throw AbortedException if aborted

	plane.registration=ui.registrationInput->text ().simplified ();
	plane.callsign=ui.callsignInput->text ().simplified ();
	plane.category=(Plane::Category)ui.categoryInput->currentItemData ().toInt ();

	plane.type=ui.typeInput->currentText ().simplified ();
	plane.club=ui.clubInput->currentText ().simplified ();
	plane.numSeats=ui.seatsInput->value ();
	plane.flarmId=ui.flarmIdInput->text ().simplified ();
	plane.comments=ui.commentsInput->text ().simplified ();

	// Error checks
	if (mode==ObjectEditorWindowBase::modeCreate && idValid (cache.getPlaneIdByRegistration (plane.registration)))
	{
		QString message=tr ("A plane with the registration %1 already exists.").arg (plane.registration);
		QMessageBox::critical (this, tr ("Plane already exists"), message, QMessageBox::Ok, QMessageBox::NoButton);
		throw AbortedException ();
	}

	if (isNone (plane.registration))
		errorCheck (tr ("Registration not specified."),
			ui.registrationInput);

	if (plane.category==Plane::categoryNone)
		errorCheck (tr ("Category not specified."),
			ui.categoryInput);

	Plane::Category registrationCategory=Plane::categoryFromRegistration (plane.registration);
	if (registrationCategory!=Plane::categoryNone && plane.category!=Plane::categoryNone && plane.category!=registrationCategory)
		errorCheck (tr ("The selected category does not match the registration."),
			ui.categoryInput);

	if (isNone (plane.type))
		errorCheck (tr ("Model not specified."),
			ui.typeInput);

	if (plane.numSeats<0)
		errorCheck (tr ("Number of seats not specified."),
			ui.seatsInput);

	if (plane.numSeats==0)
		errorCheck (tr ("0 seats specified."),
			ui.seatsInput);

	int maxSeats=Plane::categoryMaxSeats (plane.category);
	if (maxSeats>=0 && plane.numSeats>maxSeats)
		errorCheck (tr ("To many seats specified for the selected category."),
			ui.seatsInput);

	FlarmIdCheck flarmIdCheck (dbManager, this);
	flarmIdCheck.interactiveCheck (plane.flarmId, getOriginalObject ().getId (),
		getOriginalObject ().flarmId);
	bool checkResult=flarmIdCheck.interactiveApply (&plane.flarmId);
	if (!checkResult)
		throw AbortedException ();
}

// **********
// ** Misc **
// **********

void PlaneEditorPane::changeEvent (QEvent *event)
{
	if (event->type () == QEvent::LanguageChange)
	{
		ui.retranslateUi (this);
		setupText ();
	}
	else
		ObjectEditorPane<Plane>::changeEvent (event);
}
