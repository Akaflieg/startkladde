#include "ConfirmOverwritePersonDialog.h"

#include <QPushButton>

#include "src/util/qString.h"
#include "src/model/Person.h"
#include "src/model/objectList/ObjectModel.h"
#include "src/model/objectList/ObjectListModel.h"
#include "src/model/objectList/MutableObjectList.h"

/**
 * Creates a ConfirmOverwritePersonDialog
 *
 * @param parent the Qt parent widgets
 * @param f the Qt window flags
 */
ConfirmOverwritePersonDialog::ConfirmOverwritePersonDialog (QWidget *parent, Qt::WindowFlags f):
	SkDialog<Ui::ConfirmOverwritePersonDialogClass> (parent, f)
{
	ui.setupUi(this);
}

ConfirmOverwritePersonDialog::~ConfirmOverwritePersonDialog ()
{
}

/**
 * Sets up the window
 *
 * This method clears, sets up and populates the tree view with the correct
 * and wrong people, and adjusts the labels.
 *
 * @param correctPerson the correct person entry
 * @param wrongPeople a list of one or more wrong person entries
 */
void ConfirmOverwritePersonDialog::setup (const Person &correctPerson, const QList<Person> &wrongPeople)
{
	// Store the people data
	this->correctPerson=correctPerson;
	this->wrongPeople=wrongPeople;

	// Clear the tree
	ui.entriesTree->clear ();

	// Create the root items
	wrongParentItem  =new QTreeWidgetItem (ui.entriesTree);
	correctParentItem=new QTreeWidgetItem (ui.entriesTree);

	// Setup the root items
	wrongParentItem  ->setFirstColumnSpanned (true);
	correctParentItem->setFirstColumnSpanned (true);

	// Create (empty) children items for the correct person and wrong people.
	// The contents depend on the language and will be filled in by setupTexts.
	new QTreeWidgetItem (correctParentItem);
	for (int i=0, n=wrongPeople.size (); i<n; ++i)
		new QTreeWidgetItem (wrongParentItem);

	// Fill in all language dependent texts
	setupTexts ();

	// Expand the root items
	wrongParentItem  ->setExpanded (true);
	correctParentItem->setExpanded (true);

	for (int i=0, n=ui.entriesTree->columnCount (); i<n; ++i)
		ui.entriesTree->resizeColumnToContents (i);
}

/**
 * Sets up all texts that may depend on the language. This method is called on
 * dialog creation and on retranslation.
 */
void ConfirmOverwritePersonDialog::setupTexts ()
{
	// Set the text of the tree widget headers
	ui.entriesTree->setHeaderLabels (model.displayHeaderStrings ());

	// Set the text of the root item for the correct person
	correctParentItem->setText (0, tr ("Correct entry"));

	// Set the texts that depend on the number of wrong people: the root item
	// for the wrong people and the labels
	if (wrongPeople.size ()>1)
	{
		wrongParentItem->setText (0, tr ("Erroneous entries (will be overwritten)"));
		ui.introLabel->setText (tr ("The following erroneous entries will be replaced with the correct entry."));
		ui.descriptionLabel->setText (tr (
			"<html>"
			"All entries must refer to the same person. All flights of the wrong people will be assigned to the correct person. "
			"<font color=\"#FF0000\">Warning: this action cannot be undone.</font> "
			"Continue?"
			"</html>"));
	}
	else
	{
		wrongParentItem->setText (0, tr ("Erroneous entry (will be overwritten)"));
		ui.introLabel->setText (tr ("The following erroneous entry will be replaced with the correct entry."));
		ui.descriptionLabel->setText (tr (
			"<html>"
			"Both entries must refer to the same person. All flights of the wrong person will be assigned to the correct person. "
			"<font color=\"#FF0000\">Warning: this action cannot be undone.</font> "
			"Continue?"
			"</html>"));

	}

	// Set the texts for the actual people entries (these, too, may depend on
	// the language, and since we copy the data to the tree widget items instead
	// of using a modelview, we must update them manually.
	for (int i=0, n=wrongPeople.size (); i<n; ++i)
		*(wrongParentItem->child (i))=QTreeWidgetItem (model.displayDataStrings (wrongPeople[i]));
	*(correctParentItem->child (0))=QTreeWidgetItem (model.displayDataStrings (correctPerson));

	// Resize the columns to the contents
	for (int i=0; i<model.columnCount (); ++i)
		ui.entriesTree->resizeColumnToContents (i);
}

/**
 * Invokes the ConfirmOverwritePersonDialog
 *
 * @param correctPerson the correct person entry
 * @param wrongPeople a list of one or more wrong person entries
 * @param parent the Qt parent widget
 * @return true if the user accepted the operation, false otherwise
 */
bool ConfirmOverwritePersonDialog::confirmOverwrite (const Person &correctPerson, const QList<Person> &wrongPeople, QWidget *parent)
{
	ConfirmOverwritePersonDialog dialog (parent);
	dialog.setModal (true);

	dialog.setup (correctPerson, wrongPeople);

	if (QDialog::Accepted==dialog.exec ())
		return true;
	else
		return false;
}

void ConfirmOverwritePersonDialog::languageChanged ()
{
	SkDialog<Ui::ConfirmOverwritePersonDialogClass>::languageChanged ();
	setupTexts ();
	adjustSize ();
}
