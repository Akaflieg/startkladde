#include "SkComboBox.h"

#include <QLineEdit>
#include <QCompleter>
#include <QFocusEvent>

/**
 * Constructs a SkComboBox
 *
 * @param parent the parent widget, passed to the QComboBox constructor
 */
SkComboBox::SkComboBox (QWidget *parent)
	:QComboBox (parent)
{
	connect (this, SIGNAL (activated (const QString &)), this, SIGNAL (textEdited (const QString &)));
	connect (this, SIGNAL (activated (const QString &)), this, SIGNAL (editingFinished (const QString &)));
}

SkComboBox::~SkComboBox ()
{
}

void SkComboBox::setEditable (bool editable)
{
	QLineEdit *oldLineEdit=lineEdit ();
	if (oldLineEdit)
		disconnect (this, NULL, oldLineEdit, NULL);

	QComboBox::setEditable (editable);

	// Setup the lineEdit. Note that this cannot be done in setLineEdit because
	// this method is not called (maybe because it is not virtual).

	QLineEdit *newLineEdit=lineEdit ();
	if (newLineEdit)
	{
		connect (newLineEdit, SIGNAL (textEdited (const QString &)), this, SIGNAL (textEdited (const QString &)));
		connect (newLineEdit, SIGNAL (editingFinished ()), this, SLOT (lineEdit_editingFinished ()));
	}

	// Here, we can setup the completer. What we want is popup *and*
	// inline completion (TODO).
	// Here is an example:
	// http://doc.trolltech.com/4.2/tools-customcompleter-textedit-cpp.html
	// It goes like this: set popup completion, connect the activated signal
	// and do inline yourself. However, in popup mode, activated doesn't
	// seem to be emitted. (QT version: 4.3.4)
	QCompleter *c=completer ();
	c->setCaseSensitivity (Qt::CaseInsensitive);
	//c->setCompletionMode (QCompleter::PopupCompletion);
	c->setCompletionMode (QCompleter::InlineCompletion);
	setCompleter (c);

//	QObject::connect(c, SIGNAL(activated(const QString&)), this, SLOT(comp(const QString&)));
//	QObject::connect(c, SIGNAL(highlighted(const QString&)), this, SLOT(comp(const QString&)));

//	QObject::connect(this, SIGNAL(activated(const QString&)), this, SLOT(comp(const QString&)));
//	QObject::connect(this, SIGNAL(highlighted(const QString&)), this, SLOT(comp(const QString&)));

}

//void SkComboBox::comp (const QString &text)
//{
//	std::cout << "da slot: " << text << std::endl;
//}


void SkComboBox::focusInEvent (QFocusEvent *event)
{
	QComboBox::focusInEvent (event);

	QLineEdit *le=lineEdit ();
	if (le)
	{
		Qt::FocusReason reason=event->reason ();

		// We don't always want to select the text:
		//   - not when the window is activated
		//   - not when the completion list is closed (typing D-X when no
		//     D-X... is in the list)
		if (reason==Qt::MouseFocusReason ||
			reason==Qt::TabFocusReason ||
			reason==Qt::BacktabFocusReason ||
			reason==Qt::ShortcutFocusReason)
		{
			if (currentText ().startsWith (defaultPrefix))
			{
				le->setCursorPosition (defaultPrefix.length ());
				le->end (true);
			}
		}
	}
}

QVariant SkComboBox::currentItemData (int role)
{
	int index=currentIndex ();
	if (index<0) return QVariant ();
	return itemData (index, role);
}

int SkComboBox::indexByItemData (const QVariant &value, int defaultIndex)
{
	for (int i=0, n=count (); i<n; ++i)
		if (itemData (i)==value)
			return i;

	return defaultIndex;
}



bool SkComboBox::setCurrentItemByItemData (QVariant value)
{
	for (int i=0; i<count (); ++i)
	{
		if (itemData (i)==value)
		{
			setCurrentIndex (i);
			return true;
		}
	}

	return false;
}

void SkComboBox::setCurrentItemByItemData (QVariant itemData, int defaultIndex)
{
	if (!setCurrentItemByItemData (itemData))
		setCurrentIndex (defaultIndex);
}

void SkComboBox::setItemTextByItemData (const QVariant &itemData, const QString &text)
{
	int index=indexByItemData (itemData, -1);
	if (index>=0)
		setItemText (index, text);
}
