#include "LaunchMethodEditorPane.h"

#include "src/model/LaunchMethod.h"
#include "src/db/cache/Cache.h"
#include "src/util/qString.h"
#include "src/i18n/notr.h"
#include "src/text.h"

// ******************
// ** Construction **
// ******************

LaunchMethodEditorPane::LaunchMethodEditorPane (ObjectEditorWindowBase::Mode mode, DbManager &dbManager, QWidget *parent):
	ObjectEditorPane<LaunchMethod> (mode, dbManager, parent)
{
	ui.setupUi(this);

	setupText ();
	loadData ();

	ui.typeInput->setCurrentItemByItemData (LaunchMethod::typeOther);
	ui.personRequiredInput->setCurrentItemByItemData (true);
	ui.towplaneRegistrationInput->setEnabled (ui.specificTowplaneInput->isChecked ());

	ui.nameInput->setFocus ();
}

LaunchMethodEditorPane::~LaunchMethodEditorPane()
{

}

template<> ObjectEditorPane<LaunchMethod> *ObjectEditorPane<LaunchMethod>::create (ObjectEditorWindowBase::Mode mode, DbManager &dbManager, QWidget *parent, ObjectEditorPaneData *paneData)
{
	(void)paneData;
	return new LaunchMethodEditorPane (mode, dbManager, parent);
}


// ***********
// ** Setup **
// ***********

void LaunchMethodEditorPane::setupText ()
{
	// Person required
	if (ui.personRequiredInput->count ()==0)
	{
		// The combo box is empty - add the items
		ui.personRequiredInput->addItem (tr ("Yes"), true );
		ui.personRequiredInput->addItem (tr ("No" ), false);
	}
	else
	{
		// The combo box is already filled - update the item texts
		ui.personRequiredInput->setItemText (0, tr ("Yes"));
		ui.personRequiredInput->setItemText (1, tr ("No"));
	}

	// Types
	if (ui.typeInput->count ()==0)
	{
		// The combo box is empty - add the items
		QList<LaunchMethod::Type> types=LaunchMethod::listTypes ();
		foreach (LaunchMethod::Type type, types)
		{
			QString text=firstToUpper (LaunchMethod::typeString (type));
			ui.typeInput->addItem (text, type);
		}
	}
	else
	{
		// The combo box is already filled - update the types stored in the
		// item data
		for (int i=0, n=ui.typeInput->count (); i<n; ++i)
		{
			LaunchMethod::Type type=(LaunchMethod::Type)ui.typeInput->itemData (i).toInt ();
			QString text=firstToUpper (LaunchMethod::typeString (type));
			ui.typeInput->setItemText (i, text);
		}
	}

}

void LaunchMethodEditorPane::loadData ()
{
	// Registrations
	ui.towplaneRegistrationInput->addItem ("");
	ui.towplaneRegistrationInput->addItems (cache.getPlaneRegistrations ());
}

// ****************
// ** GUI events **
// ****************

void LaunchMethodEditorPane::on_typeInput_activated (int index)
{
	bool airtow=(ui.typeInput->itemData (index)==LaunchMethod::typeAirtow);

	ui.individualTowplaneInput->setEnabled (airtow);
	ui.specificTowplanePane->setEnabled (airtow);
}

void LaunchMethodEditorPane::on_specificTowplaneInput_toggled (bool checked)
{
	ui.towplaneRegistrationInput->setEnabled (checked);
}


// ******************************
// ** ObjectEditorPane methods **
// ******************************

void LaunchMethodEditorPane::objectToFields (const LaunchMethod &launchMethod)
{
	ui.nameInput                ->setText                  (launchMethod.name);
	ui.shortNameInput           ->setText                  (launchMethod.shortName);
	ui.logStringInput           ->setText                  (launchMethod.logString);
	ui.keyboardShortcutInput    ->setText                  (launchMethod.keyboardShortcut);
	ui.typeInput                ->setCurrentItemByItemData (launchMethod.type);
	if (isBlank (launchMethod.towplaneRegistration))
		ui.individualTowplaneInput->setChecked (true);
	else
		ui.specificTowplaneInput->setChecked (true);
	ui.towplaneRegistrationInput->setEditText              (launchMethod.towplaneRegistration);
	ui.personRequiredInput      ->setCurrentItemByItemData (launchMethod.personRequired);
	ui.commentsInput            ->setText                  (launchMethod.comments);

	on_typeInput_activated (ui.typeInput->currentIndex ());
	on_specificTowplaneInput_toggled (ui.specificTowplaneInput->isChecked ());
}

void LaunchMethodEditorPane::fieldsToObject (LaunchMethod &launchMethod, bool performChecks)
{
	launchMethod.name                 = ui.nameInput                     ->text ().simplified ();
	launchMethod.shortName            = ui.shortNameInput                ->text ().simplified ();
	launchMethod.logString            = ui.logStringInput                ->text ().simplified ();
	launchMethod.keyboardShortcut     = ui.keyboardShortcutInput         ->text ().simplified ();
	launchMethod.type                 = (LaunchMethod::Type)ui.typeInput ->currentItemData ().toInt ();
	if (ui.individualTowplaneInput->isChecked ())
		launchMethod.towplaneRegistration = "";
	else
		launchMethod.towplaneRegistration = ui.towplaneRegistrationInput ->currentText ().simplified ();
	launchMethod.personRequired       = ui.personRequiredInput           ->currentItemData ().toBool ();
	launchMethod.comments             = ui.commentsInput                 ->text ().simplified ();

	if (!performChecks) return;

	// Error checks
	// TODO error checks:
	//   - towplane is glider
	//   - create towplane if it does not exist
	requiredField (launchMethod.name     , ui.nameInput     , tr ("Name not specified"         ));
	requiredField (launchMethod.shortName, ui.shortNameInput, tr ("Short name not specified"   ));
	requiredField (launchMethod.logString, ui.logStringInput, tr ("Logbook label not specified"));
}

// **********
// ** Misc **
// **********

void LaunchMethodEditorPane::changeEvent (QEvent *event)
{
	if (event->type () == QEvent::LanguageChange)
	{
		ui.retranslateUi (this);
		setupText ();
	}
	else
		ObjectEditorPane<LaunchMethod>::changeEvent (event);
}
