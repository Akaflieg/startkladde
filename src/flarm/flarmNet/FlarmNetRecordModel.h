#ifndef FLARMNETRECORDMODEL_H_
#define FLARMNETRECORDMODEL_H_

#include <QVariant>

#include "src/model/objectList/ObjectModel.h"

class FlarmNetRecord;

class FlarmNetRecordModel: public ObjectModel<FlarmNetRecord>
{
	public:
		FlarmNetRecordModel ();
		virtual ~FlarmNetRecordModel ();

		virtual int columnCount () const;

	protected:
		virtual QVariant displayHeaderData (int column) const;
		virtual QVariant displayData (const FlarmNetRecord &flarmNetRecord, int column) const;
};

#endif
