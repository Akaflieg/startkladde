#include "CopiedResult.h"

#include <QSqlQuery>
#include <QSqlRecord>

/**
 * Copies the data from the given Result
 *
 * @result the Result to copy the data from; the data will be copied
 *         starting at the current position of result and the current
 *         position of result will not be reset.
 */
CopiedResult::CopiedResult (Result &result)
{
	// Make the copy
	while (result.next ())
		records.append (result.record ());

	_lastQuery=result.lastQuery ();
	_numRowsAffected=result.numRowsAffected ();
	_lastInsertId=result.lastInsertId ();

	current=QSql::BeforeFirstRow;
}

CopiedResult::~CopiedResult ()
{
}

int CopiedResult::at () const
{
	return current;
}

bool CopiedResult::first ()
{
	if (records.isEmpty ())
	{
		current=QSql::BeforeFirstRow;
		return false;
	}
	else
	{
		current=0;
		return true;
	}
}

bool CopiedResult::isNull (int field) const
{
	return records.at (current).isNull (field);
}

bool CopiedResult::last ()
{
	if (records.isEmpty ())
	{
		current=QSql::AfterLastRow;
		return false;
	}
	else
	{
		current=records.size ()-1;
		return true;
	}
}

QVariant CopiedResult::lastInsertId () const
{
	return _lastInsertId;
}

QString CopiedResult::lastQuery () const
{
	return _lastQuery;
}

bool CopiedResult::next ()
{
	++current;

	if (current<records.size ())
	{
		return true;
	}
	else
	{
		current=QSql::AfterLastRow;
		return false;
	}
}

int CopiedResult::numRowsAffected () const
{
	return _numRowsAffected;
}

bool CopiedResult::previous ()
{
	--current;

	if (current<0)
	{
		current=QSql::BeforeFirstRow;
		return false;
	}
	else
	{
		return true;
	}
}

QSqlRecord CopiedResult::record () const
{
	return records.at (current);
}

bool CopiedResult::seek (int index, bool relative)
{
	// See QSqlQuery#seek

	if (relative)
	{
		// If relative is true, the following rules apply:
		if (index<0 && (current==0 || current==QSql::BeforeFirstRow))
		{
			// If the result is currently positioned before the first
			// record or on the first record, and index is negative,
			// there is no change, and false is returned.
			return false;
		}
		if (index>0 && (current>records.size () || current==QSql::AfterLastRow))
		{
			// If the result is currently located after the last
			// record, and index is positive, there is no change, and
			// false is returned.
			return false;
		}
		else
		{
			// If the result is currently located somewhere in the
			// middle, and the relative offset index moves the result
			// below zero, the result is positioned before the first
			// record and false is returned.
			// Otherwise, an attempt is made to move to the record
			// index records ahead of the current record (or index
			// records behind the current record if index is negative).
			// If the record at offset index could not be retrieved,
			// the result is positioned after the last record if index
			// >= 0, (or before the first record if index is negative),
			// and false is returned. If the record is successfully
			// retrieved, true is returned.
			return seek (current+index, false);
		}
	}
	else
	{
		// If relative is false [...], the following rules apply:
		if (index<0)
		{
			// If index is negative, the result is positioned before the
			// first record and false is returned.
			current=QSql::BeforeFirstRow;
			return false;
		}
		else if (index<records.size ())
		{
			// Otherwise, an attempt is made to move to the record at
			// position index. If the record at position index could
			// not be retrieved, the result is positioned after the
			// last record and false is returned.
			current=index;
			return true;
		}
		else
		{
			// If the record is successfully retrieved, true is
			// returned.
			current=QSql::AfterLastRow;
			return false;
		}
	}
}

int CopiedResult::size () const
{
	return records.size ();
}

QVariant CopiedResult::value (int index) const
{
	if (current<0 || current>=records.size ())
		return QVariant ();
	else
		return records.at (current).value (index);
}
