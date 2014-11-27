#include "FlarmNetRecordModel.h"

#include <QApplication>

#include "src/flarm/flarmNet/FlarmNetRecord.h"

FlarmNetRecordModel::FlarmNetRecordModel ()
{
}

FlarmNetRecordModel::~FlarmNetRecordModel ()
{
}

int FlarmNetRecordModel::columnCount () const
{
	return 7;
}

QVariant FlarmNetRecordModel::displayHeaderData (int column) const
{
	switch (column)
	{
		case 0: return qApp->translate ("FlarmNetRecordModel", "Flarm ID");
		case 1: return qApp->translate ("FlarmNetRecordModel", "Registration");
		case 2: return qApp->translate ("FlarmNetRecordModel", "Callsign");
		case 3: return qApp->translate ("FlarmNetRecordModel", "Owner");
		case 4: return qApp->translate ("FlarmNetRecordModel", "Airfield");
		case 5: return qApp->translate ("FlarmNetRecordModel", "Model");
		case 6: return qApp->translate ("FlarmNetRecordModel", "Frequency");
	}

	return QVariant ();
}


QVariant FlarmNetRecordModel::displayData (const FlarmNetRecord &object, int column) const
{
	switch (column)
	{
		case 0: return object.flarmId;
		case 1: return object.registration;
		case 2: return object.callsign;
		case 3: return object.owner;
		case 4: return object.airfield;
		case 5: return object.type;
		case 6: return object.frequency;
	}

	return QVariant ();
}
