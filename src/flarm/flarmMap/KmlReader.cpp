#include "src/flarm/flarmMap/KmlReader.h"

#include <iostream>

#include <QDomDocument>
#include <QDomNode>
#include <QDomElement>
#include <QFile>
#include <QIODevice>
#include <QStringList>
#include <QFileInfo>
#include <QDir>

#include "src/util/qString.h"

// Note that methods like QDomNode::firstChildElement return a null QDomElement
// if there is no such element, and we can safely access the null element's
// attributes.

KmlReader::KmlReader ()
{
}

KmlReader::~KmlReader ()
{
}

void KmlReader::readStyle (const QDomElement &styleElement)
{
	QString styleId=styleElement.attribute ("id");

	Kml::Style style;
	style.labelColor=Kml::parseColor (styleElement.firstChildElement ("LabelStyle").firstChildElement ("color"  ).text ());
	style.lineColor =Kml::parseColor (styleElement.firstChildElement ("LineStyle" ).firstChildElement ("color"  ).text ());
	style.lineWidth =                 styleElement.firstChildElement ("LineStyle" ).firstChildElement ("width"  ).text ().toDouble ();
	style.polyColor =Kml::parseColor (styleElement.firstChildElement ("PolyStyle" ).firstChildElement ("color"  ).text ());
	style.noOutline =                (styleElement.firstChildElement ("PolyStyle" ).firstChildElement ("outline").text ()=="0");
	style.noFill    =                (styleElement.firstChildElement ("PolyStyle" ).firstChildElement ("fill"   ).text ()=="0");

	styles.insert (styleId, style);
}

void KmlReader::readStyleMap (const QDomElement &styleMapElement)
{
	QString styleMapId=styleMapElement.attribute ("id");

	Kml::StyleMap styleMap;
	QDomNodeList pairElements=styleMapElement.elementsByTagName ("Pair");
	for (int i=0, n=pairElements.size (); i<n; ++i)
	{
		QDomElement pairElement=pairElements.at (i).toElement ();
		QString key     =pairElement.firstChildElement ("key"     ).text ();
		QString styleUrl=pairElement.firstChildElement ("styleUrl").text ();
		styleMap.styles.insert (key, styleUrl);
	}

	styleMaps.insert (styleMapId, styleMap);
}

void KmlReader::readMarker (const QString &name, const QString &styleUrl, const QDomElement &lookAtElement)
{
	 double longitude=lookAtElement.firstChildElement ("longitude").text ().toDouble ();
	 double latitude =lookAtElement.firstChildElement ("latitude" ).text ().toDouble ();
	 GeoPosition position=GeoPosition::fromDegrees (latitude, longitude);

	 Kml::Marker marker;
	 marker.position=position;
	 marker.name=name;
	 marker.styleUrl=styleUrl;

	 addToBoundingRect (position);

	 markers.append (marker);
}

void KmlReader::readPath (const QString &name, const QString &styleUrl, const QDomElement &lineStringElement)
{
	 QList<GeoPosition> points;
	 QDomElement coordinatesElement=lineStringElement.firstChildElement ("coordinates");
	 foreach (const QString &pointCoordinates, coordinatesElement.text ().trimmed ().split (" "))
	 {
		 QStringList parts=pointCoordinates.split (",");
		 if (parts.size ()>=2)
		 {
			 double longitude=parts[0].toDouble ();
			 double latitude =parts[1].toDouble ();
			 GeoPosition position=GeoPosition::fromDegrees (latitude, longitude);

			 points.append (position);

			 addToBoundingRect (position);
		 }
	 }

	 Kml::Path path;
	 path.name=name;
	 path.styleUrl=styleUrl;
	 path.positions=points;

	 paths.append (path);
}

/**
 * Note that the polygon will be closed, i. e. there will be one point more than
 * the number of corners in the polygon and the last point will be identical to
 * the first point.
 */
void KmlReader::readPolygon (const QString &name, const QString &styleUrl, const QDomElement &polygonElement)
{
	QList<GeoPosition> points;
	QDomElement coordinatesElemnt=polygonElement
			.firstChildElement ("outerBoundaryIs")
			.firstChildElement ("LinearRing")
			.firstChildElement ("coordinates");
	foreach (const QString &pointCoordinates, coordinatesElemnt.text ().trimmed ().split (" "))
	{
		QStringList parts=pointCoordinates.split (",");
		if (parts.size ()>=2)
		{
			double longitude=parts[0].toDouble ();
			double latitude =parts[1].toDouble ();
			GeoPosition position=GeoPosition::fromDegrees (latitude, longitude);

			points.append (position);

			addToBoundingRect (position);
		}
	}

	Kml::Polygon polygon;
	polygon.name=name;
	polygon.styleUrl=styleUrl;
	polygon.positions=points;

	polygons.append (polygon);
}

/**
 * A placemark is either a marker (LookAt element), a path (LineString element)
 * or a polygon (Polygon element).
 */
void KmlReader::readPlacemark (const QDomElement &placemarkElement)
{
	 QString placemarkName=placemarkElement.firstChildElement ("name").text ();
	 QString styleUrl=placemarkElement.firstChildElement ("styleUrl").text ();

	 QDomElement lookAtElement=placemarkElement.firstChildElement ("LookAt");
	 if (!lookAtElement.isNull ())
		 readMarker (placemarkName, styleUrl, lookAtElement);

	 QDomElement lineStringElement=placemarkElement.firstChildElement ("LineString");
	 if (!lineStringElement.isNull ())
		 readPath (placemarkName, styleUrl, lineStringElement);

	 QDomElement polygonElement=placemarkElement.firstChildElement ("Polygon");
	 if (!polygonElement.isNull ())
		 readPolygon (placemarkName, styleUrl, polygonElement);
}

// TODO should also support LatLonQuad (non-rectangular quadrilaterals), but
// that is probably not easy to draw
void KmlReader::readGroundOverlay (const QDomElement &groundOverlayElement, const QDir &dir)
{
	QString groundOverlayName=groundOverlayElement.firstChildElement ("name").text ();

	QDomElement colorElement=groundOverlayElement.firstChildElement ("color");
	QColor color=Kml::parseColor (colorElement.text ());

	QDomElement iconElement=groundOverlayElement.firstChildElement ("Icon");
	QString filename=iconElement.firstChildElement ("href").text ();

	QDomElement latLonBoxElement=groundOverlayElement.firstChildElement ("LatLonBox");
	double north=latLonBoxElement.firstChildElement ("north").text ().toDouble ();
	double south=latLonBoxElement.firstChildElement ("south").text ().toDouble ();
	double east =latLonBoxElement.firstChildElement ("east" ).text ().toDouble ();
	double west =latLonBoxElement.firstChildElement ("west" ).text ().toDouble ();
	// If there is no rotation element, the string will be empty and toDouble
	// will return 0.
	double rotation=latLonBoxElement.firstChildElement ("rotation").text ().toDouble ();

	Kml::GroundOverlay groundOverlay;
	groundOverlay.name=groundOverlayName;
	groundOverlay.filename=dir.filePath (filename);
	groundOverlay.north=Angle::fromDegrees (north);
	groundOverlay.south=Angle::fromDegrees (south);
	groundOverlay.east =Angle::fromDegrees (east );
	groundOverlay.west =Angle::fromDegrees (west );
	groundOverlay.rotation=Angle::fromDegrees (rotation);
	groundOverlay.color=color;

	groundOverlays.append (groundOverlay);
}

/**
 * This method is not reentrant because it calls a non-reentrant method of
 * QDomDocument.
 *
 * @param filename
 */
KmlReader::ReadResult KmlReader::read (const QString &filename)
{
	// Check if the file exists at all
	if (!QFile::exists (filename))
		return readNotFound;

	// Try to open the file
	QFile file (filename);
	if (!file.open (QIODevice::ReadOnly))
		return readOpenError;

	// Try to parse the file
	QDomDocument document ("kmlDocument");
	 if (!document.setContent (&file))
	     return readParseError;

	 // The file has been read and can be closed
	 file.close();

	 // Extract the placemarks (includes markers, paths and polygons) from the
	 // DOM structure
	 QDomNodeList placemarkNodes=document.elementsByTagName ("Placemark");
	 for (int i=0, n=placemarkNodes.size (); i<n; ++i)
		 readPlacemark (placemarkNodes.at (i).toElement ());

	 // Extract the ground overlays
	 QDomNodeList groundOverlayNodes=document.elementsByTagName ("GroundOverlay");
	 for (int i=0, n=groundOverlayNodes.size (); i<n; ++i)
		 readGroundOverlay (groundOverlayNodes.at (i).toElement (), QFileInfo (filename).dir ());

	 // Extract the styles from the DOM structure
	 QDomNodeList styleNodes=document.elementsByTagName ("Style");
	 for (int i=0, n=styleNodes.size (); i<n; ++i)
		 readStyle (styleNodes.at (i).toElement ());

	 // Extract the style maps from the DOM structure
	 QDomNodeList styleMapNodes=document.elementsByTagName ("StyleMap");
	 for (int i=0, n=styleMapNodes.size (); i<n; ++i)
		 readStyleMap (styleMapNodes.at (i).toElement ());

	 return readOk;
}

bool KmlReader::isEmpty () const
{
	if (!markers .isEmpty ()) return false;
	if (!paths   .isEmpty ()) return false;
	if (!polygons.isEmpty ()) return false;
	return true;
}

Kml::Style KmlReader::findStyle (const QString &styleUrl)
{
	if (!styleUrl.startsWith ("#"))
		return Kml::Style ();

	QString styleId=styleUrl.mid (1);

	if (styles.contains (styleId))
		return styles.value (styleId);

	if (styleMaps.contains (styleId))
	{
		Kml::StyleMap styleMap=styleMaps.value (styleId);
		if (styleMap.styles.contains ("normal"))
			return findStyle (styleMap.styles.value ("normal"));
	}

	return Kml::Style ();
}

void KmlReader::addToBoundingRect (const GeoPosition &position)
{
	if (boundingRectSouthWest.isValid ())
		boundingRectSouthWest=GeoPosition::southWest (boundingRectSouthWest, position);
	else
		boundingRectSouthWest=position;

	if (boundingRectNorthEast.isValid ())
		boundingRectNorthEast=GeoPosition::northEast (boundingRectNorthEast, position);
	else
		boundingRectNorthEast=position;
}
