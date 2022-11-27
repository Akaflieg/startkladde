#ifndef SKCOMPLETER_H
#define SKCOMPLETER_H

#include <QCompleter>
#include <QWidget>
#include <QStringListModel>
#include <QStandardItemModel>
#include <QPointer>
#include <QList>
#include <QKeyEvent>
#include <QEvent>
#include <QDebug>
#include "src/model/Plane.h"

class Cache;
class CompletionLineEdit;

class SkCompleter : public QCompleter
{
    Q_OBJECT

public:
    SkCompleter(CompletionLineEdit*, Cache&, QList<QVariant>, bool (*)(QVariant&, QString), QString (*)(QVariant&));
    ~SkCompleter();

    void update(QString word);

    bool (*itemMatches)(QVariant& p, QString str);
    QString (*itemToString)(QVariant& p);

signals:
    void selected(QVariant);

protected:
    bool event(QEvent *e);
    bool eventFilter(QObject *obj, QEvent *event);

private:
    Cache& cache;
    QList<QVariant> itemList;
    QStandardItemModel* model;
    CompletionLineEdit* edit;

private slots:
    void activated_(const QModelIndex &index);

};

#endif // SKCOMPLETER_H
