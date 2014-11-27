#ifndef PASSWORDCHECK_H_
#define PASSWORDCHECK_H_

#include <QString>

class QWidget;

/**
 * Queries a password from the user upon request
 *
 * If the correct password has been entered before, the user is not queried
 * again, unless the forget method has been called inbetween.
 */
class PasswordCheck
{
	public:
		PasswordCheck (const QString &password="");
		virtual ~PasswordCheck ();

		void setPassword (const QString &password, bool forget=true);

		bool query (const QString &message, QWidget *parent);
		void forget ();

	private:
		QString password;
		bool passwordOk;
};

#endif
