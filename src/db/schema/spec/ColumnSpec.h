/*
 * ColumnSpec.h
 *
 *  Created on: 14.03.2010
 *      Author: Martin Herrmann
 */

#ifndef COLUMNSPEC_H_
#define COLUMNSPEC_H_

#include <QString>

template<class T> class QList;

class ColumnSpec
{
	public:
		ColumnSpec (const QString &name, const QString &type, const QString &extra=QString ());
		virtual ~ColumnSpec ();

		virtual QString createClause () const;
		static QString createClause (const QList<ColumnSpec> &list);

	private:
		QString name;
		QString type;
		QString extra;
};

#endif /* COLUMNSPEC_H_ */
