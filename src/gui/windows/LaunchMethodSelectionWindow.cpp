#include "LaunchMethodSelectionWindow.h"

#include <QPushButton>

#include "src/db/cache/Cache.h"
#include "src/gui/dialogs.h"
#include "src/util/qString.h"

LaunchMethodSelectionWindow::LaunchMethodSelectionWindow (QWidget *parent):
	SkDialog<Ui::LaunchMethodSelectionWindowClass> (parent)
{
	ui.setupUi (this);
	this->adjustSize ();
}

LaunchMethodSelectionWindow::~LaunchMethodSelectionWindow ()
{

}

bool LaunchMethodSelectionWindow::select (Cache &cache, dbId &value, bool &loadPreselection, QWidget *parent)
{
	if (cache.getLaunchMethods ().getList ().isEmpty ())
	{
		showWarning (tr ("No launch methods defined"),
			tr ("No launch method can be preselected because no launch methods are defined."), parent);
		return false;
	}

	LaunchMethodSelectionWindow *window=new LaunchMethodSelectionWindow (parent);
	window->setModal (true);

	foreach (const LaunchMethod &launchMethod, cache.getLaunchMethods ().getList ())
		window->ui.launchMethodInput->addItem (launchMethod.nameWithShortcut (), launchMethod.getId ());

	window->ui.preselectionCheckbox->setChecked (idValid (value));
	if (idValid (value))
		window->ui.launchMethodInput->setCurrentItemByItemData (value);

    window->ui.loadPreselectionCheckbox->setChecked(loadPreselection);

	if (window->exec ()!=QDialog::Accepted) return false;

	if (window->ui.preselectionCheckbox->isChecked ())
		value=window->ui.launchMethodInput->currentItemData ().toLongLong ();
	else
		value=invalidId;

    loadPreselection = window->ui.loadPreselectionCheckbox->isChecked();

	return true;
}

void LaunchMethodSelectionWindow::languageChanged ()
{
	SkDialog<Ui::LaunchMethodSelectionWindowClass>::languageChanged ();

	// Required because we have a label with word wrapping enabled.
	adjustSize ();
}
