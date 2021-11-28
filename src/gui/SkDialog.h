#ifndef SKDIALOG_H_
#define SKDIALOG_H_

#include <QDialog>
#include <QEvent>

/**
 * A base class for Designer created, QDialog based classes
 *
 * This is essentially the same as SkMainWindow, only for QDialog.
 */
template<class UiClass> class SkDialog: public QDialog
{
	public:
		/**
		 * Initializes the QDialog base class
		 */
    SkDialog (QWidget *parent=NULL, Qt::WindowFlags f={}):
			QDialog (parent, f)
		{
		}

		virtual ~SkDialog ()
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
				QDialog::changeEvent (event);
		}
};

#endif

