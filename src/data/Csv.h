#ifndef CSV_H_
#define CSV_H_

#include <QString>
#include "src/itemDataRoles.h"

class QAbstractItemModel;

class Csv
{
	public:
        Csv (const QAbstractItemModel &model, const QString &separator);
		virtual ~Csv ();

		QString toString ();

	protected:
		QString escape (const QString &text) const;

	private:
        const QAbstractItemModel &model;

		const QString separator;
};

#endif
