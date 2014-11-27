#ifndef FLARMNETWINDOW_H
#define FLARMNETWINDOW_H

#include "ui_FlarmNetWindow.h"
#include "src/gui/SkDialog.h"

class DbManager;
class QSortFilterProxyModel;
class FlarmNetRecord;
template<class T> class ObjectListModel;

class FlarmNetWindow: public SkDialog<Ui::FlarmNetWindowClass>
{
	Q_OBJECT

	public:
		FlarmNetWindow (DbManager &dbManager, QWidget *parent);
		virtual ~FlarmNetWindow ();

	private:
		DbManager &dbManager;

		ObjectListModel<FlarmNetRecord> *objectListModel;
		QSortFilterProxyModel* proxyModel;
		QMenu *contextMenu;

	private slots:
		void searchClear();
		void searchTextChanged (const QString&);

		void on_flarmNetTable_customContextMenuRequested (const QPoint &pos);
		void on_createPlaneAction_triggered ();

};

#endif
