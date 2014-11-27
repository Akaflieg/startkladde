#ifndef DATEINPUTDIALOG_H
#define DATEINPUTDIALOG_H

#include "src/gui/SkDialog.h"

#include "ui_DateInputDialog.h"

/**
 * Note that the results of selecting "today" or "yesterday" might be unexpected
 * if the date changes between invocation and acceptance of the dialog (i. e. at
 * midnight).
 */
class DateInputDialog: public SkDialog<Ui::DateInputDialogClass>
{
	public:
		DateInputDialog (QWidget *parent = 0, Qt::WindowFlags f=0);
		~DateInputDialog();

		static bool editDate  (QDate *date ,              const QString &title, const QString &text, QWidget *parent);
		static bool editRange (QDate *first, QDate *last, const QString &title, const QString &text, QWidget *parent);

	protected:
		void setup (bool modal, const QString &title, const QString &text, bool rangeEnabled);

		void setupDates (QDate otherDate, QDate firstDate, QDate lastDate);

		void activateOption (QRadioButton *selection, QWidget *focusWidget);
		void activateToday ();
		void activateYesterday ();
		void activateOther ();
		void activateRange ();
};

#endif // DATEINPUTDIALOG_H
