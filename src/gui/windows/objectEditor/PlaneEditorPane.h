#ifndef PLANEEDITORPANE_H_
#define PLANEEDITORPANE_H_

#include "ObjectEditorPane.h"
#include "ui_PlaneEditorPane.h"

#include "src/gui/windows/objectEditor/ObjectEditorWindowBase.h" // Required for ObjectEditorWindowBase::Mode
#include "src/model/Plane.h"

class PlaneEditorPaneData: public ObjectEditorPaneData
{
	public:
		PlaneEditorPaneData (): registrationReadOnly (false), flarmIdReadOnly (false) {}
		bool registrationReadOnly;
		bool flarmIdReadOnly;
};

class PlaneEditorPane: public ObjectEditorPane<Plane>
{
    Q_OBJECT

	public:
		PlaneEditorPane (ObjectEditorWindowBase::Mode mode, DbManager &dbManager, QWidget *parent=NULL, PlaneEditorPaneData *paneData=NULL);
		virtual ~PlaneEditorPane();

		virtual void objectToFields (const Plane &plane);
		virtual void fieldsToObject (Plane &object, bool performChecks);
		virtual void setNameObject (const Plane &nameObject);

	public slots:
		virtual void on_registrationInput_editingFinished ();

	protected:
		virtual void loadData ();
		virtual void prepareText ();
		virtual void setupText ();
		virtual void changeEvent (QEvent *event);

	private:
		void checkFlarmId (const QString &newFlarmId, const QString &oldFlarmId);

		Ui::PlaneEditorPaneClass ui;
		PlaneEditorPaneData *paneData;
};


#endif // PLANEEDITORPANE_H
