#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QDialog>
#include "ui_AboutDialog.h"

class AboutDialog: public QDialog
{
	Q_OBJECT

	public:
		AboutDialog (QWidget *parent);
		~AboutDialog ();

	private:
		Ui::AboutDialogClass ui;
};

#endif

