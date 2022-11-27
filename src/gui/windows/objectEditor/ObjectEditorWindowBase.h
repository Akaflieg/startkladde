/*
 * ObjectEditorWindowBase.h
 *
 *  Created on: Aug 23, 2009
 *      Author: Martin Herrmann
 */

#ifndef OBJECTEDITORWINDOWBASE_H_
#define OBJECTEDITORWINDOWBASE_H_

#include "src/gui/SkDialog.h"

#include "ui_ObjectEditorWindowBase.h"

class DbManager;

/**
 * Base class for ObjectEditorWindow, because templates cannot be Q_OBJECTs
 * TODO document for all, also: what goes into the base class and what not:
 *   - signals/slots must be declared in base
 *   - non-template stuff should be declared in base because it does not have
 *     to be in the header
 *   - template stuff must be declared in the template
 */
class ObjectEditorWindowBase: public SkDialog<Ui::ObjectEditorWindowBaseClass>
{
	Q_OBJECT

	public:
		// Types
		// modeDisplay is not tested
		enum Mode { modeCreate, modeEdit/*, modeDisplay*/ };

		ObjectEditorWindowBase (DbManager &manager, QWidget *parent=NULL, Qt::WindowFlags flags=Qt::Widget);
		virtual ~ObjectEditorWindowBase ();

	public slots:
		virtual void on_buttonBox_accepted ()=0;

	protected:
		DbManager &manager;
};


#endif
