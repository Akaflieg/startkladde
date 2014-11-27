#include "ObjectListWindowBase.h"

#include <iostream>

#include <QKeyEvent>
#include <QPushButton>

#include "src/util/qString.h"
#include "src/config/Settings.h"

ObjectListWindowBase::ObjectListWindowBase (DbManager &manager, QWidget *parent):
	SkMainWindow<Ui::ObjectListWindowBaseClass> (parent),
	manager (manager), databasePasswordCheck (), editPermission (databasePasswordCheck)
{
        // setup the gui in the base class to allow connect to gui elements
        ui.setupUi(this);
        
        // Clear search button
        QStyle* style = QApplication::style ();
        //ui.clearButton->setIcon (style->standardIcon (QStyle::SP_DialogDiscardButton));
        ui.clearButton->setIcon (style->standardIcon (QStyle::SP_DialogCloseButton));
        ui.clearButton->setVisible (false);
        
	QObject::connect (&manager, SIGNAL (stateChanged (DbManager::State)), this, SLOT (databaseStateChanged (DbManager::State)));
        QObject::connect (ui.searchEdit,  SIGNAL(textChanged(const QString&)), this, SLOT(searchTextChanged(const QString&)));
	QObject::connect (ui.clearButton, SIGNAL(pressed()), this, SLOT(searchClear()));
                
	databasePasswordCheck.setPassword (Settings::instance ().databaseInfo.password);
}

ObjectListWindowBase::~ObjectListWindowBase()
{
}

void ObjectListWindowBase::on_actionClose_triggered ()
{
	close ();
}

void ObjectListWindowBase::keyPressEvent (QKeyEvent *e)
{
	// KeyEvents are accepted by default
	switch (e->key ())
	{
		case Qt::Key_F2: ui.actionNew->trigger (); break;
		case Qt::Key_F4: ui.actionEdit->trigger (); break;
		case Qt::Key_F8: ui.actionDelete->trigger (); break;
		case Qt::Key_F12: ui.actionRefresh->trigger (); break;
		case Qt::Key_Escape: ui.actionClose->trigger(); break;
		case Qt::Key_Insert: ui.actionNew->trigger (); break;
		case Qt::Key_Delete: ui.actionDelete->trigger (); break;
		default: e->ignore (); break;
	}

	if (!e->isAccepted ()) QMainWindow::keyPressEvent (e);
}

void ObjectListWindowBase::databaseStateChanged (DbManager::State state)
{
	if (state==DbManager::stateDisconnected)
		close ();
}
                                