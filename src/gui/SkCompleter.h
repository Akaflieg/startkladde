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
#include <optional>
#include "src/model/Plane.h"

class Cache;
class CompletionLineEdit;

class SkCompleter : public QCompleter
{
    Q_OBJECT

public:
    SkCompleter(CompletionLineEdit*, Cache&, QList<QVariant>, std::optional<QString> (*)(QVariant&, QString), QString (*)(QVariant&));
    ~SkCompleter() { }

    void update(QString completionPrefix);

    std::optional<QString> (*itemMatches)(QVariant& v, QString completionPrefix);
    QString (*itemToStringWhenSelected)(QVariant& v);

signals:
    void selected(QVariant);

private:
    Cache& cache;
    QList<QVariant> itemList;
    QStandardItemModel* model;
    CompletionLineEdit* edit;

private slots:
    void activated_(const QModelIndex &index);

};

#endif // SKCOMPLETER_H
