/*
 * IndexSpec.h
 *
 *  Created on: 27.04.2010
 *      Author: Martin Herrmann
 */

#ifndef INDEXSPEC_H_
#define INDEXSPEC_H_

#include <QString>
#include <QList>

class IndexSpec
{
	public:
		IndexSpec (const QString &table, const QString &name, const QString &columns);
//		static IndexSpec singleColumn (const QString &table, const QString &column);
		virtual ~IndexSpec ();

		const QString &getTable   () const { return table  ; }
		const QString &getName    () const { return name   ; }
		const QString &getColumns () const { return columns; }

		virtual QString createClause () const;
		static QString createClause (const QList<IndexSpec> &list);

	private:
		QString table;
		QString name;
		QString columns;
};

#endif

