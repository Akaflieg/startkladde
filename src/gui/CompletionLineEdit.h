#ifndef COMPLETIONLINEEDIT_H
#define COMPLETIONLINEEDIT_H

#include <QLineEdit>
#include "src/model/Plane.h"

class PlaneCompleter;

class CompletionLineEdit : public QLineEdit
{
    Q_OBJECT

public:
    CompletionLineEdit(QWidget*);
    ~CompletionLineEdit();

    void setCompleter(PlaneCompleter*);
    PlaneCompleter* completer() const;

    Plane getSelectedItem() const;
    void setSelectedItem(Plane item);

protected:
    void keyPressEvent(QKeyEvent *e);

private slots:
    void insertCompletion(Plane completion);

private:
    Plane selectedItem;
    bool itemSelected;
    PlaneCompleter* c;

    void updateSelection(bool s, Plane item);
    void adaptColor();

signals:
    void selectionStateChanged();
};

#endif // COMPLETIONLINEEDIT_H
