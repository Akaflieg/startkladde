#include "PlaneCompleter.h"
#include "qdebug.h"
#include "src/db/cache/Cache.h"

PlaneCompleter::PlaneCompleter(QWidget* parent, Cache& cache) :
    QCompleter(parent), cache(cache), planeList(cache.getPlanesSortedByUsage())
{
    model = new QStandardItemModel(this);
    setModel(model);
    connect(this, SIGNAL(activated(QModelIndex)), this, SLOT(activated_(QModelIndex)));
}

PlaneCompleter::~PlaneCompleter()
{

}

void PlaneCompleter::update(QString prefix)
{
    model->clear();
    QStringList tokens = prefix.split(" ", QString::SkipEmptyParts);

    foreach (Plane p, planeList) {
        bool failed = false;
        for (int j = 0; j < tokens.size() && !failed; j++)
        {
            if (!planeMatches(p, tokens[j]))
                failed = true;
        }

        if (!failed) {
            QStandardItem* item = new QStandardItem(planeItemString(p));
            item->setData(QVariant::fromValue(p), Qt::UserRole);
            model->appendRow(item);
        }

    }

    complete();
}

bool PlaneCompleter::planeMatches(Plane &p, QString str)
{
    return p.registration.toLower().contains(str.toLower()) ||
           p.callsign.toLower().contains(str.toLower()) ||
           p.type.toLower().contains(str.toLower()) ||
           p.club.toLower().contains(str.toLower());
}

QString PlaneCompleter::planeItemString(Plane &p)
{
    QString str = p.callsign;
    str = str.leftJustified(3, ' ');
    str += p.registration;
    str = str.leftJustified(12, ' ');
    str += p.type;
    str = str.leftJustified(30, ' ');
    str += p.club;

    return str;
}

void PlaneCompleter::activated_(const QModelIndex &index)
{
    QAbstractItemModel* m = this->completionModel();
    Plane p = m->data(index, Qt::UserRole).value<Plane>();
    emit selected(p);
}
