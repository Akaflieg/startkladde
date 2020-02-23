#ifndef DATEDELEGATE_H
#define DATEDELEGATE_H

#include <QDate>
#include <QStyledItemDelegate>
#include "src/util/qDate.h"

class DateDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:

    DateDelegate(QObject* parent = nullptr) : QStyledItemDelegate(parent) { }

    QString displayText(const QVariant &value, const QLocale &locale) const {
        if (value.type() == QVariant::Date) {
            return value.toDate().toString(defaultNumericDateFormat());
        }
        return QStyledItemDelegate::displayText(value, locale);
    }
};

#endif // DATEDELEGATE_H
