#include "PersonEditorPane.h"

#include "src/db/cache/Cache.h"
#include "src/model/Person.h"
#include "src/text.h"
#include "src/config/Settings.h"
#include "src/util/qString.h"
#include "src/i18n/notr.h"
#include "src/gui/PasswordPermission.h"

/*
 * TODO: disallow person name changes; allow merges only
 */

// ******************
// ** Construction **
// ******************

PersonEditorPane::PersonEditorPane (ObjectEditorWindowBase::Mode mode, DbManager &dbManager, QWidget *parent, PersonEditorPaneData *paneData):
	ObjectEditorPane<Person> (mode, dbManager, parent),
	paneData (paneData)
{
	ui.setupUi (this);

	ui.medicalCheckDisabledLabel->setVisible (false);

	setupText ();
	loadData ();

	ui.clubInput->setEditText ("");
	ui.checkMedicalInput->setCurrentItemByItemData (false);

	ui.lastNameInput->setFocus ();

	if (paneData)
	{
		setMedicalValidityDisplayed (paneData->displayMedicalData);
	}
	else
	{
		// Set it to false because we don't want to open a loophole, but we can
		// accept that we have to click the button, or even not be able to view
		// the medical data if the window has been opened in a specific way.
		setMedicalValidityDisplayed (false);
		std::cerr << "No pane data in person editor pane" << std::endl;
	}
}

PersonEditorPane::~PersonEditorPane ()
{

}

template<> ObjectEditorPane<Person> *ObjectEditorPane<Person>::create (ObjectEditorWindowBase::Mode mode, DbManager &dbManager, QWidget *parent, ObjectEditorPaneData *paneData)
{
	return new PersonEditorPane (mode, dbManager, parent, dynamic_cast<PersonEditorPaneData *> (paneData));
}


// ***********
// ** Setup **
// ***********

void PersonEditorPane::setupText ()
{
	if (ui.checkMedicalInput->count ()==0)
	{
		// The combo box is empty - add the items
		ui.checkMedicalInput->addItem (tr ("Yes"), true );
		ui.checkMedicalInput->addItem (tr ("No" ), false);
	}
	else
	{
		// The combo box is already filled -
		// update the item texts
		ui.checkMedicalInput->setItemText (0, tr ("Yes"));
		ui.checkMedicalInput->setItemText (1, tr ("No"));
	}
}

void PersonEditorPane::loadData ()
{
	// Clubs
	ui.clubInput->addItem ("");
	ui.clubInput->addItems (cache.getClubs ());
}

void PersonEditorPane::setNameObject (const Person &nameObject)
{
	if (!isBlank (nameObject.lastName))
	{
		ui.lastNameInput->setText (nameObject.lastName);
		ui.lastNameInput->setEnabled (false);
	}

	if (!isBlank (nameObject.firstName))
	{
		ui.firstNameInput->setText (nameObject.firstName);
		ui.firstNameInput->setEnabled (false);
	}
}

void PersonEditorPane::setMedicalValidityDisplayed (bool displayed)
{
	ui.displayMedicalValidityButton  ->setVisible (!displayed);
	ui.medicalValidityUnknownCheckbox->setVisible ( displayed);
	ui.medicalValidityInput          ->setVisible ( displayed);
}

// ****************
// ** GUI events **
// ****************

void PersonEditorPane::on_medicalValidityUnknownCheckbox_toggled ()
{
	ui.medicalValidityInput->setEnabled (!ui.medicalValidityUnknownCheckbox->isChecked ());
}

void PersonEditorPane::on_checkMedicalInput_currentIndexChanged ()
{
	ui.medicalCheckDisabledLabel->setVisible (
		ui.checkMedicalInput->currentItemData ().toBool ()==true &&
		!Settings::instance ().checkMedicals
	);
}

void PersonEditorPane::on_displayMedicalValidityButton_clicked ()
{
	if (!paneData) return;

	if (paneData->viewMedicalDataPermission->permit (this))
	{
		setMedicalValidityDisplayed (true);
		paneData->displayMedicalData=true;
	}
}


// ******************************
// ** ObjectEditorPane methods **
// ******************************

void PersonEditorPane::objectToFields (const Person &person)
{
	ui.lastNameInput->setText (person.lastName);
	ui.firstNameInput->setText (person.firstName);
	ui.clubInput->setEditText (person.club);
	ui.commentsInput->setText (person.comments);
	ui.checkMedicalInput->setCurrentItemByItemData (person.checkMedical);
	if (person.medicalValidity.isValid ())
	{
		ui.medicalValidityInput->setDate (person.medicalValidity);
		ui.medicalValidityUnknownCheckbox->setChecked (false);
	}
	else
	{
		ui.medicalValidityInput->setDate (QDate::currentDate ());
		ui.medicalValidityUnknownCheckbox->setChecked (true);
	}
	ui.clubIdInput->setText (person.clubId);
}

QDate PersonEditorPane::getEffectiveMedicalValidity ()
{
	if (ui.medicalValidityUnknownCheckbox->isChecked ())
		return QDate ();
	else
		return ui.medicalValidityInput->date ();
}

void PersonEditorPane::fieldsToObject (Person &person, bool performChecks)
{
	// Require "change medical data" permission if the medical data changed
	Person originalPerson=getOriginalObject ();
	if (performChecks && getEffectiveMedicalValidity ()!=originalPerson.medicalValidity)
	{
		if (paneData && !paneData->changeMedicalDataPermission->permit (this))
		{
			throw AbortedException ();
		}
	}

	person.lastName             =ui.lastNameInput       ->text ().simplified ();
	person.firstName            =ui.firstNameInput      ->text ().simplified ();
	person.club                 =ui.clubInput           ->currentText ().simplified ();
	person.comments             =ui.commentsInput       ->text ().simplified ();
	person.checkMedical         =ui.checkMedicalInput   ->currentItemData ().toBool ();
	person.medicalValidity      =getEffectiveMedicalValidity ();
	person.clubId               =ui.clubIdInput         ->text ();

	if (!performChecks) return;

	// Error checks

	if (isNone (person.lastName))
		errorCheck (tr ("Last name not specified."),
			ui.lastNameInput);

	if (isNone (person.firstName))
		errorCheck (tr ("First name not specified."),
			ui.firstNameInput);
}

// **********
// ** Misc **
// **********

void PersonEditorPane::changeEvent (QEvent *event)
{
	if (event->type () == QEvent::LanguageChange)
	{
		ui.retranslateUi (this);
		setupText ();
	}
	else
		ObjectEditorPane<Person>::changeEvent (event);
}
