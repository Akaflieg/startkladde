#include "CompletionLineEdit.h"
#include "PlaneCompleter.h"
#include <QAbstractItemView>
#include <QKeyEvent>
#include <QPainter>
#include "qdebug.h"

CompletionLineEdit::CompletionLineEdit(QWidget* parent) :
    QLineEdit(parent)
{
    c = NULL;
    itemSelected = false;
}

CompletionLineEdit::~CompletionLineEdit()
{

}

Plane CompletionLineEdit::getSelectedItem() const
{
    return selectedItem;
}

void CompletionLineEdit::setSelectedItem(Plane item)
{
    updateSelection(true, item);
}

void CompletionLineEdit::setCompleter(PlaneCompleter* completer)
{
    if (c != NULL) QObject::disconnect(c, 0, this, 0);

    c = completer;

    if (c == NULL) return;

    c->setWidget(this);
    connect(completer, SIGNAL(selected(Plane)), this, SLOT(insertCompletion(Plane)));
}

PlaneCompleter* CompletionLineEdit::completer() const
{
    return c;
}

void CompletionLineEdit::insertCompletion(Plane sel)
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

void CompletionLineEdit::updateSelection(bool s, Plane item)
{
    if (itemSelected && !s) {
        setText("");
    }
    if ((!itemSelected && s) || (s && (item.getId() != selectedItem.getId()))) {
        setText(item.toNiceString());
    }
    itemSelected = s;
    selectedItem = item;
    adaptColor();

}

void CompletionLineEdit::keyPressEvent(QKeyEvent *e)
{
    if (itemSelected &&
         e->key() != Qt::Key_Right &&
         e->key() != Qt::Key_Left) {
        updateSelection(false, Plane());
    }

    if (c && c->popup()->isVisible())
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
    }

    bool isShortcut = (e->modifiers() & Qt::ControlModifier) && e->key() == Qt::Key_E;
    if (!isShortcut)
        QLineEdit::keyPressEvent(e); // Don't send the shortcut (CTRL-E) to the text edit.

    if (!c)
        return;

    bool ctrlOrShift = e->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier);
    if (!isShortcut && !ctrlOrShift && e->modifiers() != Qt::NoModifier)
    {
        c->popup()->hide();
        return;
    }

    c->update(text());
    c->popup()->setFont(this->font());
    c->popup()->setCurrentIndex(c->completionModel()->index(0, 0));
}
