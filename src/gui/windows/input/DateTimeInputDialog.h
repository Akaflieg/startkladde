#ifndef DATETIMEINPUTDIALOG_H
#define DATETIMEINPUTDIALOG_H

#include "src/gui/SkDialog.h"

#include "ui_DateTimeInputDialog.h"

class DateTimeInputDialog: public SkDialog<Ui::DateTimeInputDialogClass>
{
	public:
		DateTimeInputDialog (QWidget *parent = NULL, Qt::WindowFlags f=Qt::Widget);
		~DateTimeInputDialog();

		static bool editDateTime (QWidget *parent, QDate *date, QTime *time, QString title);
};

#endif
