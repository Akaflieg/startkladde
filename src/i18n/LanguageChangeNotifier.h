#ifndef LANGUAGECHANGENOTIFIER_H_
#define LANGUAGECHANGENOTIFIER_H_

#include <QWidget>

/**
 * Notifies objects of language change events
 *
 * QWidgets get a QEvent::LanguageChange event on language change. For
 * notifying non-qwidgets about a lanugage change, this class can be used.
 *
 * This class is a singleton QWidget which emits a signal on language change.
 * To use this class, the object to be notified ("subscriber") must provide a
 * languageChanged() slot. The static method LanguageChangeNotifier::subscribe
 * connects the signal from the singleton object to the languageChanged slot of
 * the subscriber.
 */
class LanguageChangeNotifier: public QWidget
{
		Q_OBJECT

	public:
		virtual ~LanguageChangeNotifier ();
		static void subscribe (QObject *subscriber);

	signals:
		void languageChanged ();

	protected:
		virtual void changeEvent (QEvent *event);

	private:
		static LanguageChangeNotifier *instance;

		LanguageChangeNotifier ();
};

#endif
