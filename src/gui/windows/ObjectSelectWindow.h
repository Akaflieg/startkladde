/*
 * ObjectSelectWindow.h
 *
 *  Created on: 23.03.2010
 *      Author: deffi
 */

#ifndef OBJECTSELECTWINDOW_H_
#define OBJECTSELECTWINDOW_H_

#include "src/gui/windows/ObjectSelectWindowBase.h"

template<class T> class ObjectModel;

// TODO: This class is not retranslatable. Making it retranslatable is
// complicated because when called from FlightWindow, it is called with text
// that is dynamically generated. Since the window is modal, a language change
// should not happen while the window is open.
// Apart from the title and the label, the model headers and model data have to
// be retranslated as well, but that is easier.

/**
 * A dialog for letting the user select an object from a list. A "new" and
 * "unknown" entry are also given.
 *
 * This template must be instantiated for every class it is used with (at the
 * end of ObjectSelectWindow.cpp).
 */
template<class T> class ObjectSelectWindow: public ObjectSelectWindowBase
{
	public:
		ObjectSelectWindow (const QList<T> &objects, ObjectModel<T> *model, bool modelOwned, dbId selectedId, bool enableSpecialEntries, QWidget *parent=NULL);
		virtual ~ObjectSelectWindow ();

		static Result select (dbId *resultId, const QString &title, const QString &text, const QList<T> &objects, ObjectModel<T> *model, bool modelOwned, dbId preselectionId, bool enableSpecialEntries, QWidget *parent=NULL);

	private:
		ObjectModel<T> *model;
		bool modelOwned;
};

#endif
