#ifndef SKTABLEVIEW_H_
#define SKTABLEVIEW_H_

#include <QTableView>

#include "src/accessor.h"

class SkItemDelegate;

class ColumnInfo;
class QSettings;
class TableButton;
class QMouseEvent;

/**
 * Adds some functionality to QTableView:
 *   - on double click on empty (non-cell) space, doubleClicked is emitted with
 *     an invalid index
 *   - colored selections via SkItemDelegate
 *   - automatic resizing of rows and columns
 *   - ...
 */
class SkTableView: public QTableView
{
	Q_OBJECT

	public:
		// Construction
		SkTableView (QWidget *parent=NULL);
		virtual ~SkTableView ();

		// Property access
		virtual void setModel (QAbstractItemModel *model);
		virtual void setEffectiveModel (QAbstractItemModel *model) { setModel (model); }
		virtual QAbstractItemModel *getEffectiveModel () { return model (); }

		value_accessor (bool, AutoResizeRows   , autoResizeRows   );
		value_accessor (bool, AutoResizeColumns, autoResizeColumns);

		// Settings
		void setColoredSelectionEnabled (bool value);
		bool getColoredSelectionEnabled ();
		void readColumnWidths (QSettings &settings, const ColumnInfo &columnInfo);
		void writeColumnWidths (QSettings &settings, const ColumnInfo &columnInfo);

		bool cellVisible (const QModelIndex &index);
		QWidget *findVisibleWidget (const QModelIndexList &indexes);
		QPersistentModelIndex findButton (TableButton *button);
		bool focusWidgetAt (const QModelIndex &index);


	public slots:
		virtual void reset ();
		virtual void layoutChanged ();

	signals:
		void buttonClicked (QPersistentModelIndex index);


	protected slots:
//		virtual void rowsAboutToBeRemoved (const QModelIndex &parent, int start, int end);
		virtual void rowsInserted (const QModelIndex &parent, int start, int end);
        virtual void dataChanged (const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles = QVector<int>());
		virtual void currentChanged (const QModelIndex &current, const QModelIndex &previous);
		virtual void selectionChanged (const QItemSelection &selected, const QItemSelection &deselected);

	protected:
		void updateButtons (int row);
		//void updateWidgetFocus (const QModelIndexList &indexes);
		virtual void mouseDoubleClickEvent (QMouseEvent *event);
		virtual void mousePressEvent (QMouseEvent *event);
		virtual void keyPressEvent (QKeyEvent *e);
		void scrollLeft ();
		void scrollRight ();


	private:
		void init ();


		// HACK: updateButtons calls setIndexWidget which calls dataChanged (as
		// of Qt 4.5 - 4.3 didn't) which usually calls updateButtons. This is
		// to avoid this recursion. This may not be the best solution for this
		// problem.
		bool settingButtons;

		bool autoResizeRows;
		bool autoResizeColumns;

		SkItemDelegate *itemDelegate;
};

#endif
