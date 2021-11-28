#include "CompletionLineEdit.h"
#include "SkCompleter.h"
#include <QAbstractItemView>
#include <QKeyEvent>
#include <QPainter>
#include <iostream>
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
    updateSelection(false, item);
}

void CompletionLineEdit::setCompleter(SkCompleter* completer)
{
    completer->setParent(this);

    if (c != NULL) QObject::disconnect(c, 0, this, 0);

    c = completer;

    if (c != NULL) {
        c->setWidget(this);
        connect(completer, SIGNAL(selected(QVariant)), this, SLOT(insertCompletion(QVariant)));
    }
}

SkCompleter* CompletionLineEdit::completer() const
{
    return c;
}

void CompletionLineEdit::insertCompletion(QVariant sel)
{
    updateSelection(false, sel);
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

void CompletionLineEdit::updateSelection(bool dropSelection, QVariant item)
{
    if (dropSelection) {
        setText("");
    } else {
        setText(c->itemToString(item));
    }
    itemSelected = !dropSelection;
    selectedItem = dropSelection ? QVariant() : item;
    adaptColor();
    emit selectionStateChanged();
}

void CompletionLineEdit::keyPressEvent(QKeyEvent *e)
{
    // If an item is already selected and enter is pressed, then ignore the enter
    if (itemSelected && (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter)) {
        e->ignore();
        return;
    }

    // If an item is already selected and a key is pressed, remove the selection
    if (itemSelected &&
         e->key() != Qt::Key_Right &&
         e->key() != Qt::Key_Left &&
         e->key() != Qt::Key_Alt &&
         e->key() != Qt::Key_AltGr) {
        updateSelection(true, QVariant());
    }

    // Pass the keypress
    QLineEdit::keyPressEvent(e);

    // Show completer dropdown
    if (c != NULL &&
        e->key() != Qt::Key_Right &&
        e->key() != Qt::Key_Left &&
        e->key() != Qt::Key_Alt &&
        e->key() != Qt::Key_AltGr)
    {
        c->update(text());
        c->popup()->setFont(this->font());
        c->popup()->setCurrentIndex(c->completionModel()->index(0, 0));
    }
}
