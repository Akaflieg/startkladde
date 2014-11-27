#include "SkTreeWidgetItem.h"

SkTreeWidgetItem::SkTreeWidgetItem (QTreeWidget *parent, QTreeWidgetItem *after, QString text)
	:QTreeWidgetItem (parent, after)
{
	setText (0, text);
}

SkTreeWidgetItem::SkTreeWidgetItem (QTreeWidget *parent, QTreeWidgetItem *after)
	:QTreeWidgetItem (parent, after)
{

}

SkTreeWidgetItem::SkTreeWidgetItem (QTreeWidget *parent, QString text)
	:QTreeWidgetItem (parent)
{
	setText (0, text);
}

SkTreeWidgetItem::SkTreeWidgetItem (QTreeWidget *parent)
	:QTreeWidgetItem (parent)
{
}
