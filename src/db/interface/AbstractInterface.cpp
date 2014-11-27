#include "AbstractInterface.h"

#include <cassert>
#include <iostream>

#include "src/util/qString.h"
#include "src/i18n/notr.h"

AbstractInterface::AbstractInterface (const DatabaseInfo &info):
	info (info)
{
}

AbstractInterface::~AbstractInterface ()
{
}



const DatabaseInfo &AbstractInterface::getInfo () const
{
	return info;
}

void AbstractInterface::setInfo (const DatabaseInfo &info)
{
	this->info=info;
}

QString AbstractInterface::transactionStatementString (TransactionStatement statement)
{
	switch (statement)
	{
		case transactionBegin   : return notr ("Transaction");
		case transactionCommit  : return notr ("Commit");
		case transactionRollback: return notr ("Rollback");
		// no default
	}

	assert (!notr ("Unhandled statement"));
	return notr ("?");
}
