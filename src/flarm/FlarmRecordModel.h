#ifndef FLARMRECORDMODEL_H_
#define FLARMRECORDMODEL_H_

#include <QVariant>

#include "src/itemDataRoles.h"
#include "src/model/objectList/ObjectModel.h"

class FlarmRecord;

class FlarmRecordModel: public ObjectModel<FlarmRecord>
{
	public:
		FlarmRecordModel ();
		virtual ~FlarmRecordModel ();

		virtual int columnCount () const;

		static const int sortRole=individualRole+0;

	protected:
		virtual QVariant displayHeaderData (int column) const;
		virtual QVariant data (const FlarmRecord &flarmRecord, int column, int role) const;
		virtual QVariant displayData (const FlarmRecord &flarmRecord, int column) const;
		virtual QVariant alignmentData (const FlarmRecord &flarmRecord, int column) const;
		virtual QVariant sortData (const FlarmRecord &flarmRecord, int column) const;
};

#endif
