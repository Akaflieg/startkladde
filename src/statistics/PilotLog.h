#ifndef PILOTLOG_H_
#define PILOTLOG_H_

#include <QAbstractTableModel>
#include <QString>
#include <QList>
#include <QDate>
#include <QDateTime>

#include "src/db/dbId.h"

class Cache;
class Flight;

// TODO: display guest as "(Guest)"
// TODO: consider basing this (and other statistics classes) on ObjectListModel
class PilotLog: public QAbstractTableModel
{
		Q_OBJECT

	protected:
		PilotLog (QObject *parent=NULL);
		~PilotLog ();

		class Entry
		{
			public:
				Entry ();
				virtual ~Entry ();

				static Entry create (const Flight &flight, Cache &cache);

				QDate date;
				QString planeType;
				QString planeRegistration;
				QString pilot;
				QString copilot;
				QString launchMethod;
				QString departureLocation;
				QString landingLocation;
				QDateTime departureTime;
				QDateTime landingTime;
				QTime flightDuration;
				QString comments;

				bool valid;

				virtual QString dateText () const;
				virtual QString departureTimeText () const;
				virtual QString landingTimeText () const;
				virtual QString flightDurationText () const;
		};

	public:
		enum FlightInstructorMode { flightInstructorNone, flightInstructorStrict, flightInstructorLoose };

		static PilotLog *createNew (dbId personId, const QList<Flight> &flights, Cache &cache, FlightInstructorMode mode=flightInstructorNone);
		static PilotLog *createNew (const QList<Flight> &flights, Cache &cache, FlightInstructorMode mode=flightInstructorNone);

		// QAbstractTableModel methods
		virtual int rowCount (const QModelIndex &index) const;
		virtual int columnCount (const QModelIndex &index) const;
		virtual QVariant data (const QModelIndex &index, int role = Qt::DisplayRole) const;
		virtual QVariant headerData (int section, Qt::Orientation orientation, int role=Qt::DisplayRole) const;

	private:
		QList<Entry> entries;
};

#endif
