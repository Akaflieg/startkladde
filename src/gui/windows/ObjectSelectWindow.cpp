#include "ObjectSelectWindow.h"

/*
 * Improvements:
 *  - enable sorting (beware the special entries "unknown" and "new")
 */

#include <iostream>

#include "src/gui/widgets/SkTreeWidgetItem.h"
//#include "src/logging/messages.h"
#include "src/text.h"
#include "src/util/qString.h"

#include "src/model/objectList/ObjectModel.h"

template<class T> ObjectSelectWindow<T>::ObjectSelectWindow (const QList<T> &objects, ObjectModel<T> *model, bool modelOwned, dbId selectedId, bool enableSpecialEntries, QWidget *parent):
	ObjectSelectWindowBase (parent),
	model (model), modelOwned (modelOwned)
{
	QString title;

	QStringList headerLabels;
	for (int i=0; i<model->columnCount (); ++i)
		headerLabels << model->headerData (i).toString ();

	ui.objectList->setHeaderLabels (headerLabels);

	if (enableSpecialEntries)
	{
		unknownItem=new SkTreeWidgetItem (ui.objectList, qApp->translate ("ObjectSelectWindow<T>", "(Unknown)"));
		unknownItem->setFirstColumnSpanned (true);

		newItem=new SkTreeWidgetItem (ui.objectList, qApp->translate ("ObjectSelectWindow<T>", "(Create new)"));
		newItem->setFirstColumnSpanned (true);

		ui.objectList->setCurrentItem (unknownItem);
	}

	int numColumns=ui.objectList->columnCount ();
	foreach (const T &object, objects)
	{
		SkTreeWidgetItem *item=new SkTreeWidgetItem (ui.objectList);
		item->id=object.getId ();

		for (int i=0; i<numColumns; ++i)
			item->setData (i, Qt::DisplayRole, model->data (object, i));

		if (!idInvalid (object.getId ()) && object.getId ()==selectedId)
			ui.objectList->setCurrentItem (item);
	}

	// Resize all columns to their contents
	for (int i=0; i<numColumns; ++i)
		ui.objectList->resizeColumnToContents (i);
}

template<class T> ObjectSelectWindow<T>::~ObjectSelectWindow ()
{
	if (modelOwned) delete model;
}

template<class T> ObjectSelectWindowBase::Result ObjectSelectWindow<T>::select
	(dbId *resultId, const QString &title, const QString &text, const QList<T> &objects, ObjectModel<T> *model, bool modelOwned, dbId preselectionId, bool enableSpecialEntries, QWidget *parent)
{
	ObjectSelectWindow<T> window (objects, model, modelOwned, preselectionId, enableSpecialEntries, parent);

	window.setWindowTitle (title);

	window.ui.textLabel->setText (text);
	if (isBlank (text)) window.ui.textLabel->setVisible (false);


	int result=window.exec ();

	if (result==QDialog::Rejected)
	{
		return resultCancelled;
	}
	else
	{
		QList<QTreeWidgetItem *> selected=window.ui.objectList->selectedItems ();
		if (selected.empty ())
			return resultNoneSelected;
		else if (!selected[0])
			return resultNoneSelected;
		else if (selected[0]==window.newItem)
			return resultNew;
		else if (selected[0]==window.unknownItem)
			return resultUnknown;
		else
		{
			// There may only be sk_list_view_items in the list.
			if (resultId) *resultId=(dynamic_cast<SkTreeWidgetItem *> (selected[0]))->id;
			return resultOk;
		}
	}
}


// Instantiate the class templates
#include "src/model/Person.h"

template class ObjectSelectWindow<Person>;
