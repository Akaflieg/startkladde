#include "StatisticsWindow.h"

#include <QPushButton>

#include "src/util/qString.h"

StatisticsWindow::StatisticsWindow (QAbstractTableModel *model, bool modelOwned, const char *ntr_title, QWidget *parent):
	SkDialog<Ui::StatisticsWindowClass> (parent),
	model (model), modelOwned (modelOwned), ntr_title (ntr_title)
{
	ui.setupUi(this);


	setupText ();

	ui.table->setModel (model);

	ui.table->resizeColumnsToContents ();
	ui.table->resizeRowsToContents ();
}

StatisticsWindow::~StatisticsWindow()
{
	if (modelOwned)
		delete model;
}

void StatisticsWindow::setupText ()
{
	setWindowTitle (tr (ntr_title));
}

void StatisticsWindow::display (QAbstractTableModel *model, bool modelOwned, const char *ntr_title, QWidget *parent)
{
	StatisticsWindow *window=new StatisticsWindow (model, modelOwned, ntr_title, parent);
	window->setAttribute (Qt::WA_DeleteOnClose, true);
	window->show ();
}

void StatisticsWindow::languageChanged ()
{
	SkDialog<Ui::StatisticsWindowClass>::languageChanged ();
	setupText ();

	ui.table->resizeColumnsToContents ();
	ui.table->resizeRowsToContents ();
}
