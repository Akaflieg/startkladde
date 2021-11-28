#ifndef SKMAINWINDOW_H_
#define SKMAINWINDOW_H_

/**
 * A base class for Designer created, QMainWindow based classes
 *
 * This is essentially the same as SkDialog, only for QMainWindow.
 *
 * This class inherits and enhances QMainWindow. It handles storage of the
 * designer-created UI (in the "ui" property).
 *
 * When a QEvent::LanguageChange event is received, the languageChanged
 * method is invoked. The default implementation of languageChanged
 * retranslates the GUI by calling ui.retranslateUi. Implementations can
 * override languageChanged to retranslate dynamically generated strings.
 *
 * To use this class, inherit from SkMainWindow, specifying the Designer
 * created UI class a template parameter. The Designer UI class is
 * available as protected property "ui". You must call "ui.setupUi (this)"
 * to initialize the GUI (see below for an explanation why).
 * To be notified of language changes, override the languageChanged method
 * and call SkDialog::languageChanged before retranslating dynamically
 * generated strings. To be notified of other change events, override the
 * changeEvent method as usual.
 *
 * Note that Designer created classes do not inherit from a common base
 * class, so retranslateUi can not be called polymorphically. Therefore, it
 * is necessary to use a template class to call retranslateUi by duck
 * typing.
 *
 * Implementations must call ui.setupUi (this) in their constructor. This cannot
 * be done by the SkMainWindow constructor because the automatically connected
 * slots (on_object_signal) will not be connected. This is probably because in
 * any base class constructor, virtual methods of the derived class can not be
 * used yet (cf. [1]).
 *
 * [1] http://www.parashift.com/c++-faq-lite/strange-inheritance.html#faq-23.5
 */
template<class UiClass> class SkMainWindow: public QMainWindow
{
	public:
		/**
		 * Initializes the QMainWindow base class
		 */
		SkMainWindow (QWidget *parent=NULL, Qt::WindowFlags f={}):
			QMainWindow (parent, f)
		{
		}

		virtual ~SkMainWindow ()
		{
		}

	protected:
		UiClass ui;

		/**
		 * Called when a QEvent::LanguageChange is received. Can be
		 * overridden to provide custom retranslation.
		 */
		virtual void languageChanged ()
		{
			// The language changed. Retranslate the UI.
			ui.retranslateUi (this);
		}

		virtual void changeEvent (QEvent *event)
		{
			if (event->type () == QEvent::LanguageChange)
				languageChanged ();
			else
				QMainWindow::changeEvent (event);
		}
};

#endif
