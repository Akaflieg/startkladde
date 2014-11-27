#ifndef DESTRUCTIONMONITOR_H_
#define DESTRUCTIONMONITOR_H_

#include <iostream>

#include <QObject>

#include "src/util/qString.h"

class QString;

/**
 * Prints a message when a QObject is destroyed
 *
 * Use the static message() method to register an object.
 */
class DestructionMonitor: public QObject
{
		Q_OBJECT

	public:
		virtual ~DestructionMonitor ();

		// QObject must be an ancestor of T
		template<class T> static T *message (
			T *object, const QString &message=QString ());

	private slots:
		void objectDestroyed ();

	private:
		DestructionMonitor (QObject *object, const QString &message);

		QObject *_object;
		QString _message;

		QString _className;
		QString _objectName;
};

/**
 * Registers an object for deletion monitoring with a message to print
 *
 * T must be a subclass of QObject. Otherwise, a compilation error will occur.
 *
 * If the object is NULL, an error message is printed to standard error and the
 * object is not monitored.
 *
 * The specified message text will be included in the message when the object is
 * deleted.
 *
 * A copy of the object pointer is returned. This makes it possible to register
 * anonymous objects, like this:
 *     ui.tabWidget->addTab (DestructionMonitor::message (new MyTabWidget (...)));
 */
template<class T> T *DestructionMonitor::message (T *object, const QString &text)
{
	if (!object)
	{
		// The object is NULL
		if (text.isEmpty ())
			std::cerr << "Destruction monitor requested for NULL object" << std::endl;
		else
			std::cerr << "Destruction monitor requested for NULL object: " << text << std::endl;
	}
	else
	{
		// Will be deleted upon receiving the destroyed() signal from object
		new DestructionMonitor (object, text);
	}

	return object;
}

#endif
