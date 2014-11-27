#include "PasswordCheck.h"

#include "src/gui/dialogs.h"

/**
 * Creates a PasswordCheck with the specified password
 *
 * The password can later be changed using setPassword.
 *
 * @param password the password
 */
PasswordCheck::PasswordCheck (const QString &password):
	password (password), passwordOk (false)
{
}

PasswordCheck::~PasswordCheck ()
{
}

/**
 * Sets the password. Optionally also forgets the entered password state.
 *
 * @param password the new password to check in future calls to query
 * @param forget if true, the forget method is called
 */
void PasswordCheck::setPassword (const QString &password, bool forget)
{
	this->password=password;

	if (forget)
		this->forget ();
}

/**
 * Asks the user for the password
 *
 * @param message the message to display to the user in the password dialog
 * @return true if the correct password (specified on construction or set with
 *         setPassword) is entered, or false if the user cancels
 */
bool PasswordCheck::query (const QString &message, QWidget *parent)
{
	// If the correct password has been entered before, the operation is
	// permitted
	if (passwordOk) return true;

	// We need to ask the user for the password. askForPassword keeps asking
	// until the the entered password is correct or the user cancels.
	if (verifyPassword (parent, password, message))
	{
		// The entered password was correct. Set passwordOk, so subsequent
		// invocations of this method will not ask for the password again.
		passwordOk=true;

		return true;
	}
	else
	{
		// The user canceled password entry
		return false;
	}
}

/**
 * Forgets the entered password state
 *
 * The user will have to re-enter the password on the next call to query, even
 * if he already entered the correct password before.
 */
void PasswordCheck::forget ()
{
	passwordOk=false;
}
