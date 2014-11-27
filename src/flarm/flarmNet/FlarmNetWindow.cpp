#include "FlarmNetWindow.h"

#include <QSettings>
#include <QSortFilterProxyModel>
#include <QMenu>

#include "src/db/DbManager.h"
#include "src/flarm/flarmNet/FlarmNetRecord.h"
#include "src/flarm/flarmNet/FlarmNetRecordModel.h"
#include "src/model/objectList/ObjectListModel.h"
#include "src/gui/windows/objectEditor/ObjectEditorWindow.h"

class FlarmNetRecord;

FlarmNetWindow::FlarmNetWindow (DbManager &dbManager, QWidget *parent):
	SkDialog<Ui::FlarmNetWindowClass> (parent),
	dbManager (dbManager)
{
	ui.setupUi (this);

	// Clear search button
	QStyle* style = QApplication::style ();
	ui.clearButton->setIcon (style->standardIcon (QStyle::SP_DialogCloseButton));
	ui.clearButton->setVisible (false);

	connect (ui.searchEdit,  SIGNAL(textChanged(const QString&)), this, SLOT(searchTextChanged(const QString&)));
	connect (ui.clearButton, SIGNAL(pressed()), this, SLOT(searchClear()));

	// Get the list of FlarmNet records from the database. It will be deleted by
	// the ObjectListModel.
	EntityList<FlarmNetRecord> *flarmNetRecords=new EntityList<FlarmNetRecord> (dbManager.getCache ().getFlarmNetRecords ());
	
	// Create the object model. It will be deleted by the ObjectListModel.
	ObjectModel<FlarmNetRecord> *flarmNetRecordModel = new FlarmNetRecordModel ();

	// Create the object list model. It will be deleted by its parent, this.
	objectListModel = new ObjectListModel<FlarmNetRecord> (this);
	objectListModel->setList  (flarmNetRecords    , true);
	objectListModel->setModel (flarmNetRecordModel, true);

	// The default row height is too big. Resizing the rows to the contents
	// (using resizeRowsToContents or setautoResizeRows) is very slow for larger
	// lists. Therefore, we use a dummy list to determine the default row height
	// and use setDefaultSectionSize of the vertical header to set the height
	// for rows added later.
	// FIXME this should be encapsulated. However, we'd like to be able to do
	// this without explicitly knowing the type of the object, so the generic
	// model viewer window does not have to be a template. The type is known to
	// the model which is passed into this class, so the model should be able
	// to return dummy contents.
	// The functionality could also be in the view, but note that at the current
	// time, the model may be empty, so we'd have to set a flag to consider the
	// first item added.
	EntityList<FlarmNetRecord> dummyList;
	dummyList.append (FlarmNetRecord ());
	FlarmNetRecordModel dummyObjectModel;
	ObjectListModel<FlarmNetRecord> dummyListModel (this);
	dummyListModel.setList  (&dummyList       , false);
	dummyListModel.setModel (&dummyObjectModel, false);
	ui.flarmNetTable->setModel (&dummyListModel);
	ui.flarmNetTable->resizeRowsToContents ();
	int rowHeight=ui.flarmNetTable->verticalHeader ()->sectionSize (0);
	ui.flarmNetTable->verticalHeader ()->setDefaultSectionSize (rowHeight);
	ui.flarmNetTable->setModel (NULL);

	// Create a sort/filter proxy model. It will be deleted by its parent, this.
	proxyModel = new QSortFilterProxyModel (this);
	proxyModel->setSourceModel (objectListModel);
	proxyModel->setSortCaseSensitivity (Qt::CaseInsensitive);
	proxyModel->setDynamicSortFilter (true);

	// filter all columns
	proxyModel->setFilterKeyColumn (-1);
	// Setup the table view
	ui.flarmNetTable->setModel (proxyModel);

	// Context menu
	contextMenu=new QMenu (this);
	contextMenu->addAction (ui.createPlaneAction);
	//contextMenu->addSeparator ();
}

FlarmNetWindow::~FlarmNetWindow ()
{
} 

void FlarmNetWindow::searchClear () {
        // qDebug () << "FlarmNetWindow::searchClear: " << endl;
        ui.searchEdit->clear();
}

void FlarmNetWindow::searchTextChanged (const QString& search) {
        // qDebug () << "FlarmNetWindow::searchTextChanged: " << search << endl;
        proxyModel->setFilterRegExp (QRegExp (search, Qt::CaseInsensitive, QRegExp::FixedString));
        ui.clearButton->setVisible (!search.isEmpty());
} 

void FlarmNetWindow::on_flarmNetTable_customContextMenuRequested (const QPoint &pos)
{
	// TODO the position seems to be off
	contextMenu->popup (ui.flarmNetTable->mapToGlobal (pos), 0);
}

void FlarmNetWindow::on_createPlaneAction_triggered ()
{
	QModelIndex proxyIndex=ui.flarmNetTable->currentIndex ();
	QModelIndex sourceIndex=proxyModel->mapToSource (proxyIndex);

	FlarmNetRecord record=objectListModel->at (sourceIndex.row ());

	Plane preset=record.toPlane (true);
	preset.comments=tr ("Created from FlarmNet record");

	// TODO if a plane with that registration already exists, offer to update it
	// instead.
	ObjectEditorWindow<Plane>::createObjectPreset (this, dbManager, preset, NULL, NULL);
}
