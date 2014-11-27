#ifndef FLARMMAPWIDGET_H_
#define FLARMMAPWIDGET_H_

#include <QAction>
#include <QString>

#include "src/flarm/flarmMap/Kml.h"
#include "src/gui/widgets/PlotWidget.h"
#include "src/numeric/GeoPosition.h"

class QModelIndex;
class QSettings;

class FlarmRecord;
class FlarmList;
class GpsTracker;
class QResizeEvent;

class FlarmMapWidget: public PlotWidget
{
		Q_OBJECT

	public:
		enum KmlStatus { kmlNone, kmlNotFound, kmlReadError, kmlParseError, kmlEmpty, kmlOk };

		struct StaticPath
		{
			StaticPath (const QVector<GeoPosition> &points, const QString &name, const QPen &pen);
			StaticPath (const Kml::Path    &path   , const Kml::Style &style);
			QVector<GeoPosition> points;
			QString name;
			QPen pen;
		};

		struct StaticPolygon
		{
			StaticPolygon (const QVector<GeoPosition> &points, const QString &name, const QPen &pen, const QBrush &brush);
			StaticPolygon (const Kml::Polygon &polygon, const Kml::Style &style);
			QVector<GeoPosition> points;
			QString name;
			QPen pen;
			QBrush brush;
		};

		struct StaticMarker
		{
			StaticMarker (const GeoPosition &position, const QString &text, const QColor &backgroundColor);
			StaticMarker (const Kml::Marker &marker, const Kml::Style &style);
			GeoPosition position;
			QString text;
			QColor backgroundColor;
		};

		struct Image
		{
			Image (const Kml::GroundOverlay &groundOverlay);

			QPixmap pixmap;
			GeoPosition northEast, southWest;
			GeoPosition northWest, southEast;
			Angle rotation;
			QColor color;
		};

		struct PlaneMarker
		{
			enum Style { invisible, minimal, verbose };

			Style style;
			QPointF position_p;
			QString text;
			QPolygonF trail_p;
			QColor color;
		};

		FlarmMapWidget (QWidget *parent);
		virtual ~FlarmMapWidget ();

		// State
		void loadState (QSettings &settings);
		void saveState (QSettings &settings);

		// Static data
		void setOwnPositionLabel (const QString &text, const QColor &color);

		// KML
		KmlStatus readKml (const QString &filename);
		KmlStatus getKmlStatus () const;

		// Flarm list
		void setFlarmList (FlarmList *flarmList);
		void setGpsTracker (GpsTracker *gpsTracker);

		// Status
		bool isOwnPositionKnown () const;

		// View
		bool isOwnPositionVisible () const;
		bool isAnyStaticElementVisible () const;
		bool findClosestStaticElement (double *distance, Angle *bearing) const;
		void resetPosition ();


	signals:
		void ownPositionUpdated () const;

		// ***** Painting
	protected:
		virtual void paintEvent (QPaintEvent *event);
		virtual void paintNorthDirection (QPainter &painter);
		virtual void paintDistanceCircles (QPainter &painter);
		virtual void paintLatLonGrid (QPainter &painter);
		virtual void paintStaticPaths (QPainter &painter);
		virtual void paintStaticPolygons (QPainter &painter);
		virtual void paintStaticMarkers (QPainter &painter);
		virtual void paintImages (QPainter &painter);
		virtual void paintPlanes (QPainter &painter);
		virtual void paintOwnPosition (QPainter &painter);





	private:
		// Settings
		QColor _ownPositionColor, _climbColor, _descentColor;
		QString _ownPositionText;

		// Flarm list
		FlarmList *flarmList;
		GpsTracker *gpsTracker;

		GeoPosition _ownPosition;

		// Static curves and Flarm data
		QList<StaticPath> staticPaths;
		QList<StaticPolygon> staticPolygons;
		QList<StaticMarker> staticMarkers;
		QList<Image> images;

		QList<GeoPosition> allStaticPositions;
		QHash<QString, PlaneMarker> planeMarkers;
		GeoPosition kmlSouthEast, kmlNorthWest;

		// Status
		KmlStatus kmlStatus;

		// Static data
		void updateStaticData ();

		// Flarm data - Flarm list changes
		virtual void addPlaneMarker (const FlarmRecord &record);
		virtual void updatePlaneMarker (const FlarmRecord &record);
		virtual void removePlaneMarker (const FlarmRecord &record);
		virtual void refreshPlaneMarkers ();

		// KML
		KmlStatus readKmlImplementation (const QString &filename);

		Angle transformWidgetToGeographic (double distance_w) const;

		double    transformGeographicToWidget (const Angle &greatCircleAngle) const;
		QPointF   transformGeographicToWidget (const GeoPosition &geoPosition) const;
		QPolygonF transformGeographicToWidget (const QVector<GeoPosition> &geoPositions) const;


	private slots:
		// FlarmList model slots
		void rowsInserted (const QModelIndex &parent, int start, int end);
		void dataChanged (const QModelIndex &topLeft, const QModelIndex &bottomRight);
		void rowsAboutToBeRemoved (const QModelIndex &parent, int start, int end);
		void modelReset ();
		void flarmListDestroyed ();

	public slots:
		virtual void ownPositionChanged (const GeoPosition &ownPosition);


	private:
		QAction showImagesAction;
		QAction showGridAction;
		QAction showCirclesAction;

};

#endif
