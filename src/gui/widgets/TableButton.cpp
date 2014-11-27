#include "TableButton.h"

#include <QKeyEvent>

#include <iostream>

TableButton::TableButton (QPersistentModelIndex index, QWidget *parent):
	QPushButton (parent),
	index (index)
{
	init ();
}

TableButton::TableButton (QPersistentModelIndex index, const QString &text, QWidget *parent):
	QPushButton (text, parent),
	index (index)
{
	init ();
}

TableButton::TableButton (QPersistentModelIndex index, const QIcon &icon, const QString &text, QWidget *parent):
	QPushButton (icon, text, parent),
	index (index)
{
	init ();
}

TableButton::~TableButton ()
{
//	std::cout << "-button" << std::endl;
//	std::cout << "-" << std::flush;
}

void TableButton::init ()
{
//	std::cout << "+button" << std::endl;
//	std::cout << "+" << std::flush;

	QObject::connect (this, SIGNAL (clicked ()), this, SLOT (clickedSlot ()));
//	setText (qnotr ("[%1, %2]").arg (index.row  ()).arg (index.column ()));

	// Don't accept keyboard focus - the tab order of buttons in the table is
	// not defined. Don't accept mouse focus - space bar on the table is handled
	// explicitly and a focused button, when different from the current row,
	// would cause confusion as to which row is affected by a space bar press.
	setFocusPolicy (Qt::NoFocus);
}

void TableButton::clickedSlot ()
{
//	setText (qnotr ("{%1, %2}").arg (index.row  ()).arg (index.column ()));
	emit clicked (index);
}

QSize TableButton::sizeHint () const
{
	QSize size=QPushButton::sizeHint ();
	size.setHeight (0);
	return size;
}

void TableButton::keyPressEvent (QKeyEvent *event)
{
//	std::cout << "key " << event->key () << " pressed in TableButton" << std::endl;

	switch (event->key ())
	{
		case Qt::Key_Space:
			// Handle space bar (button press)
			QPushButton::keyPressEvent (event);
			break;
		default:
			// Ignore all other events
			event->ignore ();
			break;
	}


	// Ignore all other keys
	event->ignore ();

//	// Ignore the cursor keys so they are handled by the table view. Otherwise,
//	// the cursor keys would change the focused button, and only when there is
//	// no button in the given direction would the key be passed to the table
//	// view and change the selection.
//	if (
//		event->key ()==Qt::Key_Down ||
//		event->key ()==Qt::Key_Up ||
//		event->key ()==Qt::Key_Left ||
//		event->key ()==Qt::Key_Right)
//		event->ignore ();
//	else
//		QPushButton::keyPressEvent (event);
}
