#ifndef KMLREADER_H_
#define KMLREADER_H_

#include <QString>
#include <QColor>
#include <QMap>
#include <QPen>
#include <QDir>

#include "src/flarm/flarmMap/Kml.h"
#include "src/numeric/GeoPosition.h"

class QDomElement;

/**
 * A simple and special purpose KML file reader
 *
 * To use, create an instance and call read() with the name of a file to read.
 * You can then get the read data from the properties markers, paths and
 * polygons. You can also find a style with a given URL using findStyle, or you
 * can access styles and styleMaps directly.
 *
 * This uses the Qt Xml module.
 */
class KmlReader
{
	public:
		enum ReadResult { readNotFound, readOpenError, readParseError, readOk };

		KmlReader ();
		virtual ~KmlReader ();

		ReadResult read (const QString &filename);

		bool isEmpty () const;
		Kml::Style findStyle (const QString &styleUrl);

		QList<Kml::Marker> markers;
		QList<Kml::Path> paths;
		QList<Kml::Polygon> polygons;
		QList<Kml::GroundOverlay> groundOverlays;

		QMap<QString, Kml::Style> styles;
		QMap<QString, Kml::StyleMap> styleMaps;

		GeoPosition boundingRectSouthWest;
		GeoPosition boundingRectNorthEast;

	private:
		void addToBoundingRect (const GeoPosition &position);

		void readStyle    (const QDomElement &styleElement);
		void readStyleMap (const QDomElement &styleMapelement);

		void readPlacemark (const QDomElement &placemarkElement);
		void readMarker  (const QString &name, const QString &styleUrl, const QDomElement &lookAtElement);
		void readPath    (const QString &name, const QString &styleUrl, const QDomElement &lineStringElement);
		void readPolygon (const QString &name, const QString &styleUrl, const QDomElement &polygonElement);
		void readGroundOverlay (const QDomElement &groundOverlayElement, const QDir &dir);
};

#endif
