#include "InfoPluginSelectionDialog.h"

#include <QPushButton>

const int nameColumn=0;
const int descriptionColumn=1;

InfoPluginSelectionDialog::InfoPluginSelectionDialog (const QList<const InfoPlugin::Descriptor *> &plugins, QWidget *parent):
    SkDialog<Ui::InfoPluginSelectionDialogClass> (parent),
    plugins (plugins)
{
	ui.setupUi (this);

	setup ();
}

InfoPluginSelectionDialog::~InfoPluginSelectionDialog()
{
}

void InfoPluginSelectionDialog::setup ()
{
	ui.pluginList->clear ();

	for (int i=0, n=plugins.size (); i<n; ++i)
		new QTreeWidgetItem (ui.pluginList);

	setupText ();

	ui.buttonBox->button (QDialogButtonBox::Ok)->setEnabled (!plugins.isEmpty ());
}

void InfoPluginSelectionDialog::setupText ()
{
	QTreeWidgetItem *rootItem=ui.pluginList->invisibleRootItem ();

	for (int i=0, n=plugins.size (); i<n; ++i)
	{
		QTreeWidgetItem *item=rootItem->child (i);
		item->setData (nameColumn       , Qt::DisplayRole, plugins[i]->getName        ());
		item->setData (descriptionColumn, Qt::DisplayRole, plugins[i]->getDescription ());
	}

	for (int i=0; i<ui.pluginList->columnCount (); ++i)
		ui.pluginList->resizeColumnToContents (i);

}

const InfoPlugin::Descriptor *InfoPluginSelectionDialog::getCurrentPluginDescriptor ()
{
 	QTreeWidget *list=ui.pluginList;
	int row=ui.pluginList->indexOfTopLevelItem (list->currentItem ());

	if (row<0 || row>=list->topLevelItemCount ())
		return NULL;
	else
		return plugins[row];
}

const InfoPlugin::Descriptor *InfoPluginSelectionDialog::select (const QList<const InfoPlugin::Descriptor *> &plugins, QWidget *parent)
{
	QList<const InfoPlugin::Descriptor *> sortedPlugins (plugins);
	qSort (sortedPlugins.begin (), sortedPlugins.end (), InfoPlugin::Descriptor::nameLessThanP);
	InfoPluginSelectionDialog *dialog=new InfoPluginSelectionDialog (sortedPlugins, parent);
	dialog->setModal (true);

	const InfoPlugin::Descriptor *result=NULL;
	if (dialog->exec ()==QDialog::Accepted)
		result=dialog->getCurrentPluginDescriptor ();

	delete dialog;

	return result;
}

void InfoPluginSelectionDialog::languageChanged ()
{
	SkDialog<Ui::InfoPluginSelectionDialogClass>::languageChanged ();
	setupText ();
}
