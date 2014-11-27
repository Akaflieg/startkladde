#ifndef PERSONEDITORPANE_H_
#define PERSONEDITORPANE_H_

#include "ObjectEditorPane.h"
#include "ui_PersonEditorPane.h"

#include "src/gui/windows/objectEditor/ObjectEditorWindowBase.h" // Required for ObjectEditorWindowBase::Mode
#include "src/model/Person.h"

class PasswordPermission;

class PersonEditorPaneData: public ObjectEditorPaneData
{
	public:
		bool displayMedicalData;
		PasswordPermission *viewMedicalDataPermission;
		PasswordPermission *changeMedicalDataPermission;
};


class PersonEditorPane: public ObjectEditorPane<Person>
{
	Q_OBJECT

	public:
		PersonEditorPane (ObjectEditorWindowBase::Mode mode, DbManager &dbManager, QWidget *parent=NULL, PersonEditorPaneData *paneData=NULL);
		virtual ~PersonEditorPane();

		virtual void objectToFields (const Person &person);
		virtual void fieldsToObject (Person &object, bool performChecks);

		void setNameObject (const Person &nameObject);

	public slots:
		void on_medicalValidityUnknownCheckbox_toggled ();
		void on_checkMedicalInput_currentIndexChanged ();
		void on_displayMedicalValidityButton_clicked ();

	protected:
		void setMedicalValidityDisplayed (bool displayed);
		virtual void loadData ();
		virtual void setupText ();
		QDate getEffectiveMedicalValidity ();
		virtual void changeEvent (QEvent *event);

	private:
		Ui::PersonEditorPaneClass ui;
		PersonEditorPaneData *paneData;
};

#endif // PERSONEDITORPANE_H
