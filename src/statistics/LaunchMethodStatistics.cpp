#include "LaunchMethodStatistics.h"

#include "src/model/LaunchMethod.h"
#include "src/model/Flight.h"
#include "src/db/cache/Cache.h"
#include "src/util/qString.h"

// ************************
// ** Entry construction **
// ************************

LaunchMethodStatistics::Entry::Entry ():
	num (0)
{
}

LaunchMethodStatistics::Entry::~Entry ()
{
}

// ******************
// ** Construction **
// ******************

LaunchMethodStatistics::LaunchMethodStatistics (QObject *parent):
	QAbstractTableModel (parent)
{
}

LaunchMethodStatistics::~LaunchMethodStatistics ()
{
}

// **************
// ** Creation **
// **************

LaunchMethodStatistics *LaunchMethodStatistics::createNew (const QList<Flight> &flights, Cache &cache)
{
	QMap<dbId, int> map;
	int numTowFlights=0;

	foreach (const Flight &flight, flights)
	{
		if (flight.happened ())
		{
			if (flight.isTowflight ())
				++numTowFlights;
			else
				// Non-existing values are initialized to 0
				++map[flight.getLaunchMethodId ()];
		}
	}

	// Get and sort the launch methods
	QList<LaunchMethod> launchMethods=cache.getObjects<LaunchMethod> (map.keys (), true);
    std::sort (launchMethods.begin (), launchMethods.end (), LaunchMethod::nameLessThan);

	// Create the entries for the launch methods
	LaunchMethodStatistics *result=new LaunchMethodStatistics ();
	foreach (const LaunchMethod &launchMethod, launchMethods)
	{
		Entry entry;
		entry.name=launchMethod.name;
		entry.num=map[launchMethod.getId ()];
		result->entries.append (entry);
	}

	// Add the entry for the towflights
	// Towflights do not have a stored launch method and there can be more than
	// one self launch method. Thus, we display an extra entry for the
	// towflights.
	if (numTowFlights>0)
	{
		Entry towflightsEntry;
		towflightsEntry.name=tr ("Towflights");
		towflightsEntry.num=numTowFlights;
		result->entries.append (towflightsEntry);
	}

	return result;
}

// *********************************
// ** QAbstractTableModel methods **
// *********************************

int LaunchMethodStatistics::rowCount (const QModelIndex &index) const
{
	if (index.isValid ())
		return 0;

	return entries.size ();
}

int LaunchMethodStatistics::columnCount (const QModelIndex &index) const
{
	if (index.isValid ())
		return 0;

	return 2;
}

QVariant LaunchMethodStatistics::data (const QModelIndex &index, int role) const
{
	const Entry &entry=entries[index.row ()];

	if (role==Qt::DisplayRole)
	{
		switch (index.column ())
		{
			case 0: return entry.name;
			case 1: return entry.num;
			default: assert (false); return QVariant ();
		}
	}
	else
		return QVariant ();
}

QVariant LaunchMethodStatistics::headerData (int section, Qt::Orientation orientation, int role) const
{
	if (role==Qt::DisplayRole)
	{
		if (orientation==Qt::Horizontal)
		{
			switch (section)
			{
                case 0: return tr ("Launch method");
                case 1: return tr ("Number of launches");
			}
		}
		else
		{
			return section+1;
		}
	}

	return QVariant ();
}
