#include "CsvExportDialog.h"

#include <iostream>

#include <QTextCodec>

#include "src/util/qString.h"
#include "src/i18n/notr.h"

CsvExportDialog::CsvExportDialog (QWidget *parent):
	SkDialog<Ui::CsvExportDialogClass> (parent)
{
	ui.setupUi (this);

	// Add some of the most commonly-used codecs at the top of the list
	addCodecEntry (0); // System
	addCodecEntry (106); // UTF-8
	addCodecEntry (1015); // UTF-16
	// ISO Latin1 (ISO-8859-1, MIB=4) is not added because it is close to the
	// top of the list

	// Add all codecs, in order of ascending MIB. MIB 0 (System) is not
	// included again; all other codecs with positive MIB are included, even if
	// they were already added at the top.
	QList<int> mibs=QTextCodec::availableMibs ();
	qSort (mibs);
	foreach (int mib, mibs)
		if (mib>0) // Don't include 0 again
			addCodecEntry (mib);

	// Select UTF-8
	ui.charsetList->setCurrentItemByItemData (106);

	// Setup the separator inputs
	ui.separatorInput->addItem (notr (","));
	ui.separatorInput->addItem (notr (";"));
}

CsvExportDialog::~CsvExportDialog ()
{

}

void CsvExportDialog::addCodecEntry (int mib)
{
	QTextCodec *codec=QTextCodec::codecForMib (mib);
	QString codecName=QString::fromUtf8 (codec->name ());

	//std::cout << mib << notr (" - ") << codecName << std::endl;

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
	return ui.separatorInput->currentText ();
}
