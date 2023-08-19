#include "SkCompleter.h"
#include <QAbstractItemView>
#include "src/db/cache/Cache.h"
#include "src/gui/CompletionLineEdit.h"

SkCompleter::SkCompleter(
    CompletionLineEdit* parent, Cache& cache,
    QList<QVariant> itemList,
    std::optional<QString> (*itemMatches)(QVariant& p, QString str),
    QString (*itemToStringWhenSelected)(QVariant&)) :
    QCompleter(parent), itemMatches(itemMatches), itemToStringWhenSelected(itemToStringWhenSelected), cache(cache), itemList(itemList)
{
    model = new QStandardItemModel(this);
    edit = parent;
    setModel(model);
    connect(this, SIGNAL(activated(QModelIndex)), this, SLOT(activated_(QModelIndex)));
    popup()->installEventFilter(this);
    this->installEventFilter(this);
    edit->installEventFilter(this);
}

void SkCompleter::update(QString completionPrefix)
{
    model->clear();

    foreach (QVariant v, itemList) {
        std::optional<QString> match = itemMatches(v, completionPrefix);
        if (match) {
            QStandardItem* item = new QStandardItem(match.value());
            item->setData(v, Qt::UserRole);
            model->appendRow(item);
        }
    }

    complete();
}

void SkCompleter::activated_(const QModelIndex &index)
{
    QAbstractItemModel* m = this->completionModel();
    QVariant v = m->data(index, Qt::UserRole);
    emit selected(v);
}
