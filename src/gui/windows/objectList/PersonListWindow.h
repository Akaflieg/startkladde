#ifndef PERSONLISTWINDOW_H_
#define PERSONLISTWINDOW_H_

#include "src/gui/windows/objectList/ObjectListWindow.h"
#include "src/gui/PasswordCheck.h"
#include "src/model/Person.h"

class QAction;

class DbManager;

class PersonListWindow: public ObjectListWindow<Person>
{
	Q_OBJECT

	public:
		PersonListWindow (DbManager &manager, QWidget *parent=NULL);
		virtual ~PersonListWindow ();

	protected:
		int editObject (const Person &object);

		virtual void prepareContextMenu (QMenu *contextMenu);

		void languageChanged ();

	protected slots:
		void mergeAction_triggered ();
		void displayMedicalDataAction_triggered ();

	private:
		Person::DefaultObjectModel *personModel;

		void setupText ();

		QAction *mergeAction;
		QAction *displayMedicalDataAction; // FIXME to context menu?

		PasswordPermission mergePermission;
		PasswordPermission viewMedicalDataPermission;
		PasswordPermission changeMedicalDataPermission;
};

#endif
