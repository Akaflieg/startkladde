#include "ObjectListWindow.h"

/*
 * Improvements:
 *   - add a way to refresh the list from the database
 *   - display centered
 */

/*
 * Creating is not as easy as for ObjectEditorPane, because we want to have a
 * default class (ObjectListWindow) which can be overridden for a given object
 * type (e. g. PersonListWindow).
 */

#include <iostream>

#include <QMessageBox>
#include <QSortFilterProxyModel>
#include <QKeyEvent>
#include <QPushButton>

#include "src/gui/windows/objectEditor/ObjectEditorWindow.h"
#include "src/model/objectList/ObjectListModel.h"
#include "src/gui/dialogs.h"
#include "src/text.h"
#include "src/model/objectList/AutomaticEntityList.h"
#include "src/model/objectList/ObjectModel.h"
#include "src/i18n/notr.h"


/**
 * ATTENTION: this constructor should never be called except by the create
 * method or a subclass constructor to allow specialization.
 *
 * @param manager
 * @param parent
 * @see create
 */
template<class T> ObjectListWindow<T>::ObjectListWindow (DbManager &manager, QWidget *parent):
	ObjectListWindowBase (manager, parent),
	manager (manager),
	contextMenu (new QMenu (this))
{
	editPermission.setMessage (qApp->translate ("ObjectListWindow<T>", "The database password must be entered to edit %1.").arg (T::objectTypeDescriptionPlural ()));

	// Create the object listModel
	list = new AutomaticEntityList<T>
		(manager.getCache (), manager.getCache ().getObjects<T> ().getList (), this);

	// Create the object model and object list model
	objectModel=new typename T::DefaultObjectModel ();
	listModel = new ObjectListModel<T> (this);
	listModel->setList  (list       , false);
	listModel->setModel (objectModel, true );


	// Set the list model as the table's model with a sort proxy
	proxyModel=new QSortFilterProxyModel (this);
	proxyModel->setSourceModel (listModel);
	proxyModel->setSortCaseSensitivity (Qt::CaseInsensitive);
	proxyModel->setDynamicSortFilter (true);

	// filter all columns
	proxyModel->setFilterKeyColumn (-1);
                                
	ui.table->setModel (proxyModel);

	ui.table->setAutoResizeRows (true);
	ui.table->sortByColumn (0, Qt::AscendingOrder);
	ui.table->resizeColumnsToContents ();
	ui.table->resizeRowsToContents (); // Do this although autoResizeRows is true - there are already rows.

	// TODO this should be done later - a subclass may add menus and the mnemonics may be missed
	setupText ();
}

template<class T> ObjectListWindow<T>::~ObjectListWindow()
{
	// list, listModel and proxyModel are deleted automatically by QWidget
	// objectModel ist deleted automatically be objectListModel
}

template<class T> void ObjectListWindow<T>::show (DbManager &manager, QWidget *parent)
{
	show (manager, false, parent);
}

template<class T> void ObjectListWindow<T>::show (DbManager &manager, bool passwordRequiredForEdit, QWidget *parent)
{
	// Create the window
	ObjectListWindow<T> *window = ObjectListWindow<T>::create (manager, parent);
	window->setAttribute (Qt::WA_DeleteOnClose, true);

	window->editPermission.setPasswordRequired (passwordRequiredForEdit);

	// Show the window
	window->show ();
}

template<class T> void ObjectListWindow<T>::setupText ()
{
	setWindowTitle (firstToUpper (T::objectTypeDescriptionPlural ()));

	// TODO should probably autodetect mnemonics
	QString mnemonics=qApp->translate ("ObjectListWindow<T>", "w", "Window menu mnemonic");
	QChar closeButtonMnemonic=getMnemonic (ui.buttonBox->button (QDialogButtonBox::Close)->text ());
	if (!closeButtonMnemonic.isNull ()) mnemonics+=closeButtonMnemonic;

	// Set the "Object" menu
	QString objectMenuText=T::objectTypeDescription ();
	objectMenuText=firstToUpper (objectMenuText);
	objectMenuText=insertMnemonic (objectMenuText, mnemonics);
	ui.menuObject->setTitle (objectMenuText);
}


/**
 * Appends an object from a specified position (index) in the table to a QList
 *
 * The target list contains objects (rather than pointers), so a copy of the
 * objects is made. Thus, the objects will remain valid even if the model
 * changes (although they may become out of sync with the database).
 *
 * @param targetList the list to append the object to
 * @param tableIndex the index of one of the cells of the object to add. Note
 *                   that this index refers to the table, not to the
 *                   ObjectListModel (there is a proxy model).
 */
template<class T> void ObjectListWindow<T>::appendObjectTo (QList<T> &targetList, const QModelIndex &tableIndex)
{
	if (!tableIndex.isValid ())
		return;

	QModelIndex listIndex=proxyModel->mapToSource (tableIndex);
	if (!listIndex.isValid ())
		return;

	const T &object=listModel->at (listIndex);
	targetList.append (object);
}

/**
 * Determines the number of active objects
 *
 * See the activeObjects method for a description of which objects are
 * considered active.
 *
 * @return the number of active objects
 * @see #activeObjects
 */
template<class T> int ObjectListWindow<T>::activeObjectCount ()
{
	// WARNING: make sure getActiveObjects corresponds to this method

	// If there is a selection, the active objects are the selected objects
	// (rows)
	if (ui.table->selectionModel ()->hasSelection ())
		return ui.table->selectionModel ()->selectedRows ().size ();

	// If there is no selection, the active object may be the current one
	QModelIndex tableIndex=ui.table->currentIndex ();
	if (tableIndex.isValid ())
	{
		QModelIndex listIndex=proxyModel->mapToSource (tableIndex);

		if (listIndex.isValid ())
			return 1;
	}

	return 0;
}

/**
 * Lists the active objects
 *
 * The active objects are the selected objects if any objects are selected, or
 * the current object (if any) if no objects are selected. If there are neither
 * selected objects nor a current object, there are no active objects and the
 * returned list will be empty.
 *
 * The list contains values, so a copy of the objects is made. The values will
 * not become invalid if the model is changed, but they may become out of sync
 * with the database.
 *
 * @return a list containing copies of the active objects
 * @see activeObjectCount
 */
template<class T> QList<T> ObjectListWindow<T>::activeObjects ()
{
	// WARNING: make sure activeObjectCount corresponds to this method

	QList<T> activeObjects;

	if (ui.table->selectionModel ()->hasSelection ())
	{
		// There's a selection
		foreach (const QModelIndex &tableIndex, ui.table->selectionModel ()->selectedRows (0))
			appendObjectTo (activeObjects, tableIndex);
	}
	else
	{
		// There's no selection
		appendObjectTo (activeObjects, ui.table->currentIndex ()); // May be invalid
	}

	assert (activeObjects.size ()==activeObjectCount ());

	return activeObjects;
}

template<class T> void ObjectListWindow<T>::on_actionNew_triggered ()
{
	if (!editPermission.permit (this)) return;

	ObjectEditorWindow<T>::createObject (this, manager);
}

template<class T> void ObjectListWindow<T>::on_actionEdit_triggered ()
{
	if (!editPermission.permit (this)) return;

	QList<T> objects=activeObjects ();
	if (objects.isEmpty ()) return;

	// Improvement: an ObjectEditorWindow that can edit multiple objects
	foreach (const T &object, objects)
	{
		int result=editObject (object);
		if (result!=QDialog::Accepted) break;
	}
}

/**
 * Can be overridden by implementations to allow creation of custom editor
 * windows
 */
template<class T> int ObjectListWindow<T>::editObject (const T &object)
{
	return ObjectEditorWindow<T>::editObject (this, manager, object);
}

/**
 *
 * @param id
 * @return false if canceled
 */
template<class T> bool ObjectListWindow<T>::checkAndDelete (const T &object)
{
	bool objectUsed=true;

	try
	{
		objectUsed=manager.objectUsed<T> (object.getId (), this);
	}
	catch (OperationCanceledException &)
	{
		return false;
	}

	if (objectUsed)
	{
		QString title=firstToUpper (
			qApp->translate ("ObjectListWindow<T>", "%1 in use")
				.arg (T::objectTypeDescription ()));
		QString text=firstToUpper (
			qApp->translate ("ObjectListWindow<T>", "%1 %2 is in use and cannot be deleted.")
				.arg (T::objectTypeDescriptionDefinite (), object.getDisplayName ()));

		QMessageBox::StandardButtons buttons=QMessageBox::Ok;

		if (QMessageBox::critical (this, title, text, buttons, QMessageBox::Ok)==QMessageBox::Ok)
			// Continue
			return true;
		else
			// Canceled
			return false;
	}
	else
	{
		try
		{
			manager.deleteObject<T> (object.getId (), this);
			return true;
		}
		catch (OperationCanceledException &)
		{
			// TODO the cache may now be inconsistent
			return false;
		}
	}
}

template<class T> void ObjectListWindow<T>::on_actionDelete_triggered ()
{
	if (!editPermission.permit (this)) return;

	QList<T> objects=activeObjects ();

	if (objects.isEmpty ())
		return;

	// TODO better selection after deletion
	//   - single object deleted => next one
	//   - last object deleted => last one
	//   - multiple (non-continuous) objects deleted => ...?
	//   - multiple (non-continuous) objects delete, seleced out of order => ...?
	//   - ...?
	// Suggestions:
	//   - the last one deleted
	//   - the first (top) one selected (they may be selected out of order)
	QModelIndex previousIndex=ui.table->currentIndex ();

	for (int i=0; i<objects.size (); ++i)
	{
		const T &object=objects.at (i);

		QString title=qApp->translate ("ObjectListWindow<T>", "Delete %1?").arg (T::objectTypeDescription ());
		QString question=qApp->translate ("ObjectListWindow<T>", "Do you want to delete %1 %2?").arg (T::objectTypeDescriptionDefinite (), object.getDisplayName ());

		bool confirmDelete=false, cancel=false;

		if (i<objects.size ()-1)
		{
			// More objects to follow
			QMessageBox::StandardButton result=yesNoCancelQuestion (this, title, question);
			confirmDelete=(result==QMessageBox::Yes);
			cancel=(result==QMessageBox::Cancel);
		}
		else
		{
			// Last object
			// TODO add a "Yes to all" option and a proper progress indicator
			confirmDelete=yesNoQuestion (this, title, question);
			cancel=false;
		}

		if (confirmDelete)
		{
			if (!checkAndDelete (object))
				break;
		}
		else if (cancel)
		{
			break;
		}
		// else: No
	}

	// Don't make any change if there are still selected rows
	if (!ui.table->selectionModel ()->hasSelection ())
	{
		QAbstractItemModel *model=ui.table->model ();

		if (previousIndex.row ()>=model->rowCount ())
			previousIndex=model->index (model->rowCount ()-1, previousIndex.column ());

		ui.table->setCurrentIndex (previousIndex); // Handles deletion of last item correctly
	}
}

template<class T> void ObjectListWindow<T>::on_actionRefresh_triggered ()
{
	try
	{
		//manager.refreshCache (this);
		manager.refreshObjects<T> (this);
	}
	catch (OperationCanceledException &ex) {}

	// We want to select the "same item as before" after the update. However,
	// this is poorly defined, since the model is replaced, so the list indexes
	// get invalid (or at least meaningless).
	// For now, we select the object from the same table index as before.
	// Improvement: select the same column as before, but pick the row
	// according the the ID of the object stored there. What to do if the
	// selected ID disappeared?

	// Store the old index and ID
	QModelIndex oldTableIndex=ui.table->currentIndex ();

	list->replaceList (manager.getCache ().getObjects<T> ().getList ());

	QAbstractItemModel *model=ui.table->model ();

	// Note that the model has been replaced, so we may not reuse the old index
	if (oldTableIndex.row () < model->rowCount ())
		// The old row still exists. Select the old table index.
		ui.table->setCurrentIndex (model->index (oldTableIndex.row (), oldTableIndex.column ()));
	else
		// The old row doesn't exist any more. Select the same column in the
		// last row.
		ui.table->setCurrentIndex (model->index (model->rowCount ()-1, oldTableIndex.column (), oldTableIndex.parent ()));
}

/**
 * Not using the activated signal because it may be emitted on single click,
 * depending on the desktop settings.
 */
template<class T> void ObjectListWindow<T>::on_table_doubleClicked (const QModelIndex &index)
{
	if (!editPermission.permit (this)) return;

	if (index.isValid ())
	{
		QModelIndex listIndex=proxyModel->mapToSource (index);
		if (!listIndex.isValid ()) return;
		editObject (listModel->at (listIndex));
	}
	else
	{
		ui.actionNew->trigger ();
	}
}

/**
 * If this method is overridden in a subclass, the parent method should be
 * called first. Then, any subclass menu entries should be added, potentially
 * with a separator. Use activeObjectCount to determine the number of active
 * (selected, or current if none are selected) objects, not the selection
 * model or the current item.
 *
 * @param contextMenu
 * @param pos
 */
template<class T> void ObjectListWindow<T>::prepareContextMenu (QMenu *contextMenu)
{
	// Note that contextMenu refers to the parameter, not the private class
	// property
	// Improvement: pass the activeObjectCount to this method
	contextMenu->clear ();

	contextMenu->addAction (ui.actionNew);

	if (activeObjectCount ()>0)
	{
		contextMenu->addSeparator ();
		contextMenu->addAction (ui.actionEdit);
		contextMenu->addAction (ui.actionDelete);
	}
}

template<class T> void ObjectListWindow<T>::on_table_customContextMenuRequested (const QPoint &pos)
{
	prepareContextMenu (contextMenu);
	contextMenu->popup (ui.table->mapToGlobal (pos), 0);
}

template<class T> void ObjectListWindow<T>::keyPressEvent (QKeyEvent *e)
{
//	std::cout << "ObjectListWindow key " << e->key () << std::endl;

	int key=e->key ();

	// Return/enter triggers the edit action
	if (key==Qt::Key_Enter || key==Qt::Key_Return)
	{
		if (ui.table->hasFocus ())
		{
			ui.actionEdit->trigger ();
			return;
		}
	}

	ObjectListWindowBase::keyPressEvent (e);
}

template<class T> void ObjectListWindow<T>::languageChanged ()
{
	ObjectListWindowBase::languageChanged ();
	setupText ();
	ui.table->resizeColumnsToContents ();
}

template<class T> void ObjectListWindow<T>::searchClear () {
        // qDebug () << "ObjectListWindow<T>::searchClear: " << endl;
	ui.searchEdit->clear();
}

template<class T> void ObjectListWindow<T>::searchTextChanged (const QString& search) {
	//qDebug () << "ObjectListWindow<T>::searchTextChanged: " << search << endl;
	proxyModel->setFilterRegExp (QRegExp (search, Qt::CaseInsensitive, QRegExp::FixedString));
	ui.clearButton->setVisible (!search.isEmpty());
}


/**
 * Creates a new instance of ObjectListWindow for the given type
 *
 * The default implementation creates an instance of ObjectListWindow. This
 * method may be specialized for a specific type to create a subclass. For
 * example, ObjectListWindow<Person>::create is specialized to create a
 * PersonListWindow instance, which inherits ObjectListWindow<Person> and adds
 * person specific functionality.
 *
 * Specializations have to be performed at the end of the file, before (!) the
 * instantiation.
 *
 * WARNING: all ObjectListWindow instances have to be created using this
 * method (or a subclass constructor), never directly using the
 * ObjectListWindow constructor. Otherwise, the generic class may be created
 * instead of the specific class.
 *
 * Note that with GCC on Linux, it is possible to specialize this method in a
 * different compilation unit. This does not seem to be portable, mingw32
 * doesn't accept it (complains about a duplicate symbol).
 */
template<class T> ObjectListWindow<T> *ObjectListWindow<T>::create (DbManager &manager, QWidget *parent)
{
	// Potential improvement: prevent creating ObjectListWindow instances
	// except through this method (note that currently, the constructor has to
	// be called from subclass constructors.
	return new ObjectListWindow<T> (manager, parent);
}

template<class T> void ObjectListWindow<T>::refreshColumn (int column)
{
	listModel->refreshColumn (column);
}

// **********************************
// ** Create method specialization **
// **********************************

#include "src/gui/windows/objectList/PersonListWindow.h"
template<> ObjectListWindow<Person> *ObjectListWindow<Person>::create (DbManager &manager, QWidget *parent)
{
	return new PersonListWindow (manager, parent);
}


// *************************
// ** Class instantiation **
// *************************

// Instantiate the class templates
#include "src/model/Plane.h"
#include "src/model/Person.h"
#include "src/model/LaunchMethod.h"

template class ObjectListWindow<Plane>;
template class ObjectListWindow<Person>;
template class ObjectListWindow<LaunchMethod>;

