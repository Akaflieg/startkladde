/*
 * ObjectEditorPane.h
 *
 *  Created on: Aug 22, 2009
 *      Author: Martin Herrmann
 */

#ifndef OBJECTEDITORPANE_H_
#define OBJECTEDITORPANE_H_

#include <iostream>

#include <QtGui/QWidget>

#include "src/db/dbId.h"
#include "src/gui/windows/objectEditor/ObjectEditorWindowBase.h" // Required for ObjectEditorWindowBase::Mode

class DbManager;
class Cache;

/**
 * TODO: the ObjectEditor/ObjectList classes are getting too complicated,
 * and hard to maintain, specifically as individual changes are required (e. g.
 * merging people, password protecting medical data etc.). Remove the templates,
 * use simple explicit classes instead (PersonEditorDialog etc.), potentially
 * inheriting from a template class. This will incur a small amount of code
 * duplication, but the current way is overengineered and inflexible.
 */

/**
 * A hack that allows passing data to an object editor pane, necessitated by the
 * unnecessarily complex templated object editor system
 */
class ObjectEditorPaneData
{
	public:
		virtual ~ObjectEditorPaneData () {}
};

/**
 * A Q_OBJECT base for the template ObjectEditorPane so we can use signals and
 * slots.
 */
class ObjectEditorPaneBase: public QWidget
{
		Q_OBJECT

	public:
		// Types
		class AbortedException: public std::exception {};

		ObjectEditorPaneBase (ObjectEditorWindowBase::Mode mode, DbManager &dbManager, QWidget *parent=NULL);
		virtual ~ObjectEditorPaneBase ();

	protected:
		virtual void errorCheck (const QString &problem, QWidget *widget);
		virtual void requiredField (const QString &value, QWidget *widget, const QString &problem);

		DbManager &dbManager;
		Cache &cache;
		ObjectEditorWindowBase::Mode mode;
};

/**
 * A QWidget with editor fields used to edit an object.
 *
 * This slightly complex design is to allow the creating the editor pane for
 * individual types with Designer.
 *
 * To implement this class, inherit from an instantiation of this template and
 * implement the abstract methods. You should also specialize the create<T>
 * method template, like this:
 *   template<> ObjectEditorPane<Foo> *ObjectEditorPane<Foo>::create (QWidget *parent) {
 *     return new PlaneEditorPane (parent);
 *   }
 * If the create method template is not specialized, the implementation can
 * still be used, but its type must be explicitly named; template classes such
 * as ObjectEditorWindow<T> use the create method template so they don't have
 * to be specialized.
 */
template<class T> class ObjectEditorPane: public ObjectEditorPaneBase
{
	public:
		ObjectEditorPane (ObjectEditorWindowBase::Mode mode, DbManager &dbManager, QWidget *parent=NULL);
		virtual ~ObjectEditorPane ();

		virtual void setObject (const T &object);
		// The API stinks
		virtual T determineObject (bool performChecks);

		/**
		 * Sets the "name" fields (e. g. last and first name for people, or
		 * registration for planes) to the values of the given object and makes
		 * them read-only, if supported by the implementation.
		 *
		 * @param nameObject
		 */
		virtual void setNameObject (const T &nameObject) { (void)nameObject; }

		/** @brief Implementations of ObjectEditorPane should specialize this template method */
		static ObjectEditorPane<T> *create (ObjectEditorWindowBase::Mode mode, DbManager &dbManager, QWidget *parent=NULL, ObjectEditorPaneData *paneData=NULL);

		// Make a copy
		T getOriginalObject () { return originalObject; }

	private:
		/**
		 * Writes the values of the object to the editor fields
		 */
		virtual void objectToFields (const T &object)=0;

		/**
		 * Writes the values of the editor fields to the object
		 */
		// TODO we should either return the object (and throw AbortedException on
		// failure) or return success as boolen.
		virtual void fieldsToObject (T &object, bool performChecks)=0;

		T originalObject;
};

// *************************************
// ** ObjectEditorPane implementation **
// *************************************

template<class T> ObjectEditorPane<T>::ObjectEditorPane (ObjectEditorWindowBase::Mode mode, DbManager &dbManager, QWidget *parent):
	ObjectEditorPaneBase (mode, dbManager, parent)
{
}

template<class T> ObjectEditorPane<T>::~ObjectEditorPane ()
{
}

/**
 * Writes the object properties to the input fields (and stores the original
 * object)
 *
 * @param object the object to write
 */
template<class T> void ObjectEditorPane<T>::setObject (const T &object)
{
	originalObject=object;

	objectToFields (object);
}

/**
 * Creates an object, using the values from the input fields (or defaults from
 * the original object). Throws AbortedException if the user aborts.
 *
 * @return the created object
 */
template<class T> T ObjectEditorPane<T>::determineObject (bool performChecks)
{
	T object (originalObject);

	fieldsToObject (object, performChecks);

	return object;
}

#endif
