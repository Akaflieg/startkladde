#ifndef CSV_H_
#define CSV_H_

#include <QString>

class QAbstractTableModel;

class Csv
{
	public:
		Csv (const QAbstractTableModel &model, const QString &separator);
		virtual ~Csv ();

		QString toString ();

	protected:
		QString escape (const QString &text) const;

	private:
		const QAbstractTableModel &model;

		const QString separator;
};

#endif
