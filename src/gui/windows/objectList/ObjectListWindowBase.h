#ifndef OBJECTLISTWINDOWBASE_H
#define OBJECTLISTWINDOWBASE_H

#include "ui_ObjectListWindowBase.h"

#include "src/db/DbManager.h" // Required for DbManager::State
#include "src/gui/PasswordCheck.h"
#include "src/gui/PasswordPermission.h"
#include "src/gui/SkMainWindow.h"

/*
 * TODO: Menu View->Sort and hotkey
 */

class ObjectListWindowBase : public SkMainWindow<Ui::ObjectListWindowBaseClass>
{
    Q_OBJECT

	public:
		ObjectListWindowBase (DbManager &manager, QWidget *parent = 0);
		~ObjectListWindowBase ();

	public slots:
		virtual void on_actionNew_triggered ()=0;
		virtual void on_actionEdit_triggered ()=0;
		virtual void on_actionDelete_triggered ()=0;
		virtual void on_actionRefresh_triggered ()=0;
		virtual void on_actionClose_triggered ();

		virtual void on_table_doubleClicked (const QModelIndex &index)=0;
		virtual void on_table_customContextMenuRequested (const QPoint &pos)=0;

	protected slots:
		virtual void databaseStateChanged (DbManager::State state);

	protected:
		DbManager &manager;
		void keyPressEvent (QKeyEvent *e);


		PasswordCheck databasePasswordCheck;
		PasswordPermission editPermission;

        private slots:
                // we cannot declare the slots in the derived template class
                virtual void searchClear() = 0;
                virtual void searchTextChanged (const QString&) = 0;
};

#endif
