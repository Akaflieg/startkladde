#include "CompletionLineEdit.h"
#include "SkCompleter.h"
#include <QAbstractItemView>
#include <QKeyEvent>
#include <QPainter>
#include "qdebug.h"

CompletionLineEdit::CompletionLineEdit(QWidget* parent) :
    QLineEdit(parent)
{
    c = NULL;
    itemSelected = false;
    this->installEventFilter(this);
}

CompletionLineEdit::~CompletionLineEdit()
{

}

bool CompletionLineEdit::isItemSelected() const
{
    return itemSelected;
}

QVariant CompletionLineEdit::getSelectedItem() const
{
    return selectedItem;
}

void CompletionLineEdit::setSelectedItem(QVariant item)
{
    updateSelection(true, item);
}

void CompletionLineEdit::setCompleter(SkCompleter* completer)
{
    completer->setParent(this);

    if (c != NULL) QObject::disconnect(c, 0, this, 0);

    c = completer;

    if (c == NULL) return;

    c->setWidget(this);
    connect(completer, SIGNAL(selected(QVariant)), this, SLOT(insertCompletion(QVariant)));
}

SkCompleter* CompletionLineEdit::completer() const
{
    return c;
}

void CompletionLineEdit::insertCompletion(QVariant sel)
{
    updateSelection(true, sel);
}

void CompletionLineEdit::adaptColor()
{
    QPalette p = palette();

    if (itemSelected)
        p.setBrush(QPalette::Base, QBrush(QColor(184, 255, 184)));
    else
        p.setBrush(QPalette::Base, QBrush(QColor(255, 255, 255)));

    setPalette(p);
}

void CompletionLineEdit::updateSelection(bool s, QVariant item)
{
    if (!s) {
        setText("");
    } else {
        setText(c->itemToString(item));
    }
    itemSelected = s;
    selectedItem = item;
    adaptColor();
    emit selectionStateChanged();
}

void CompletionLineEdit::keyPressEvent(QKeyEvent *e)
{
    // qDebug() << "press " << e->key();
    // qDebug() << (int) e->modifiers();

    if (e->key() == Qt::Key_Enter) {
        e->accept();
        return;
    }

    /*if (e->key() == Qt::Key_Alt) {
        qDebug() << "ALT!";
    }*/

    if (itemSelected &&
         e->key() != Qt::Key_Right &&
         e->key() != Qt::Key_Left &&
         e->key() != Qt::Key_Alt &&
         e->key() != Qt::Key_AltGr) {
        updateSelection(false, QVariant());
    }

    /*if (c && c->popup()->isVisible())
    {
        // The following keys are forwarded by the completer to the widget
        switch (e->key())
        {
        case Qt::Key_Enter:
        case Qt::Key_Return:
        case Qt::Key_Escape:
        case Qt::Key_Tab:
        case Qt::Key_Backtab:
            e->ignore();
            return; // Let the completer do default behavior
        }
    }*/

    //bool isShortcut = (e->modifiers() & Qt::ControlModifier) && e->key() == Qt::Key_E;
    //if (!isShortcut)
    QLineEdit::keyPressEvent(e); // Don't send the shortcut (CTRL-E) to the text edit.

    if (c != NULL && e->key() != Qt::Key_Alt && e->key() != Qt::Key_AltGr)
    {
        /*bool ctrlOrShift = e->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier);
        if (!isShortcut && !ctrlOrShift && e->modifiers() != Qt::NoModifier)
        {
            c->popup()->hide();
            return;
        }*/

        c->update(text());
        c->popup()->setFont(this->font());
        c->popup()->setCurrentIndex(c->completionModel()->index(0, 0));
    }
}
