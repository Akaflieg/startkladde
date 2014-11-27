#ifndef FLARM_H_
#define FLARM_H_

#include <QDateTime>
#include <QList>
#include <QObject>

#include "src/container/Maybe.h"
#include "src/numeric/GeoPosition.h"
#include "src/io/dataStream/ManagedDataStream.h" // For ManagedDataStream::State::Type

class DbManager;
class FlarmList;
class GpsTracker;
class NmeaDecoder;
class ManagedDataStream;

/**
 * A container for all non-GUI Flarm related functionality
 */
class Flarm: public QObject
{
		Q_OBJECT

	public:
		struct State { enum Type { disabled, /*closed,*/ active }; };

		// Connection type
		enum ConnectionType { noConnection, serialConnection, tcpConnection, fileConnection };
		static QString               ConnectionType_toString   (ConnectionType type);
		static ConnectionType        ConnectionType_fromString (const QString &string, ConnectionType defaultValue);
		static QString               ConnectionType_text       (ConnectionType type);
		static QList<ConnectionType> ConnectionType_list ();

		Flarm (QObject *parent, DbManager &dbManager);
		virtual ~Flarm ();

		// FIXME remove?
		NmeaDecoder *nmeaDecoder () { return _nmeaDecoder; }
		GpsTracker  *gpsTracker  () { return _gpsTracker;  }
		FlarmList   *flarmList   () { return _flarmList;   }

		// GPS tracker facade
		GeoPosition getPosition () const;
		QDateTime getGpsTime () const;

		bool isOpen () const;
		const ManagedDataStream *getManagedStream () const;

		State::Type state () const;

	public slots:
		void open  ();
		void close ();
		void setOpen (bool open);

	signals:
		void stateChanged (Flarm::State::Type state);
		void streamStateChanged (ManagedDataStream::State::Type state);

	protected:
		void updateDataStream ();
		void goToState (State::Type state);

	protected slots:
		void settingsChanged ();
		void managedDataStream_stateChanged (ManagedDataStream::State::Type state);

	private:
		template<class T> T *getOrCreateTypedDataStream ();

		DbManager &_dbManager;

		State::Type _state;

		ManagedDataStream *_managedDataStream;
		NmeaDecoder *_nmeaDecoder;
		GpsTracker  *_gpsTracker;
		FlarmList   *_flarmList;
};

#endif
