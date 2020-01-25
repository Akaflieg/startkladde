#include "CsvExportDialog.h"

#include <iostream>

#include <QTextCodec>

#include "src/util/qString.h"
#include "src/i18n/notr.h"

CsvExportDialog::CsvExportDialog (QWidget *parent):
	SkDialog<Ui::CsvExportDialogClass> (parent)
{
	ui.setupUi (this);

    // Only show the two commonly used codecs
    addCodecEntry(4);       // ISO Latin1 (ISO-8859-1
    addCodecEntry (106);    // UTF-8

    // Setup the separator inputs
    ui.separatorInput->addItem (notr (";"));
    ui.separatorInput->addItem (notr (","));
    ui.separatorInput->addItem (notr ("|"));

    // Select UTF-8
    ui.charsetList->setCurrentItemByItemData (106);

    // Select ";"
    ui.separatorInput->setCurrentIndex(0);
}

CsvExportDialog::~CsvExportDialog ()
{

}

void CsvExportDialog::addCodecEntry (int mib)
{
	QTextCodec *codec=QTextCodec::codecForMib (mib);
	QString codecName=QString::fromUtf8 (codec->name ());

	ui.charsetList->addItem (codecName, mib);
}

void CsvExportDialog::on_buttonBox_accepted ()
{
	selectedMib=ui.charsetList->itemData (ui.charsetList->currentIndex ()).toInt ();
	this->accept ();
}

int CsvExportDialog::getSelectedMib ()
{
	return selectedMib;
}

QTextCodec *CsvExportDialog::getSelectedCodec ()
{
	return QTextCodec::codecForMib (selectedMib);
}

QString CsvExportDialog::getSeparator ()
{
    return ui.separatorInput->currentText();
}
