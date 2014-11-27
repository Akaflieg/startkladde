#include "DateInputDialog.h"

#include <QPushButton>

#include "src/util/qDate.h"

DateInputDialog::DateInputDialog (QWidget *parent, Qt::WindowFlags f):
	SkDialog<Ui::DateInputDialogClass> (parent, f)
{
	ui.setupUi(this);
}

DateInputDialog::~DateInputDialog()
{

}

void DateInputDialog::setup (bool modal, const QString &title, const QString &text, bool rangeEnabled)
{
	setModal (modal);

	// Setup window title and date text label
	setWindowTitle (title);
	ui.dateLabel->setText (text);

	// Enable or disable the "range" option
	ui.dateRangeSelect ->setVisible (rangeEnabled);
	ui.firstDateInput  ->setVisible (rangeEnabled);
	ui.dateRangeToLabel->setVisible (rangeEnabled);
	ui.lastDateInput   ->setVisible (rangeEnabled);

	// Enable the correct date input(s)
	ui.otherDateInput->setEnabled (ui.otherDateSelect->isChecked ());

	ui.firstDateInput ->setEnabled (ui.dateRangeSelect->isChecked ());
	ui.lastDateInput  ->setEnabled (ui.dateRangeSelect->isChecked ());

	resize (sizeHint ());
}

void DateInputDialog::setupDates (QDate otherDate, QDate firstDate, QDate lastDate)
{
	ui.otherDateInput->setDate (otherDate);
	ui.firstDateInput->setDate (firstDate);
	ui.lastDateInput ->setDate (lastDate );
}

void DateInputDialog::activateOption (QRadioButton *selection, QWidget *focusWidget)
{
	selection->setChecked (true);
	focusWidget->setFocus ();
}

void DateInputDialog::activateToday     () { activateOption (ui.todaySelect    , ui.todaySelect    );}
void DateInputDialog::activateYesterday () { activateOption (ui.yesterdaySelect, ui.yesterdaySelect);}
void DateInputDialog::activateOther     () { activateOption (ui.otherDateSelect, ui.otherDateInput );}
void DateInputDialog::activateRange     () { activateOption (ui.dateRangeSelect, ui.firstDateInput );}

bool DateInputDialog::editDate  (QDate *date, const QString &title, const QString &text, QWidget *parent)
{
	// Setup the dialog
	DateInputDialog dialog (parent);
	dialog.setup (true, title, text, false);

	// Determine some constants
	const QDate today=QDate::currentDate ();
	const QDate yesterday=today.addDays (-1);

	// Setup the date fields
	dialog.ui.otherDateInput->setDate (validDate (*date, today));

	// Select the initial option
	if      (*date == today    ) dialog.activateToday     ();
	else if (*date == yesterday) dialog.activateYesterday ();
	else if (date->isValid ()  ) dialog.activateOther     ();
	else                         dialog.activateToday     ();

	// Show the dialog
	int result=dialog.exec ();
	if (result!=QDialog::Accepted) return false;

	// Depending on the selected option, store the result
	if      (dialog.ui.todaySelect    ->isChecked ()) *date=today;
	else if (dialog.ui.yesterdaySelect->isChecked ()) *date=yesterday;
	else if (dialog.ui.otherDateSelect->isChecked ()) *date=dialog.ui.otherDateInput->date ();
	else return false;

	return true;
}

/**
 * @param first must not be null
 * @param last must not be null
 * @param title
 * @param text
 * @param parent
 * @return
 */
bool DateInputDialog::editRange (QDate *first, QDate *last, const QString &title, const QString &text, QWidget *parent)
{
	// Setup the dialog
	DateInputDialog dialog (parent);
	dialog.setup (true, title, text, true);

	// Determine some constants
	QDate today      =QDate::currentDate ();
	QDate yesterday  =today.addDays (-1);


	// Setup the date fields and select the initial option
	if      (!first->isValid () || !last->isValid ()) { dialog.setupDates (today    , firstOfYear (today)    , today    ); dialog.activateToday     (); } // One invalid - today
	else if (*first!=*last)                           { dialog.setupDates (*last    , *first                 , *last    ); dialog.activateRange     (); } // Different - range
	else if (*first==today)                           { dialog.setupDates (today    , firstOfYear (today)    , today    ); dialog.activateToday     (); } // Identical (today)
	else if (*first==yesterday)                       { dialog.setupDates (yesterday, firstOfYear (yesterday), yesterday); dialog.activateYesterday (); } // Identical (yesterday)
	else                                              { dialog.setupDates (*first   , firstOfYear (*first)   , *first   ); dialog.activateOther     (); } // Identical (other)

	// Show the dialog
	int result=dialog.exec ();
	if (result!=QDialog::Accepted) return false;

	// Depending on the selected option, store the result
	if      (dialog.ui.todaySelect    ->isChecked ()) { *first=today                            ; *last=*first; }
	else if (dialog.ui.yesterdaySelect->isChecked ()) { *first=yesterday                        ; *last=*first; }
	else if (dialog.ui.otherDateSelect->isChecked ()) { *first=dialog.ui.otherDateInput->date (); *last=*first; }
	else if (dialog.ui.dateRangeSelect->isChecked ()) { *first=dialog.ui.firstDateInput->date (); *last=dialog.ui.lastDateInput->date (); }
	else return false;

	return true;
}
