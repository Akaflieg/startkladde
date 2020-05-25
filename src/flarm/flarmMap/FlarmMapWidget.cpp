#include "src/flarm/flarmMap/FlarmMapWidget.h"

#include <iostream>

#include <QBrush>
#include <QModelIndex>
#include <QPainter>
#include <QPen>

#include "src/flarm/FlarmList.h"
#include "src/flarm/FlarmRecord.h"
#include "src/flarm/flarmMap/KmlReader.h"
#include "src/graphics/grid.h"
#include "src/i18n/notr.h"
#include "src/nmea/GpsTracker.h"
#include "src/numeric/Velocity.h"
#include "src/util/double.h"
#include "src/util/qPainter.h"
#include "src/util/qPointF.h"
#include "src/util/qRect.h"
#include "src/util/qRectF.h"
#include "src/util/qSize.h"


// **************************
// ** StaticMarker methods **
// **************************

FlarmMapWidget::StaticMarker::StaticMarker (const GeoPosition &position, const QString &text, const QColor &backgroundColor):
	position (position), text (text), backgroundColor (backgroundColor)
{
}

FlarmMapWidget::StaticMarker::StaticMarker (const Kml::Marker &marker, const Kml::Style &style):
	position (marker.position), text (marker.name), backgroundColor (style.labelColor)
{
}


// ************************
// ** StaticPath methods **
// ************************

FlarmMapWidget::StaticPath::StaticPath (const QVector<GeoPosition> &points, const QString &name, const QPen &pen):
	points (points), name (name), pen (pen)
{
}

FlarmMapWidget::StaticPath::StaticPath (const Kml::Path &path, const Kml::Style &style):
	points (path.positions.toVector ()), name (path.name), pen (style.linePen ())
{
}


// ***************************
// ** StaticPolygon methods **
// ***************************

FlarmMapWidget::StaticPolygon::StaticPolygon (const QVector<GeoPosition> &points, const QString &name, const QPen &pen, const QBrush &brush):
	points (points), name (name), pen (pen), brush (brush)
{
}

FlarmMapWidget::StaticPolygon::StaticPolygon (const Kml::Polygon &polygon, const Kml::Style &style):
	points (polygon.positions.toVector ()), name (polygon.name),
	pen (style.linePen ()), brush (style.polyBrush ())
{
	// The KML polygon contains a copy of the first point at the end; remove
	// that.
	points.remove (points.size ()-1);
}

// *******************
// ** Image methods **
// *******************

FlarmMapWidget::Image::Image (const Kml::GroundOverlay &groundOverlay)
{
	// TODO error handling - .load returns false
	pixmap.load (groundOverlay.filename);

	northEast=GeoPosition (groundOverlay.north, groundOverlay.east);
	southWest=GeoPosition (groundOverlay.south, groundOverlay.west);
	northWest=GeoPosition (groundOverlay.north, groundOverlay.west);
	southEast=GeoPosition (groundOverlay.south, groundOverlay.east);
	rotation=groundOverlay.rotation;
	color=groundOverlay.color;
}


// ******************
// ** Construction **
// ******************

FlarmMapWidget::FlarmMapWidget (QWidget *parent): PlotWidget (parent),
	_ownPositionColor (255,   0, 0, 127),
	_climbColor       (  0, 255, 0, 127),
	_descentColor     (255, 255, 0, 127),
	flarmList (NULL), gpsTracker (NULL),
	kmlStatus (kmlNone),
	showImagesAction  (tr ("Show images" ), this),
	showGridAction    (tr ("Show grid"   ), this),
	showCirclesAction (tr ("Show circles"), this)
{
	setDiameter_p (4000);

	showImagesAction .setCheckable (true);
	showGridAction   .setCheckable (true);
	showCirclesAction.setCheckable (true);

	showImagesAction .setChecked (true);
	showGridAction   .setChecked (true);
	showCirclesAction.setChecked (true);

	addAction (&showImagesAction);
	addAction (&showGridAction);
	addAction (&showCirclesAction);

	setContextMenuPolicy (Qt::ActionsContextMenu);

	_ownPositionText=tr ("Me"); // FIXME proper English word for "Startstelle"
}

FlarmMapWidget::~FlarmMapWidget ()
{
}


// ***********
// ** State **
// ***********

void FlarmMapWidget::saveState (QSettings &settings)
{
	PlotWidget::saveState (settings);

	settings.setValue ("showImages" , showImagesAction .isChecked ());
	settings.setValue ("showGrid"   , showGridAction   .isChecked ());
	settings.setValue ("showCircles", showCirclesAction.isChecked ());
}

void FlarmMapWidget::loadState (QSettings &settings)
{
	PlotWidget::loadState (settings);

	showImagesAction .setChecked (settings.value ("showImages" , true).toBool ());
	showGridAction   .setChecked (settings.value ("showGrid"   , true).toBool ());
	showCirclesAction.setChecked (settings.value ("showCircles", true).toBool ());

	update ();
}


// ****************
// ** Flarm list **
// ****************

/**
 * Sets the Flarm list to use
 *
 * If a Flarm list was set before, it is replaced by the new Flarm list. If the
 * new Flarm list is the same as before, nothing is changed. Setting the Flarm
 * list to NULL (the default) effectively disables Flarm data display.
 *
 * This view does not take ownership of the Flarm list.
 */
void FlarmMapWidget::setFlarmList (FlarmList *flarmList)
{
	if (flarmList==this->flarmList)
		return;

	// Note that we can ignore layoutChanged because we don't consider the
	// order of the entries anyway, and we refer to entries by Flarm ID, not
	// index.

	if (this->flarmList)
	{
		disconnect (this->flarmList, SIGNAL (destroyed()), this, SLOT (flarmListDestroyed ()));
		disconnect (this->flarmList, SIGNAL (dataChanged (QModelIndex, QModelIndex)), this, SLOT (dataChanged (QModelIndex, QModelIndex)));
		disconnect (this->flarmList, SIGNAL (rowsInserted (QModelIndex, int, int)), this, SLOT (rowsInserted (QModelIndex, int, int)));
		disconnect (this->flarmList, SIGNAL (rowsAboutToBeRemoved (QModelIndex, int, int)),this, SLOT (rowsAboutToBeRemoved (QModelIndex, int, int)));
		disconnect (this->flarmList, SIGNAL (modelReset ()), this, SLOT (modelReset ()));
	}

	this->flarmList=flarmList;

	if (this->flarmList)
	{
		connect (this->flarmList, SIGNAL (destroyed()), this, SLOT (flarmListDestroyed ()));
		connect (this->flarmList, SIGNAL (dataChanged (QModelIndex, QModelIndex)), this, SLOT (dataChanged (QModelIndex, QModelIndex)));
		connect (this->flarmList, SIGNAL (rowsInserted (QModelIndex, int, int)), this, SLOT (rowsInserted (QModelIndex, int, int)));
		connect (this->flarmList, SIGNAL (rowsAboutToBeRemoved (QModelIndex, int, int)),this, SLOT (rowsAboutToBeRemoved (QModelIndex, int, int)));
		connect (this->flarmList, SIGNAL (modelReset ()), this, SLOT (modelReset ()));
	}

	update ();
}

void FlarmMapWidget::setGpsTracker (GpsTracker *gpsTracker)
{
	if (gpsTracker==this->gpsTracker)
		return;

	if (this->gpsTracker)
	{
		disconnect (this->gpsTracker, SIGNAL (positionChanged (const GeoPosition &)), this, SLOT (ownPositionChanged (const GeoPosition &)));
	}

	this->gpsTracker=gpsTracker;

	if (this->gpsTracker)
	{
		connect (this->gpsTracker, SIGNAL (positionChanged (const GeoPosition &)), this, SLOT (ownPositionChanged (const GeoPosition &)));
	}

	if (this->gpsTracker)
	{
		ownPositionChanged (gpsTracker->getPosition ());
	}
}

// **********
// ** View **
// **********

/**
 * Updates the static curves for the new position
 *
 * Call this method when the own position changes.
 */
void FlarmMapWidget::ownPositionChanged (const GeoPosition &ownPosition)
{
	// Only redraw if either the current own position is invalid (i. e., we
	// didn't draw yet), the new own position is invalid (GPS reception lost),
	// or the position changed by more than a given threshold.
	// The typical noise on the Flarm GPS data seems to be substantially less
	// than 1 m. Currently, the threshold is set to 0, so the widget is redrawn
	// every time a position is received (typically once per second).
	if (
		!_ownPosition.isValid () ||
		! ownPosition.isValid () ||
		ownPosition.distanceTo (_ownPosition)>0)
	{
		_ownPosition=ownPosition;
		emit ownPositionUpdated ();
	}
}

bool FlarmMapWidget::isOwnPositionKnown () const
{
	return _ownPosition.isValid ();
}


// *****************
// ** Static data **
// *****************

/**
 * Adds, updates or removes a static marker at the origin (i. e. the own
 * position)
 *
 * @param text the text to display at the own position. If empty, the own
 * position is not shown.
 * @param color the background color of the label
 */
void FlarmMapWidget::setOwnPositionLabel (const QString &text, const QColor &color)
{
	_ownPositionText=text;
	_ownPositionColor=color;
	update ();
}


// ****************
// ** Flarm data **
// ****************

/**
 * Adds a marker for a given Flarm record
 *
 * This must be called exactly once for each Flarm record after it is added. If
 * the plane data changes subsequently, updatePlaneMarker must be called. You
 * may only call updatePlaneMarker after addPlaneMarker has been called with
 * the respective Flarm record.
 *
 * This method also calls updatePlaneMarker.
 */
void FlarmMapWidget::addPlaneMarker (const FlarmRecord &record)
{
	planeMarkers.insert (record.getFlarmId (), PlaneMarker ());
	updatePlaneMarker (record);
}

/**
 * Updates the plane marker data for a given Flarm record
 *
 * This method must be called whenever a Flarm record is updated. It also
 * schedules a repaint of the widget.
 */
void FlarmMapWidget::updatePlaneMarker (const FlarmRecord &record)
{
	PlaneMarker &marker=planeMarkers[record.getFlarmId ()];

	// Always set the position, even if the marker is not visible
	marker.position_p=record.getRelativePosition ();
	marker.trail_p=record.getPreviousRelativePositions ().toVector ();

	switch (record.getState ())
	{
		case FlarmRecord::stateStarting:
		case FlarmRecord::stateFlying:
		case FlarmRecord::stateLanding:
			marker.style=PlaneMarker::verbose;

			marker.text=qnotr ("%1\n%2/%3/%4")
				.arg (record.getRegistration ())
				.arg (record.getRelativeAltitude ())
				.arg (record.getGroundSpeed () / Velocity::km_h)
				.arg (record.getClimbRate (), 0, 'f', 1);

			if (record.getClimbRate () > 0.0)
				marker.color=_climbColor;
			else
				marker.color=_descentColor;

			break;
		case FlarmRecord::stateOnGround:
			marker.style=PlaneMarker::minimal;
			break;
		case FlarmRecord::stateUnknown:
		case FlarmRecord::stateFlyingFar:
			marker.style=PlaneMarker::invisible;
			break;
		// no default
	}

	// Schedule a repaint
	update ();
}

/**
 * Removes the plane marker for a given Flarm record
 *
 * This must be called exactly once for each Flarm record before (!) it is
 * removed. updateFlarmData may not be called after this method has been called
 * for the given Flarm record. This method also schedules a repaint of the
 * widget.
 */
void FlarmMapWidget::removePlaneMarker (const FlarmRecord &record)
{
	planeMarkers.remove (record.getFlarmId ());
	update ();
}

/**
 * Refreshes the plane markers for all Flarm records in the Flarm list
 *
 * This is done by first removing all plane markers and then adding the plane
 * markers by calling addPlaneMarker for each Flarm record in the Flarm list.
 *
 * This method can be called even if there is no Flarm list. All plot data will
 * still be removed and nothing will be added.
 */
void FlarmMapWidget::refreshPlaneMarkers ()
{
	planeMarkers.clear ();

	// Only draw if we have a Flarm list
	if (flarmList)
	{
		for (int i=0, n=flarmList->size (); i<n; ++i)
		{
			const FlarmRecord &record=flarmList->at (i);
			addPlaneMarker (record);
		}
	}
}


// **********************
// ** Flarm list slots **
// **********************

/**
 * Called after one or more rows have been inserted into the Flarm list. Adds
 * the Flarm data for the new row(s).
 */
void FlarmMapWidget::rowsInserted (const QModelIndex &parent, int start, int end)
{
	Q_UNUSED (parent);

	if (flarmList)
		for (int i=start; i<=end; ++i)
			addPlaneMarker (flarmList->at (i));
}

/**
 * Called after one or more rows have changed in the Flarm list. Updates the
 * Flarm data for the changed row(s).
 */
void FlarmMapWidget::dataChanged (const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
	if (flarmList)
		for (int i=topLeft.row (); i<=bottomRight.row (); ++i)
			updatePlaneMarker (flarmList->at (i));
}

/**
 * Called before (!) one or more rows are removed from the Flarm list. Removes
 * the Flarm data for the row(s) to be removed.
 */
void FlarmMapWidget::rowsAboutToBeRemoved (const QModelIndex &parent, int start, int end)
{
	Q_UNUSED (parent);

	if (flarmList)
		for (int i=start; i<=end; ++i)
			removePlaneMarker (flarmList->at (i));
}

/**
 * Called after the Flarm list has changed completely. Refreshes all Flarm data.
 */
void FlarmMapWidget::modelReset ()
{
	refreshPlaneMarkers ();
}

/**
 * Called before the Flarm list is destroyed
 *
 * This method sets the Flarm list to NULL, meaning "no Flarm list", to prevent
 * further accesses to the model.
 */
void FlarmMapWidget::flarmListDestroyed ()
{
	this->flarmList=NULL;
	modelReset ();
}


// *********
// ** KML **
// *********

/**
 * Note that if multiple KML files have been loaded (by multiple calls to
 * readKml), this only reflects the status of the last file.
 */
FlarmMapWidget::KmlStatus FlarmMapWidget::getKmlStatus () const
{
	return kmlStatus;
}

/**
 * Never call this method except from readKml
 *
 * This method only exists to facilitate setting kmlStatus in readKml and to
 * allow the compiler to issue a warning in case no status is assigned.
 */
FlarmMapWidget::KmlStatus FlarmMapWidget::readKmlImplementation (const QString &filename)
{
	QString effectiveFilename=filename.trimmed ();
	if (effectiveFilename.isEmpty ())
		return kmlNone;

	KmlReader kmlReader;
	KmlReader::ReadResult readResult=kmlReader.read (effectiveFilename);

	switch (readResult)
	{
		case KmlReader::readNotFound:   return kmlNotFound  ;
		case KmlReader::readOpenError:  return kmlReadError ;
		case KmlReader::readParseError: return kmlParseError;
		case KmlReader::readOk:         break; // Go on, process it
		// no default
	}

	if (kmlReader.isEmpty ())
		return kmlEmpty;

	// For each KML marker in the KML file, add a static marker
	foreach (const Kml::Marker &kmlMarker, kmlReader.markers)
	{
		Kml::Style style=kmlReader.findStyle (kmlMarker.styleUrl);
		StaticMarker staticMarker (kmlMarker, style);
		staticMarkers.append (staticMarker);
		allStaticPositions.append (kmlMarker.position);
	}

	// For each KML path in the KML file, add a static path
	foreach (const Kml::Path &path, kmlReader.paths)
	{
		Kml::Style style=kmlReader.findStyle (path.styleUrl);
		StaticPath staticCurve=StaticPath (path, style);
		staticPaths.append (staticCurve);
		allStaticPositions.append (staticCurve.points.toList ());
	}

	// For each KML polygon in the KML file, add a static polygon
	foreach (const Kml::Polygon &polygon, kmlReader.polygons)
	{
		Kml::Style style=kmlReader.findStyle (polygon.styleUrl);
		StaticPolygon staticCurve=StaticPolygon (polygon, style);
		staticPolygons.append (staticCurve);
		allStaticPositions.append (staticCurve.points.toList ());
	}

	// For each ground overlay in the KML file, add an image
	foreach (const Kml::GroundOverlay &overlay, kmlReader.groundOverlays)
	{
		Image image=Image (overlay);
		images.append (image);
		allStaticPositions.append (image.northWest);
		allStaticPositions.append (image.northEast);
		allStaticPositions.append (image.southWest);
		allStaticPositions.append (image.southEast);
	}

	// Something changed, so we have to schedule a repaint
	update ();

	return kmlOk;
}

FlarmMapWidget::KmlStatus FlarmMapWidget::readKml (const QString &filename)
{
	kmlStatus=readKmlImplementation (filename);
	return kmlStatus;
}


// **********
// ** View **
// **********

bool FlarmMapWidget::isOwnPositionVisible () const
{
	return rect ().contains (toWidget (0, 0).toPoint ());
}

bool FlarmMapWidget::isAnyStaticElementVisible () const
{
	if (!_ownPosition.isValid ())
		return false;

	QRectF visibleRect_widget=rect ();

	foreach (const GeoPosition &position, allStaticPositions)
	{
		QPointF position_widget=transformGeographicToWidget (position);
		if (visibleRect_widget.contains (position_widget))
			return true;
	}

	return false;
}

bool FlarmMapWidget::findClosestStaticElement (double *distance, Angle *bearing) const
{
	if (!_ownPosition.isValid ())
		return false;

	QPointF closestRelativePoint;
	double closestDistanceSquared=-1;

	foreach (const GeoPosition &geoPosition, allStaticPositions)
	{
		// In the local coordinate system
		QPointF position_p=geoPosition.relativePositionTo (_ownPosition);

		// Relative to the display center
		QPointF relativePoint=position_p-center_p ();

		double distanceSquared=lengthSquared (relativePoint);

		if (closestDistanceSquared<0 || distanceSquared<closestDistanceSquared)
		{
			closestRelativePoint=relativePoint;
			closestDistanceSquared=distanceSquared;
		}
	}

	if (closestDistanceSquared>=0)
	{
		if (distance) (*distance)=sqrt (closestDistanceSquared);
		// Note the transposition - atan2 calculates mathematical angle
		// (starting at x, going counter-clockwise), we need geographical angle
		// (starting at y, going clockwise).
		if (bearing)  (*bearing)=Angle::atan2 (transposed (closestRelativePoint));

		return true;
	}
	else
	{
		return false;
	}
}

void FlarmMapWidget::resetPosition ()
{
	scrollToCenter (QPointF (0, 0));
}




// **********
// ** View **
// **********

double FlarmMapWidget::transformGeographicToWidget (const Angle &greatCircleAngle) const
{
	return toWidget (GeoPosition::greatCircleDistance (greatCircleAngle));
}

Angle FlarmMapWidget::transformWidgetToGeographic (double distance_w) const
{
	return GeoPosition::greatCircleAngle (toPlot (distance_w));
}

/**
 * Undefined if the own position is invalid
 */
QPointF FlarmMapWidget::transformGeographicToWidget (const GeoPosition &geoPosition) const
{
	return toWidget (geoPosition.relativePositionTo (_ownPosition));
}

// Vectorized transform
QPolygonF FlarmMapWidget::transformGeographicToWidget (const QVector<GeoPosition> &geoPositions) const
{
	QPolygonF result;

	foreach (const GeoPosition &geoPosition, geoPositions)
		result.append (transformGeographicToWidget (geoPosition));

	return result;
}




// **************
// ** Painting **
// **************

// Notes for the paint* methods:
//   * The painter passed as an argument is set to widget coordinates. It may be
//     freely modified
//   * All prerequisites (like the validity of the own position) have to be
//     checked by the methods. If the prerequisites are not met, the method
//     should return right away.

void FlarmMapWidget::paintImages (QPainter &painter)
{
	if (!_ownPosition.isValid ()) return;
	if (!showImagesAction.isChecked ()) return;

	// The y axis of the pixmap points down, so we use a coordinate system
	// that is identical to the plot coordinate system, only with the y
	// axis pointing to the south instead of the north. This coordinate
	// system is called the "draw" coordinate system. We will later transform
	// the painter to this coordinate system.

	QTransform plotSystem_draw;
	plotSystem_draw.scale (1, -1);

	transformToPlot (painter);
	painter.setTransform (plotSystem_draw.inverted (), true);

	foreach (const Image &image, images)
	{
		// Calculate the northwest and southeast corners of the image in the
		// local coordinate system
		QPointF northWest_p=image.northWest.relativePositionTo (_ownPosition);
		QPointF southEast_p=image.southEast.relativePositionTo (_ownPosition);

		// Calculate the northwest and southeast corners of the image in the
		// draw coordinate system (the painter will be transformed to this
		// coordinate system). These two corners define the rectangle the pixmap
		// will be drawn into.
		QPointF northWest_draw=northWest_p*plotSystem_draw;
		QPointF southEast_draw=southEast_p*plotSystem_draw;
		QRectF imageRect_draw (northWest_draw, southEast_draw);

		// Rotate the painter coordinate system around the image center
		painter.save ();
		painter.translate (imageRect_draw.center ());
		painter.rotate (-image.rotation.toDegrees ());
		painter.translate (-imageRect_draw.center ());

		// Draw the whole pixmap in the draw coordinate system, into the
		// rectangle determined earlier
		painter.setOpacity (image.color.alphaF ());
		painter.drawPixmap (imageRect_draw, image.pixmap, image.pixmap.rect ());
		painter.restore ();
	}
}

void FlarmMapWidget::paintDistanceCircles (QPainter &painter)
{
	if (!showCirclesAction.isChecked ()) return;

	// Set a cosmetic pen (i. e. one that uses pixel dimensions, regardless of
	// the transformation) with a width of 0.5 pixels.
	QPen pen=painter.pen ();
	pen.setCosmetic (true);
	pen.setWidthF (0.5);
	painter.setPen (pen);

	// The minimum distance between circles is given by the height of a sample
	// text
	int minimumIncrement_w=4*textSize (painter, "1 km").height ();
	double minimumIncrement_p=toPlot (minimumIncrement_w);
	double radiusIncrement_p=getDecimalGridSize (minimumIncrement_p);
	// Smallest radius increment we'll use: 500 m
	if (radiusIncrement_p<=1000) radiusIncrement_p=1000;

	QRectF rect_w=rect ();
	QPointF center_w=toWidget (0, 0);

	double minimumDistance_w=minimumDistance (rect_w, center_w);
	double maximumDistance_w=maximumDistance (rect_w, center_w);

	double minimumDistance_p=toPlot (minimumDistance_w);
	double maximumDistance_p=toPlot (maximumDistance_w);

	double startRadius_p=floor (minimumDistance_p/radiusIncrement_p)*radiusIncrement_p;
	double endRadius_p=maximumDistance_p;

	// Don't start at 0
	if (startRadius_p==0)
		startRadius_p=radiusIncrement_p;

	for (double radius_p=startRadius_p; radius_p<=endRadius_p; radius_p+=radiusIncrement_p)
	{
		double radius_w=toWidget (radius_p);

		drawCircle (painter, center_w, radius_w);

		QString text=tr ("%1 km").arg (radius_p/1000);
		drawText (painter, center_w+QPointF (0, -radius_w), Qt::AlignBottom, text, 0);
	}
}

void FlarmMapWidget::paintLatLonGrid (QPainter &painter)
{
	if (!_ownPosition.isValid ()) return;
	if (!showGridAction.isChecked ()) return;

	// Set a cosmetic pen (i. e. one that uses pixel dimensions, regardless of
	// the transformation) with a width of 0.5 pixels.
	QPen pen=painter.pen ();
	pen.setCosmetic (true);
	pen.setWidthF (0.5);
	painter.setPen (pen);
	// Set a white, semi-transparent background (for the text)
	painter.setBrush (QBrush (QColor (255, 255, 255, 127)));

	// The lines will be drawn into the bounding rectangle to the widget
	// rectangle in plot coordinates. They will therefore be longer than what
	// fits in the widget if the orientation is not 0°, 90°, 180° or 270°.
	QRectF bounding_p=boundingRect_p ();

	GeoPosition southWest=_ownPosition.offsetPosition (bounding_p.topLeft     ());
	GeoPosition northEast=_ownPosition.offsetPosition (bounding_p.bottomRight ());

	Angle westLongitude=southWest.getLongitude ();
	Angle eastLongitude=northEast.getLongitude ();
	Angle southLatitude=southWest.getLatitude ();
	Angle northLatitude=northEast.getLatitude ();

	double west_min=westLongitude.toMinutes ();
	double east_min=eastLongitude.toMinutes ();
	double south_min=southLatitude.toMinutes ();
	double north_min=northLatitude.toMinutes ();

	// The minimum distance between grid lines is given by the width of a sample
	// text
	int minimumDistance_w=textSize (painter, "999° 99° E").width ();
	Angle minimumDistance_geo=transformWidgetToGeographic (minimumDistance_w);

	// Determine the actual increment
	Angle increment=getAngleGridSize (minimumDistance_geo);
	double increment_min=increment.toMinutes ();

	// Depending on the orientation, draw the latitude/longitude values at
	// different positions. The orientation type may be 0 (north up), 1 (north
	// right), 2 (north down) or 3 (north left)
	int o=orientation ().toDegrees ();
	int orientationType;
	if      (o>=  0+45 && o< 90+45) orientationType=3;
	else if (o>= 90+45 && o<180+45) orientationType=2;
	else if (o>=180+45 && o<270+45) orientationType=1;
	else                            orientationType=0;

	// Calculate the outlines of the rectangle. Shrink the rectangle a bit to
	// get a margin between the widget borders and the text. Also, for
	// orthogonal orientations, the latitude/longitude lines' points may be on
	// the outlines of the rectangle, so due to rounding errors, they may not
	// intersect.
	QRectF rect_w=this->rect_w ();
	rect_w=enlarged (rect_w, -2);

	// Longitude (vertical) lines (_min signifies minutes)
	for (double longitude_min=floor (west_min, increment_min); longitude_min<=east_min; longitude_min+=increment_min)
	{
		// Calculate the line extents
		Angle longitude=Angle::fromMinutes (longitude_min);
		QPointF p1=transformGeographicToWidget (GeoPosition (southLatitude, longitude));
		QPointF p2=transformGeographicToWidget (GeoPosition (northLatitude, longitude));
		QLineF line (p1, p2);

		// Draw the line
		painter.drawLine (line);

		// Draw text at intersection with widget outline
		QLineF intersectionLine;
		Qt::Alignment textAlignment;
		if      (orientationType==0) { intersectionLine=topLine    (rect_w); textAlignment=Qt::AlignTop   ; }
		else if (orientationType==1) { intersectionLine=rightLine  (rect_w); textAlignment=Qt::AlignRight ; }
		else if (orientationType==2) { intersectionLine=bottomLine (rect_w); textAlignment=Qt::AlignBottom; }
		else if (orientationType==3) { intersectionLine=leftLine   (rect_w); textAlignment=Qt::AlignLeft  ; }

		QString text=longitude.formatDmSuffix ("E", "W");
		QPointF intersection;
#ifdef QT_COMPAT
	if (line.intersect (intersectionLine, &intersection)==QLineF::BoundedIntersection)
#else
        if (line.intersects (intersectionLine, &intersection)==QLineF::BoundedIntersection)
#endif
			drawText (painter, intersection, textAlignment, text);
	}

	// Latitude (horizontal) lines (_min signifies minutes)
	for (double latitude_min=floor (south_min, increment_min); latitude_min<=north_min; latitude_min+=increment_min)
	{
		// Calculate the line extents
		Angle latitude=Angle::fromMinutes (latitude_min);
		QPointF p1=transformGeographicToWidget (GeoPosition (latitude, westLongitude));
		QPointF p2=transformGeographicToWidget (GeoPosition (latitude, eastLongitude));
		QLineF line (p1, p2);

		// Draw the line
		painter.drawLine (line);

		// Draw text at intersection with widget outline
		QLineF intersectionLine;
		Qt::Alignment textAlignment;
		if      (orientationType==0) { intersectionLine=leftLine   (rect_w); textAlignment=Qt::AlignLeft  ; }
		else if (orientationType==1) { intersectionLine=topLine    (rect_w); textAlignment=Qt::AlignTop   ; }
		else if (orientationType==2) { intersectionLine=rightLine  (rect_w); textAlignment=Qt::AlignRight ; }
		else if (orientationType==3) { intersectionLine=bottomLine (rect_w); textAlignment=Qt::AlignBottom; }

		QString text=latitude.formatDmSuffix ("N", "S");
		QPointF intersection;
        if (line.intersects (intersectionLine, &intersection)==QLineF::BoundedIntersection)
            drawText (painter, intersection, textAlignment, text);
	}
}

void FlarmMapWidget::paintNorthDirection (QPainter &painter)
{
	// Draw an arrow from the own position to the north direction

	// Set a cosmetic pen (i. e. one that uses pixel dimensions, regardless of
	// the transformation) with a width of 0.5 pixels.
	QPen pen=painter.pen ();
	pen.setCosmetic (true);
	pen.setWidthF (0.5);
	painter.setPen (pen);

	// Translate to the origin of the plot coordinate system
	painter.translate (toWidget (0, 0));
	// Flip the y axis
	painter.scale (1, -1);
	// Rotate
	painter.rotate (orientation ().toDegrees ());

	// TODO Arrow looks bad
	int smallerSide=qMin (width (), height ());
	double len=smallerSide/4.0;
	double arrowLength=10;
	double arrowWidth=5;
	painter.drawLine (QPointF (0, 0)                        , QPointF (0, len));
	painter.drawLine (QPointF (-arrowWidth, len-arrowLength), QPointF (0, len));
	painter.drawLine (QPointF ( arrowWidth, len-arrowLength), QPointF (0, len));

	QString text=tr (" N ");
	QSize size=textSize (painter, text);
	painter.translate (0, len+diameter (size)/2);
	painter.rotate (-orientation ().toDegrees ());
	painter.scale (1, -1);
	drawCenteredText (painter, QPointF (0, 0), text);
}

void FlarmMapWidget::paintOwnPosition (QPainter &painter)
{
	// Draw the own position
	painter.setBrush (_ownPositionColor);
	QPointF position_w=toWidget (QPointF (0, 0));
	drawCenteredText (painter, position_w, _ownPositionText);
}

void FlarmMapWidget::paintStaticPaths (QPainter &painter)
{
	if (!_ownPosition.isValid ())
		return;

	// Draw all static paths
	foreach (const StaticPath &path, staticPaths)
	{
		QPolygonF p=transformGeographicToWidget (path.points);
		painter.setPen (path.pen);
		painter.drawPolyline (p);
	}
}

void FlarmMapWidget::paintStaticPolygons (QPainter &painter)
{
	if (!_ownPosition.isValid ())
		return;

	// Draw all static paths
	foreach (const StaticPolygon &polygon, staticPolygons)
	{
		QPolygonF p=transformGeographicToWidget (polygon.points);
		painter.setPen (polygon.pen);
		painter.setBrush (polygon.brush);
		painter.drawPolygon (p);
	}
}

void FlarmMapWidget::paintStaticMarkers (QPainter &painter)
{
	if (!_ownPosition.isValid ())
		return;

	foreach (const StaticMarker &marker, staticMarkers)
	{
		QPointF p=transformGeographicToWidget (marker.position);
		painter.setBrush (marker.backgroundColor);
		drawCenteredText (painter, p, marker.text);
	}
}

void FlarmMapWidget::paintPlanes (QPainter &painter)
{
	foreach (const PlaneMarker &marker, planeMarkers.values ())
	{
		QPointF position_w=toWidget (marker.position_p);

		switch (marker.style)
		{
			case PlaneMarker::invisible:
				// Don't paint at all
				break;
			case PlaneMarker::minimal:
			{
				// Cross, 8 pixels in size
				drawOrthogonalCross (painter, position_w, 4);
			} break;
			case PlaneMarker::verbose:
				// State-dependent text with state-dependent background color
				painter.setBrush (QBrush (marker.color));
				QPen pen;
				pen.setWidth (2);
				painter.setPen (pen);
				drawCenteredText (painter, position_w, marker.text);
				painter.drawPolyline (toWidget (marker.trail_p));
				break;
			// No default
		}
	}
}

/**
 * Paints the widget
 */
void FlarmMapWidget::paintEvent (QPaintEvent *event)
{
	// Paint the frame
	QFrame::paintEvent (event);

	// Create the painter and turn anti-aliasing on
	QPainter painter (this);
	painter.setRenderHint (QPainter::Antialiasing         , true);
	painter.setRenderHint (QPainter::TextAntialiasing     , true);
	painter.setRenderHint (QPainter::SmoothPixmapTransform, true);

	// Paint the various items, in order from bottom to top. Note that some of
	// the items can only be painted if the own position is valid. For example,
	// static data is specified in absolute (earth) coordinates and the display
	// coordinate system is centered at the own position.
	painter.save (); paintImages           (painter); painter.restore ();
	painter.save (); paintDistanceCircles  (painter); painter.restore ();
	painter.save (); paintLatLonGrid       (painter); painter.restore ();
	painter.save (); paintNorthDirection   (painter); painter.restore ();
	painter.save (); paintOwnPosition      (painter); painter.restore ();
	painter.save (); paintStaticPaths      (painter); painter.restore ();
	painter.save (); paintStaticPolygons   (painter); painter.restore ();
	painter.save (); paintStaticMarkers    (painter); painter.restore ();
	painter.save (); paintPlanes           (painter); painter.restore ();
}
