#include "dialogs.h"

#include <QApplication>
#include <QInputDialog>

// TODO: this is sometimes used where an error or an info should be displayed instead;
void showWarning (const QString &title, const QString &text, QWidget *parent)
	/*
	 * Displays a warning dialog with the given text.
	 * Parameters:
	 *   - text: the text to display.
	 *   - parent: the parent widget, passed on to the dialog constructor.
	 */
{
	QMessageBox::warning (parent, title, text, QMessageBox::Ok, QMessageBox::NoButton);
}

bool yesNoQuestion (QWidget *parent, QString title, QString question)
{
	return yesNoQuestionStandardButton (parent, title, question)==QMessageBox::Yes;
}

QMessageBox::StandardButton yesNoQuestionStandardButton (QWidget *parent, QString title, QString question)
{
	QMessageBox::StandardButtons buttons=QMessageBox::Yes | QMessageBox::No;
	QMessageBox::StandardButton result=QMessageBox::question (parent, title, question, buttons, QMessageBox::Yes);
	return result;
}

QMessageBox::StandardButton yesNoCancelQuestion (QWidget *parent, QString title, QString question)
{
	QMessageBox::StandardButtons buttons=QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel;
	QMessageBox::StandardButton result=QMessageBox::question (parent, title, question, buttons, QMessageBox::Yes);
	return result;
}


bool confirmProblem (QWidget *parent, const QString title, const QString problem)
	/*
	 * Displays a message and ask the user if he wishes to accept anyway.
	 * Parameters:
	 *   - parent: passed on to the QMessageBox constructor.
	 *   - msg: the message.
	 * Return value:
	 *   - if the user accepted.
	 */
{
	// TODO: Buttons Yes/No, but with Esc handling
	QString question=qApp->translate ("dialogs", "%1 Accept anyway?").arg (problem);
	return yesNoQuestion (parent, title, question);
}

bool confirmProblem (QWidget *parent, const QString problem)
{
	return confirmProblem (parent, qApp->translate ("dialogs", "Warning"), problem);
}

bool verifyPassword (QWidget *parent, const QString &password, const QString &message)
{
	// TODO get rid of the "Please enter the password", it should be clear from
	// the message (verify all messages)
	QString title=qApp->translate ("dialogs", "Password required");
	QString label=qApp->translate ("dialogs", "%1 Please enter the password:").arg (message);

	// Keep asking the user for the password until he either enters the correct
	// password or cancels.
	while (true)
	{
		// Show a dialog asking the user for the password
		bool ok=false;
		QString enteredPassword=QInputDialog::getText (parent, title, label,
			QLineEdit::Password, QString (), &ok);

		// If the user canceled, return false
		if (!ok)
			return false;

		// If the entered password is correct, return true
		if (enteredPassword==password)
			return true;

		// The user neither canceled nor entered the correct password, or we
		// would have returned by now.
		label=qApp->translate ("dialogs", "The entered password is not correct. Please enter password:");
	}

	// This cannot be reached, but code analysis may not recognize this and warn
	// about reaching the end of the function. Also, it's more robust against
	// possible changes of this function to explicitly return a valid value
	// here.
	return false;
}
