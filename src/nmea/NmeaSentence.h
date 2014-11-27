#ifndef NMEASENTENCE_H_
#define NMEASENTENCE_H_

#include <QString>
#include <QStringList>

/**
 * An NMEA sentence
 *
 * This class serves as a base class for concrete NMEA sentence implementations.
 *
 * To create an NMEA sentence implementation, inherit from this class. In the
 * constructor, pass the expected sentence type and the expected number of
 * parts. Then, parse the parts returned by parts (), calling setValid (false)
 * if a parse error occurs.
 */
class NmeaSentence
{
	public:
		NmeaSentence (const QString &line, const QString &expectedType, int expectedNumberOfParts);
		~NmeaSentence ();

		QString line () const;

		bool isValid () const;

		QStringList parts () const;
		QString type () const;

	protected:
		void setValid (bool valid);

	private:
		QString _line;
		QStringList _parts;
		bool _valid;
};

#endif

