#include "AboutDialog.h"

#include "src/version.h"

AboutDialog::AboutDialog (QWidget *parent):
	QDialog (parent)
{
	ui.setupUi (this);

	ui.versionInput->setText (getVersion ());
}

AboutDialog::~AboutDialog()
{
}
