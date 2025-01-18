#include "CsvExportDialog.h"

#include <iostream>

#include "src/util/qString.h"
#include "src/i18n/notr.h"

CsvExportDialog::CsvExportDialog (QWidget *parent):
	SkDialog<Ui::CsvExportDialogClass> (parent)
{
	ui.setupUi (this);

    // Setup the separator inputs
    ui.separatorInput->addItem (notr (";"));
    ui.separatorInput->addItem (notr (","));
    ui.separatorInput->addItem (notr ("|"));

    // Select ";"
    ui.separatorInput->setCurrentIndex(0);
}

CsvExportDialog::~CsvExportDialog ()
{

}

void CsvExportDialog::on_buttonBox_accepted ()
{
	this->accept ();
}

QString CsvExportDialog::getSeparator ()
{
    return ui.separatorInput->currentText();
}
