#ifndef CONFIRMOVERWRITEPERSONDIALOG_H
#define CONFIRMOVERWRITEPERSONDIALOG_H

#include "src/gui/SkDialog.h"
#include "src/model/Person.h"

#include "ui_ConfirmOverwritePersonDialog.h"

template<class T> class QList;
template<class T> class ObjectModel;
template<class T> class ObjectListModel;
template<class T> class MutableObjectList;

/**
 * A window to let the user confirm the merging of people
 *
 * The window displays the wrong entries and the correct one and informs the
 * user about the consequences of merging. The user can confirm or cancel the
 * process.
 */
class ConfirmOverwritePersonDialog: public SkDialog<Ui::ConfirmOverwritePersonDialogClass>
{
		Q_OBJECT

	public:
		ConfirmOverwritePersonDialog (QWidget *parent=NULL, Qt::WindowFlags f=Qt::Widget);
		~ConfirmOverwritePersonDialog ();

		static bool confirmOverwrite (const Person &correctPerson, const QList<Person> &wrongPeople, QWidget *parent=NULL);

	protected:
		void setup (const Person &correctPerson, const QList<Person> &wrongPeople);
		void setupTexts ();
		virtual void languageChanged ();

	private:
		Person correctPerson;
		QList<Person> wrongPeople;

		QTreeWidgetItem *  wrongParentItem;
		QTreeWidgetItem *correctParentItem;

		// The model to get the data from
		Person::DefaultObjectModel model;

};

#endif
