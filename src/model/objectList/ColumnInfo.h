#ifndef COLUMNINFO_H_
#define COLUMNINFO_H_

#include <QString>

/**
 * Gives the column a name (which has nothing to do with the title)
 *
 * This can, for example, be used for storing the column widths.
 */
class ColumnInfo
{
	public:
		virtual ~ColumnInfo () {}

		virtual int columnCount () const=0;
		virtual QString columnName (int columnIndex) const=0;
		virtual QString sampleText (int columnIndex) const=0;
};

#endif
