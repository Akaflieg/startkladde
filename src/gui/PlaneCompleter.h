#ifndef PLANECOMPLETER_H
#define PLANECOMPLETER_H

#include <QCompleter>
#include <QWidget>
#include <QStringListModel>
#include <QStandardItemModel>
#include <QPointer>
#include <QList>
#include "src/model/Plane.h"

class Cache;

class PlaneCompleter : public QCompleter
{
    Q_OBJECT

public:
    PlaneCompleter(QWidget*, Cache&);
    ~PlaneCompleter();

    void update(QString word);

signals:
    void selected(Plane);

private:
    Cache& cache;
    QList<Plane> planeList;
    QStandardItemModel* model;

    static QString planeItemString(Plane& p);
    static bool planeMatches(Plane& p, QString str);

private slots:
    void activated_(const QModelIndex &index);

};

#endif // PLANECOMPLETER_H
