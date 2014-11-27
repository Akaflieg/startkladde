#ifndef SKTREEWIDGETITEM_H_
#define SKTREEWIDGETITEM_H_

#include <QTreeWidget>
#include <QString>

#include "src/db/dbId.h"

class SkTreeWidgetItem:public QTreeWidgetItem
{
	public:
		SkTreeWidgetItem (QTreeWidget *parent, QTreeWidgetItem *after, QString text);
		SkTreeWidgetItem (QTreeWidget *parent, QTreeWidgetItem *after);
		SkTreeWidgetItem (QTreeWidget *parent, QString text);
		SkTreeWidgetItem (QTreeWidget *parent);

		dbId id;
};

#endif

