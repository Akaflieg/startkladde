#ifndef CSVEXPORTDIALOG_H
#define CSVEXPORTDIALOG_H

#include "src/gui/SkDialog.h"

#include "ui_CsvExportDialog.h"

class CsvExportDialog: public SkDialog<Ui::CsvExportDialogClass>
{
    Q_OBJECT

	public:
    	CsvExportDialog (QWidget *parent=0);
    	~CsvExportDialog ();

    	QString getSeparator ();

    private slots:
    	void on_buttonBox_accepted ();

};

#endif
