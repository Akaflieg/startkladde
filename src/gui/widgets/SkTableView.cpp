/*
  * Improvements:
 *   - horizontal scrolling using the scroll bar (mouse) does not update the
 *     widget focus (updateWidgetFocus is not called)
 *   - updating the data (e. g. flight duration on minute change) recreates the
 *     buttons, even if not necessary; this is probably inefficient. Also, it
 *     necessitates determining the index of the focused button
 *     (focusWidgetIndex) beforehand and focusing the widget at that index
 *     afterwards (focusWidgetAt) (from MainWindow)
 *   - on every move, the current selection has to be searched for a visible
 *     button in order to set the focus; this could possibly be made more
 *     efficient
 *   - use a QTreeView instead? May be useful for horizontal scrolling
 *   - use a custom delegate for rendering the buttons? Might be faster and
 *     provide better control over the focus (which is disabled at the moment
 *     due to poor performance)
 *   - Don't mention flights in the documentation, it's not flight specific
 */

/*
 * The selection color scheme is implemented by using an SkItemDelegate. Style
 * sheets could also be used by changing the style sheet every time the
 * selection is changed, but this only allows one single color (scheme), even
 * if multiple indexes with different background colors are selected. Also,
 * setting a style sheet is VERY SLOW, don't do it. The palette should not be
 * used as styles are allowed to ignore the palette.
 *
 * Using a style sheet works like this:
 *   QColor color=index.data (Qt::BackgroundRole).value<QBrush> ().color ();
 *   if (color.isValid ())
 *     setStyleSheet (QString ("...: %1;").arg (color.name ()));
 *
 * Examples:
 *   Item color on dark gray:
 *     selection-background-color: #3F3F3f; selection-color: %1;
 *   Fake border:
 *     selection-color: #000000;
 *     selection-background-color:
 *       qlineargradient(
 *         x1: 0, y1: 0, x2: 0, y2: 1,
 *         stop: 0 #000000, stop: 0.2 %1, stop: 0.8 %1, stop: 1 #000000);
 *   Vertical gradient:
 *     selection-color: #000000;
 *     selection-background-color:
 *       qlineargradient(
 *         x1: 0, y1: 0, x2: 0, y2: 1,
 *         stop: 0 %1, stop: 1 %2);
 *     With color.darker(135).name() and color.lighter(135).name()
 *
 * WARNING: in some styles (e. g. Gnome), gradients appear as solid black
 *
 * Note: setting a style sheet on every selection change is VERY SLOW. Don't
 * do it.
 *
 * In short:
 *   - Don't use a style sheet - it's slow.
 *   - Don't use a palette - it may be ignored by the style
 */

#include "SkTableView.h"

#include <QDebug>
#include <QSettings>
#include <QFont>
#include <QFontMetrics>
#include <QHeaderView>
#include <QApplication>
#include <QStyle>
#include <QKeyEvent>

#include "src/gui/widgets/TableButton.h"
#include "src/itemDataRoles.h"
#include "src/concurrent/threadUtil.h" // Required for assert (isGuiThread ());
#include "src/model/objectList/ColumnInfo.h"
#include "src/util/color.h"
#include "src/util/qString.h"
#include "src/util/io.h" // remove
#include "src/gui/views/SkItemDelegate.h"
#include "src/i18n/notr.h"

#include <iostream>
#include <cassert>

SkTableView::SkTableView (QWidget *parent):
	QTableView (parent),
	settingButtons (false),
	autoResizeRows (false), autoResizeColumns (false),
	itemDelegate (new SkItemDelegate (this))
{
	setTabKeyNavigation (false);

	setItemDelegate (itemDelegate);
}

SkTableView::~SkTableView ()
{
}

void SkTableView::setModel (QAbstractItemModel *model)
{
	QTableView::setModel (model);

	QObject::disconnect (this, SLOT (layoutChanged ()));

	if (model)
		connect (model, SIGNAL (layoutChanged ()), this, SLOT (layoutChanged ()));
}

void SkTableView::layoutChanged ()
{
	// This happens when the SortFilterModel filter settings are changed
	if (autoResizeRows)
		resizeRowsToContents ();

	if (autoResizeColumns)
		resizeColumnsToContents ();
}

void SkTableView::updateButtons (int row)
{
	// TODO: this should not happen, but when opening the person or plane
	// editor, it sometimes does (note that the models have no buttons in this
	// case).
//	assert (!settingButtons);
	if (settingButtons) return;

	QAbstractItemModel *m=model ();
	int columns=m->columnCount ();

	for (int column=0; column<columns; ++column)
	{
		QModelIndex index=m->index (row, column);

		if (m->data (index, isButtonRole).toBool ())
		{
			QString buttonText=m->data (index, buttonTextRole).toString ();
			TableButton *button=new TableButton (index, buttonText);
			QObject::connect (button, SIGNAL (clicked (QPersistentModelIndex)), this, SIGNAL (buttonClicked (QPersistentModelIndex)));

			// Avoid recursive calls, see above
			settingButtons=true;
			setIndexWidget (index, button);
			settingButtons=false;
		}
		else
		{
			// Avoid recursive calls, see above
			settingButtons=true;
			setIndexWidget (index, NULL);
			settingButtons=false;
		}
	}
}

//void SkTableView::rowsAboutToBeRemoved (const QModelIndex &parent, int start, int end)
//{
//}

void SkTableView::rowsInserted (const QModelIndex &parent, int start, int end)
{
	QTableView::rowsInserted (parent, start, end);

	for (int i=start; i<=end; ++i)
		updateButtons (i);

	// If a row is inserted, the rows after that get renumbered. A row that
	// gets an index that did not exist before may be resized to the default
	// size (at least with 4.3.4). This may be a bug in Qt.
	// Workaround: resize all rows, not only the ones that were inserted.

	if (autoResizeRows)
		resizeRowsToContents ();

	if (autoResizeColumns)
		resizeColumnsToContents ();
}

void SkTableView::dataChanged (const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
	assert (isGuiThread ());

	QTableView::dataChanged (topLeft, bottomRight);

	if (settingButtons) return;

	for (int i=topLeft.row (); i<=bottomRight.row (); ++i)
		updateButtons (i);

	if (autoResizeRows)
		for (int i=topLeft.row (); i<=bottomRight.row (); ++i)
			resizeRowToContents (i);

	if (autoResizeColumns)
		for (int i=topLeft.column (); i<=bottomRight.column (); ++i)
			resizeColumnToContents (i);

	// The button focus may be lost after a prepared flight has been edited.
	// Don't update the focus, it is slow
//	updateWidgetFocus (selectionModel ()->selectedIndexes ());
}

void SkTableView::reset ()
{
	// Strange things happening here:
	// Calling QTableView::reset here seems to *sometimes* cause a segfault
	// (when refreshing), probably related to the table buttons, like so:
	//
	// #0  0x01098b9c in QObject::disconnect(QObject const*, char const*, QObject const*, char const*)
	// #1  0x00a4b1f9 in QAbstractItemView::reset() [the following call]
	// #2  0x080ac6ed in SkTableView::reset [this method]
	//
	// This problem is hard to reproduce. On some runs of the program, it does
	// not appear at all. Restarting the program may help in reproducing the
	// problem.
	//
	// Note that this is the case even if the button signal is not connected.
	// Note also that QAbstractItemView::reset does not seem to call
	// QObject::disconnect at all.
	//
	// Now, the funny thing is: calling QTableView::reset does not even seem to
	// be necessary here, the table is still refreshed and all of the buttons
	// are deleted. Also, not calling QTableView::reset seems to fix the
	// problem.
//	QTableView::reset (); // DO NOT CALL!

	if (!model ())
		return;

	// Set up the buttons
	int rows=model ()->rowCount ();
	for (int row=0; row<rows; ++row)
		updateButtons (row);

	if (autoResizeRows)
		resizeRowsToContents ();

	if (autoResizeColumns)
		resizeColumnsToContents ();
}

void SkTableView::keyPressEvent (QKeyEvent *e)
{
//	std::cout << "key " << e->key () << "/" << e->modifiers () << " pressed in SkTableView" << std::endl;

	switch (e->key ())
	{
		// Hack: it seems that as of Qt 4.6.2 (Ubuntu Lucid), QTableView consumes
		// the delete key, which is not passed to the parent widget (the containing
		// window). This only seems to happen with the delete key proper, not the
		// keypad delete key, even though both have a value of Qt::Key_Delete.
		// Ignore the delete key here to propagate it to the parent widget.
		case Qt::Key_Delete: e->ignore (); break;
		case Qt::Key_Left: scrollLeft (); break;
		case Qt::Key_Right: scrollRight (); break;
		case Qt::Key_Return:
		case Qt::Key_Enter:
			e->ignore (); // Don't call base, it will accept it
			break;
		default:
			e->ignore (); // Propagate to parent widget (unless the QTableView accepts it)
			QTableView::keyPressEvent (e);
			break;
	}
}

// Current cell changed - row (flight) or column
void SkTableView::currentChanged (const QModelIndex &current, const QModelIndex &previous)
{
	QTableView::currentChanged (current, previous);

	// If no cells are selected, explicitly make the current index current, so
	// the row gets selected. This may seem redundant, but it seems to work.
	if (!selectionModel ()->hasSelection ())
		setCurrentIndex (current);
}

bool SkTableView::cellVisible (const QModelIndex &index)
{
	QRect viewportRect=viewport ()->rect ();
	QRect indexRect=visualRect (index);

	return
		viewportRect.contains (indexRect.topLeft ()) &&
		viewportRect.contains (indexRect.bottomRight ())
		;
}

QWidget *SkTableView::findVisibleWidget (const QModelIndexList &indexes)
{
	foreach (const QModelIndex &index, indexes)
	{
		QWidget *widget=indexWidget (index);

		if (widget)
			if (cellVisible (index))
				return widget;
	}

	return NULL;
}

/**
 * @param button may be NULL
 * @return
 */
QPersistentModelIndex SkTableView::findButton (TableButton *button)
{
	if (!button) return QPersistentModelIndex ();

	return button->getIndex ();
}

bool SkTableView::focusWidgetAt (const QModelIndex &index)
{
	if (!index.isValid ()) return false;

	QWidget *widget=indexWidget (index);
	if (!widget) return false;

	widget->setFocus ();
	return true;
}

/**
 * This method is slow, don't use it. It's only left here for reference, until
 * a better solution is available.
 *
 * More specifically, it's the setFocus call that is slow.
 *
 * If this method is called on every change of the current item in the table,
 * scrolling is slowed down noticeably for rows with a button.
 *
 * @param indexes
 */
//void SkTableView::updateWidgetFocus (const QModelIndexList &indexes)
//{
//	(void)indexes;
//	assert (!notr ("This method should not be used, it's slow."));
//
//	// If the current selection contains a widget, focus it if it is visible
//	QWidget *widget=findVisibleWidget (indexes);
//	if (widget)
//		widget->setFocus (Qt::OtherFocusReason);
//	else
//		this->setFocus (Qt::OtherFocusReason);
//}


void SkTableView::scrollLeft ()
{
	if (!selectionModel ()) return;

	// The current row is selected completely if there is a current
	// index - see currentChanged
	QList<QModelIndex> indexes=selectionModel ()->selectedIndexes ();

	// Find the first visible cell and scroll to the one before it
	QModelIndex lastIndex;
	QListIterator<QModelIndex> i (indexes);
	while (i.hasNext ())
	{
		QModelIndex index=i.next ();
		if (cellVisible (index))
		{
			if (lastIndex.isValid ())
			{
				setCurrentIndex (lastIndex);
				// Don't update the focus, it is slow
				//updateWidgetFocus (selectionModel ()->selectedIndexes ());
			}
			return;
		}

		lastIndex=index;
	}

	// Don't update the focus, it is slow
	//updateWidgetFocus (selectionModel ()->selectedIndexes ());
}

void SkTableView::scrollRight ()
{
	if (!selectionModel ()) return;

	// The current row is selected completely if there is a current
	// index - see currentChanged
	QList<QModelIndex> indexes=selectionModel ()->selectedIndexes ();

	// Find the first visible cell and scroll to the one before it
	QModelIndex lastIndex;
	QListIterator<QModelIndex> i (indexes);
	i.toBack ();
	while (i.hasPrevious ())
	{
		QModelIndex index=i.previous ();
		if (cellVisible (index))
		{
			if (lastIndex.isValid ())
			{
				setCurrentIndex (lastIndex);
				// Don't update the focus, it is slow
				//updateWidgetFocus (selectionModel ()->selectedIndexes ());
			}
			return;
		}

		lastIndex=index;
	}
}

void SkTableView::mouseDoubleClickEvent (QMouseEvent *event)
{
	if (indexAt (event->pos ()).isValid ())
		QTableView::mouseDoubleClickEvent (event);
	else
	{
		emit doubleClicked (QModelIndex ());
		event->accept ();
	}
}

void SkTableView::mousePressEvent (QMouseEvent *event)
{
	QTableView::mousePressEvent (event);

	if (!selectionModel ()) return;

	// The button focus may be lost when another cell of the
	// currently selected (!) flight is clicked
	// Don't update the focus, it is slow
	//updateWidgetFocus (selectionModel ()->selectedIndexes ());
}

/**
 * Reads the column widths from the settings or uses defaults from a columnInfo
 *
 * The settings object has to be set to the correct section. The widths are
 * read from the value columnWidth_(name) where name is the column name from
 * columnInfo.
 *
 * If no width is stored in settings for a given column, the sample text from
 * columnInfo and the column title from the model are used to determine a
 * default width.
 *
 * @param settings the QSettings to read the widths from
 * @param columnInfo the ColumnInfo to read the default widths from
 */
void SkTableView::readColumnWidths (QSettings &settings, const ColumnInfo &columnInfo)
{
	if (!getEffectiveModel ()) return;

	// The column info set must have the same number of columns as the model of
	// this table.
	// FIXME this fails when the Flarm debug columns are filtered out (?). Do
	// we really need this? Do we need to proxy the columnInfo? Should we refer
	// to the columns by name rather than index?
	//qDebug () << columnInfo.columnCount () << getEffectiveModel ()->columnCount ();
	//assert (columnInfo.columnCount ()==getEffectiveModel ()->columnCount ());

	for (int i=0; i<columnInfo.columnCount (); ++i)
	{
		// Determine the column name and the settings key
		QString columnName=columnInfo.columnName (i);
		QString key=qnotr ("columnWidth_%1").arg (columnName);

		// Determine the font metrics and the frame margin
		const QFont &font=horizontalHeader ()->font ();
		QFontMetrics metrics (font);
		QStyle *style=horizontalHeader ()->style ();
		if (!style) style=QApplication::style ();
		// Similar to QItemDelegate::textRectangle
		const int margin=style->pixelMetric (QStyle::PM_FocusFrameHMargin)+1;

		if (settings.contains (key))
		{
			// The settings contain a width for this column
			setColumnWidth (i, settings.value (key).toInt ());
		}
		else
		{
			// No width for this column in the settings. Determine the default.
			QString sampleText=columnInfo.sampleText (i);
			QString headerText=model ()->headerData (i, Qt::Horizontal).toString ();

			// The 2/4 were determined experimentally. Probably, some metric
			// should be used. For headerWidth, +2 is enough on Linux/Gnome,
			// but not on Windows XP.
			int sampleWidth=metrics.boundingRect (sampleText).width ()+2*margin+2;
			int headerWidth=metrics.boundingRect (headerText).width ()+2*margin+4;

			setColumnWidth (i, qMax (sampleWidth, headerWidth));
		}
	}
}

void SkTableView::writeColumnWidths (QSettings &settings, const ColumnInfo &columnInfo)
{
	if (!getEffectiveModel ()) return;

	// FIXME this fails when the Flarm debug columns are filtered out (?). See
	// the same assertion above.
	//assert (columnInfo.columnCount ()==getEffectiveModel ()->columnCount ());

	for (int i=0; i<columnInfo.columnCount (); ++i)
	{
		QString columnName=columnInfo.columnName (i);
		QString key=qnotr ("columnWidth_%1").arg (columnName);
		int value=columnWidth (i);

		settings.setValue (key, value);
	}
}



// Selection changed - since selectionBehavior is SelectRows, this means that a
// different flight (or none) was selected
void SkTableView::selectionChanged (const QItemSelection &selected, const QItemSelection &deselected)
{
	// Don't update the focus, it is slow
	//updateWidgetFocus (selected.indexes ());
	QTableView::selectionChanged (selected, deselected);
}


void SkTableView::setColoredSelectionEnabled (bool value)
{
	itemDelegate->setColoredSelectionEnabled (value);
}

bool SkTableView::getColoredSelectionEnabled ()
{
	return itemDelegate->getColoredSelectionEnabled ();
}
