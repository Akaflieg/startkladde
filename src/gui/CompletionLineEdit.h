#ifndef COMPLETIONLINEEDIT_H
#define COMPLETIONLINEEDIT_H

#include <QLineEdit>

class SkCompleter;

class CompletionLineEdit : public QLineEdit
{
    Q_OBJECT

public:
    CompletionLineEdit(QWidget*);
    ~CompletionLineEdit();

    void setCompleter(SkCompleter*);
    SkCompleter* completer() const;

    bool isItemSelected() const;
    QVariant getSelectedItem() const;
    void setSelectedItem(QVariant item);

protected:
    void keyPressEvent(QKeyEvent *e);

private slots:
    void insertCompletion(QVariant completion);

private:
    QVariant selectedItem;
    bool itemSelected;
    SkCompleter* c;

    void updateSelection(bool dropSelection, QVariant item);
    void adaptColor();

signals:
    void selectionStateChanged();
};

#endif // COMPLETIONLINEEDIT_H
