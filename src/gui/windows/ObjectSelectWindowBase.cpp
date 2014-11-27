#include "ObjectSelectWindowBase.h"

ObjectSelectWindowBase::ObjectSelectWindowBase (QWidget *parent):
	SkDialog<Ui::ObjectSelectWindowBaseClass> (parent),
	resultId (invalidId),
	newItem (NULL), unknownItem (NULL)
{
	ui.setupUi (this);
	setModal (true);
}

ObjectSelectWindowBase::~ObjectSelectWindowBase ()
{
}

dbId ObjectSelectWindowBase::getResultId () const
{
	return resultId;
}

/**
 * Not using the itemActivated signal because it may be emitted on single
 * click, depending on the desktop settings.
 */
void ObjectSelectWindowBase::on_objectList_itemDoubleClicked (QTreeWidgetItem *item, int column)
{
	(void)column;
	if (item) accept ();
}
