#ifndef DATETIMEINPUTDIALOG_H
#define DATETIMEINPUTDIALOG_H

#include "src/gui/SkDialog.h"

#include "ui_DateTimeInputDialog.h"

class DateTimeInputDialog: public SkDialog<Ui::DateTimeInputDialogClass>
{
	public:
		DateTimeInputDialog (QWidget *parent = 0, Qt::WindowFlags f=0);
		~DateTimeInputDialog();

		static bool editDateTime (QWidget *parent, QDate *date, QTime *time, QString title);
};

#endif
