#include "DateTimeInputDialog.h"

#include <QPushButton>

// TODO: The title is not retranslated because it is specified outside of this
// class. This can be changed using the technique used for StatisticsWindow.
// Since the window is modal, a language change should not happen while the
// window is open.

DateTimeInputDialog::DateTimeInputDialog (QWidget *parent, Qt::WindowFlags f):
	SkDialog<Ui::DateTimeInputDialogClass> (parent, f)
{
	ui.setupUi(this);
}

DateTimeInputDialog::~DateTimeInputDialog()
{

}

bool DateTimeInputDialog::editDateTime (QWidget *parent, QDate *date, QTime *time, QString title)
{
	DateTimeInputDialog dialog (parent);
	dialog.setModal (true);

	dialog.setWindowTitle (title);

	dialog.ui.dateInput->setDate (*date);
	dialog.ui.timeInput->setTime (*time);

	if (QDialog::Accepted==dialog.exec ())
	{
		*date=dialog.ui.dateInput->date ();
		*time=dialog.ui.timeInput->time ();

		return true;
	}
	else
	{
		return false;
	}
}
