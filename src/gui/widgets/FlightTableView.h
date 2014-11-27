#ifndef FLIGHTTABLEVIEW_H_
#define FLIGHTTABLEVIEW_H_

#include "SkTableView.h"

#include <QList>
#include <QHash>

#include "src/db/dbId.h"
#include "src/db/DbManager.h"
#include "src/FlightReference.h"

class QSettings;

template<class T> class EntityList;
class FlightProxyList;
class FlightModel;
template<class T> class ObjectListModel;
class FlightSortFilterProxyModel;
class DbManager;
class NotificationWidget;
class NotificationsLayout;


/**
 * An SkTableView implementation with some flight specific functionality
 *
 * This model view uses a an EntityList<FLight> as a model and maintains other
 * required models (flight proxy model, flight model, object list model, and
 * flight sort/filter proxy model) internally. There are some methods for
 * accessing flights, or determining or changing the currently selected flight.
 * Appropriate signals are emitted when a table button is pressed.
 *
 * The table can be sorted by column (as usual) or by the effective time of the
 * flights. Clicking the table header toggles between ascending and descending
 * sorting (as usual) and effective time sorting. Since this requires custom
 * handling of the sort order, QTableView's method sortByColumn should not be
 * used.
 *
 * Additionally, this class handles notifications for flights (using
 * NotificationWidget), updating the position and visibility of the
 * notifications when the table contents change (e. g. flights are added or
 * removed, or the order changes). Flights that would normally be hidden are
 * kept visible for as long as a notification is visible for the flight.
 *
 * Note that this view class also contains some models internally, in addition
 * to the model proper, which is set from outside as usual.
 */
class FlightTableView: public SkTableView
{
		Q_OBJECT

	public:
		FlightTableView (QWidget *parent);
		virtual ~FlightTableView ();
		void init (DbManager *dbManager);

		void setModel (EntityList<Flight> *flightList);

		FlightReference selectedFlightReference ();
		bool selectFlight (const FlightReference &flight, int column);

		void readColumnWidths (QSettings &settings);
		void writeColumnWidths (QSettings &settings);

		// Model
		QModelIndex getModelIndex (const FlightReference &flightReference, int column) const;
		FlightReference getFlightReference (const QModelIndex &modelIndex) const;

		// View
		QRect rectForFlight (const FlightReference &flight, int column) const;


	public slots:
		// Properties
		void setHideFinishedFlights (bool hideFinishedFlights);
		void setAlwaysShowExternalFlights (bool alwaysShowExternalFlights);
		void setAlwaysShowErroneousFlights (bool alwaysShowErroneousFlights);
		void setShowPreparedFlights (bool showPreparedFlights);

		// Configuration
		void languageChanged ();

		// Model
		void minuteChanged ();
		// Selection
		void interactiveJumpToTowflight ();


		// Sorting
		void setSorting (int column, Qt::SortOrder order);
		void setCustomSorting ();
		void toggleSorting (int column);

		// Notifications
		void showNotification (const FlightReference &flight, const QString &message, int milliseconds);


	signals:
		/** Emitted when the depart button of a flight is clicked */
		void departButtonClicked (FlightReference flight);

		/** Emitted when the land button of a flight is clicked */
		void landButtonClicked (FlightReference flight);

	protected slots:
		void base_buttonClicked (QPersistentModelIndex proxyIndex);
		void layoutNotifications ();
		void notificationWidget_closed ();

	private:
		DbManager *_dbManager;

		// The models involved in displaying the flight list
		EntityList<Flight> *_flightList;
		FlightProxyList *_proxyList;
		FlightModel *_flightModel;
		ObjectListModel<Flight> *_flightListModel;
		FlightSortFilterProxyModel *_proxyModel;

		int _sortColumn; // -1 for custom
		Qt::SortOrder _sortOrder;

		QHash<NotificationWidget *, FlightReference> notifications;
		NotificationsLayout *notificationsLayout;
};

#endif
