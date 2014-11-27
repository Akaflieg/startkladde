#ifndef FLARMLIST_H_
#define FLARMLIST_H_

#include "src/model/objectList/AbstractObjectList.h"
#include "src/flarm/FlarmRecord.h"
#include "src/db/DbManager.h"

class FlarmRecord;
class QPersistentModelIndex;
class NmeaDecoder;

class FlarmList: public AbstractObjectList<FlarmRecord>
{
		Q_OBJECT

	public:
		FlarmList (QObject *parent=NULL);
		virtual ~FlarmList ();

		// AbstractObjectList methods
		virtual int size () const;
		virtual const FlarmRecord &at (int index) const;
		virtual QList<FlarmRecord> getList () const;

		void setNmeaDecoder (NmeaDecoder *nmeaDecoder);
		void setDatabase (DbManager *dbManager);

	signals:
		void departureDetected  (const QString &flarmId);
		void landingDetected    (const QString &flarmId);
		void touchAndGoDetected (const QString &flarmId);

	public slots:
		void pflaaSentence (const PflaaSentence &sentence);

	private:
		QList<FlarmRecord *> flarmRecords;
		QMap<QString, QPersistentModelIndex> byFlarmId;

		NmeaDecoder *nmeaDecoder;
		DbManager *dbManager;

	private slots:
		// Controller slots (the signals are in FlarmRecord)
		void removeFlarmRecord (const QString &flarmId);
};

#endif
