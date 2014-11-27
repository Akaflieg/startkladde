#include "qString.h"

#include "src/i18n/notr.h"

std::ostream &operator<< (std::ostream &s, const QString &c)
{
	return s << qPrintable (c);
}

/** Converts a std::string to a QString */
QString std2q (const std::string &s)
{
	return QString (s.c_str ());
}

/** Converts a QString to a std::string */
std::string q2std (const QString &s)
{
	if (s.isNull ()) return "";
	return std::string (s.toUtf8 ().constData ());
}
