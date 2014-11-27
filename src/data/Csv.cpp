#include "Csv.h"

#include <QAbstractTableModel>
#include <QStringList>

#include "src/i18n/notr.h"

Csv::Csv (const QAbstractTableModel &model, const QString &separator):
	model (model), separator (separator)
{
}

Csv::~Csv ()
{
}

/**
 * Replaces every quote by double quotes and encloses the string in double
 * quotes if it contains the separator or a quote.
 *
 * Examples, with separator==",":
 *   foo           => foo
 *   foo "bar" baz => "foo ""bar"" baz"
 *   foo, bar      => "foo, bar"
 *   "foo"         => """foo"""
 *   "foo, bar"    => """foo, bar"""
 *
 *
 */
QString Csv::escape (const QString &text) const
{
	QString result=text;

	QString quote      =notr ("\"");
	QString doubleQuote=notr ("\"\"");

	// Replace quotes with double quotes
	result.replace (quote, doubleQuote);

	// Quote the text if it contains a quote or the separator
	if (result.contains (separator) or result.contains (quote))
		result=quote+result+quote;

	return result;
}

QString Csv::toString ()
{
	int rows=model.rowCount ();
	int columns=model.columnCount ();

	QStringList items;
	QStringList lines;

	// Header
	for (int column=0; column<columns; ++column)
	{
		QVariant value=model.headerData (column, Qt::Horizontal);
		items.append (escape (value.toString ()));
	}
	lines.append (items.join (separator));
	items.clear ();

	// Body
	for (int row=0; row<rows; ++row)
	{
		for (int column=0; column<columns; ++column)
		{
			QVariant value=model.data (model.index (row, column));
			items.append (escape (value.toString ()));
		}
		lines.append (items.join (separator));
		items.clear ();
	}

	QString newline=notr ("\n");
	return lines.join (newline)+newline;
}
