#include <src/flarm/FlarmRecordModel.h>

#include <iostream>

#include <QApplication>

#include "src/flarm/FlarmRecord.h"
#include "src/i18n/notr.h"
#include "src/util/qString.h"
#include "src/numeric/Velocity.h"

FlarmRecordModel::FlarmRecordModel ()
{
}

FlarmRecordModel::~FlarmRecordModel ()
{
}

int FlarmRecordModel::columnCount () const
{
	return 6;
}

QVariant FlarmRecordModel::displayHeaderData (int column) const
{
	switch (column)
	{
		case 0: return qApp->translate ("FlarmRecordModel", "Flarm ID"); break;
		case 1: return qApp->translate ("FlarmRecordModel", "Registration"); break;
		case 2: return qApp->translate ("FlarmRecordModel", "Altitude"); break;
		case 3: return qApp->translate ("FlarmRecordModel", "Ground speed"); break;
		case 4: return qApp->translate ("FlarmRecordModel", "Vario"); break;
		case 5: return qApp->translate ("FlarmRecordModel", "State"); break;
	}

	return QVariant ();
}

QVariant FlarmRecordModel::data (const FlarmRecord &flarmRecord, int column, int role) const
{
	if (role==Qt::DisplayRole)
		return displayData (flarmRecord, column);
	else if (role==Qt::TextAlignmentRole)
		return alignmentData (flarmRecord, column);
	else if (role==FlarmRecordModel::sortRole)
		return sortData (flarmRecord, column);
	else
		return QVariant ();
}

QVariant FlarmRecordModel::displayData (const FlarmRecord &flarmRecord, int column) const
{
	switch (column)
	{
		case 0: return flarmRecord.getFlarmId ();
		case 1: return flarmRecord.getRegistration ();
		case 2: return QString ("%1 m"   ).arg (flarmRecord.getRelativeAltitude()                       );
		case 3: return QString ("%1 km/h").arg (flarmRecord.getGroundSpeed     ()/Velocity::km_h           );
		case 4: return QString ("%1 m/s" ).arg (flarmRecord.getClimbRate       ()/Velocity::m_s , 0, 'f', 1);
		case 5: return FlarmRecord::stateText (flarmRecord.getState ());
	}

	return QVariant ();
}

QVariant FlarmRecordModel::alignmentData (const FlarmRecord &flarmRecord, int column) const
{
	Q_UNUSED (flarmRecord);

	switch (column)
	{
		case 0: return Qt::AlignLeft;
		case 1: return Qt::AlignLeft;
		case 2: return Qt::AlignRight;
		case 3: return Qt::AlignRight;
		case 4: return Qt::AlignRight;
		case 5: return Qt::AlignLeft;
	}

	return QVariant ();
}

QVariant FlarmRecordModel::sortData (const FlarmRecord &flarmRecord, int column) const
{
	switch (column)
	{
		case 2: return flarmRecord.getRelativeAltitude ();
		case 3: return flarmRecord.getGroundSpeed      ();
		case 4: return flarmRecord.getClimbRate        ();
		default: return displayData (flarmRecord, column);
	}

	return QVariant ();
}
