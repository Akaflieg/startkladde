#include "PasswordPermission.h"

/**
 * Creates a PasswordPermission, using the given PasswordCheck
 */
PasswordPermission::PasswordPermission (PasswordCheck &passwordCheck):
	passwordCheck (passwordCheck),
	passwordRequired (false)
{

}

PasswordPermission::~PasswordPermission ()
{
}

/**
 * Sets whether the password is required
 *
 * @see permit
 */
void PasswordPermission::setPasswordRequired (bool required)
{
	passwordRequired=required;
}

bool PasswordPermission::getPasswordRequired () const
{
	return passwordRequired;
}

void PasswordPermission::setMessage (const QString &message)
{
	this->message=message;
}

/**
 * Determines whether the operation is permitted
 *
 * If the password is not required (see setPasswordRequired), the operation is
 * always permitted. If the password is required, the operation is permitted if
 * the correct password is entered, has been entered before, as determined by
 * the associated PasswordCheck.
 *
 * @param parent
 * @return true if the operation is permitted, false otherwise
 */
bool PasswordPermission::permit (QWidget *parent)
{
	if (!passwordRequired)
		return true;

	return passwordCheck.query (message, parent);
}
