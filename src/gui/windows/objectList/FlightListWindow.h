#ifndef FLIGHTLISTWINDOW_H
#define FLIGHTLISTWINDOW_H

#include "ui_FlightListWindow.h"

#include <QDate>

#include "src/db/DbManager.h" // Required for DbManager::State
#include "src/gui/SkMainWindow.h"

class FlightSortFilterProxyModel;

class FlightModel;
template<class T> class MutableObjectList;
template<class T> class ObjectListModel;

/**
 * A window for showing and exporting a flight list
 *
 * Note that this window is not based on ObjectListWindow as it is significantly
 * different from other object list windows:
 *   - it does not allow creating and editing of objects
 *   - the data it displays is not cached by the database
 *   - it allow retrieving different sets of data (by date)
 */
class FlightListWindow: public SkMainWindow<Ui::FlightListWindowClass>
{
	Q_OBJECT

	public:
		FlightListWindow (DbManager &manager, QWidget *parent=0);
		~FlightListWindow();

		static void show (DbManager &manager, QWidget *parent);

		using QMainWindow::show;

	public slots:
		virtual void on_actionClose_triggered ();

		virtual void on_actionSelectDate_triggered ();
		virtual void on_actionRefresh_triggered ();

		virtual void on_actionExport_triggered ();

	protected:
		bool fetchFlights (const QDate &first, const QDate &last);

		void updateLabel ();

		// Events
		void keyPressEvent (QKeyEvent *e);
		void languageChanged ();

	protected slots:
		virtual void databaseStateChanged (DbManager::State state);

	private:
		DbManager &manager;

		QDate currentFirst, currentLast;

		MutableObjectList<Flight> *flightList;
		FlightModel *flightModel;
		ObjectListModel<Flight> *flightListModel;

        FlightSortFilterProxyModel *proxyModel;
};

#endif
