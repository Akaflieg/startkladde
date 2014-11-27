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

    	int getSelectedMib ();
    	QTextCodec *getSelectedCodec ();
    	QString getSeparator ();

    private slots:
    	void on_buttonBox_accepted ();


	private:
    	void addCodecEntry (int mib);

    	int selectedMib;
};

#endif
