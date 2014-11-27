#ifndef INFOPLUGINSELECTIONDIALOG_H
#define INFOPLUGINSELECTIONDIALOG_H

#include "src/gui/SkDialog.h"
#include "src/plugin/info/InfoPlugin.h"

#include "ui_InfoPluginSelectionDialog.h"

template<class T> class QList;

class InfoPluginSelectionDialog: public SkDialog<Ui::InfoPluginSelectionDialogClass>
{
		Q_OBJECT

	public:
		InfoPluginSelectionDialog (const QList<const InfoPlugin::Descriptor *> &plugins, QWidget *parent=NULL);
		~InfoPluginSelectionDialog ();

		void setup ();
		void setupText ();
		const InfoPlugin::Descriptor *getCurrentPluginDescriptor ();

		static const InfoPlugin::Descriptor *select (const QList<const InfoPlugin::Descriptor *> &plugins, QWidget *parent=NULL);

	protected:
		virtual void languageChanged ();

	private:
		QList<const InfoPlugin::Descriptor *> plugins;
};

#endif
