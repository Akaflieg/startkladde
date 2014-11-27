#ifndef OBJECTSELECTWINDOWBASE_H
#define OBJECTSELECTWINDOWBASE_H

#include "src/db/dbId.h"
#include "src/gui/SkDialog.h"

#include "ui_ObjectSelectWindowBase.h"

class SkTreeWidgetItem;

class ObjectSelectWindowBase: public SkDialog<Ui::ObjectSelectWindowBaseClass>
{
	Q_OBJECT

	public:
		enum Result { resultCancelled, resultOk, resultNew, resultUnknown, resultNoneSelected };

		ObjectSelectWindowBase (QWidget *parent=NULL);
		~ObjectSelectWindowBase ();

		virtual dbId getResultId () const;

	protected slots:
		void on_objectList_itemDoubleClicked (QTreeWidgetItem *item, int column);

	protected:
		dbId resultId;
		SkTreeWidgetItem *newItem;
		SkTreeWidgetItem *unknownItem;
};

#endif
