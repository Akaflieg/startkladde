#ifndef TABULARTEXTDOCUMENT_H_
#define TABULARTEXTDOCUMENT_H_

#include <QString>

class QAbstractItemModel;
class QTextDocument;

class TabularTextDocument
{
public:
    TabularTextDocument(const QAbstractItemModel &model, const QString title, const QString subtitle);

    QString generateHtml();
    void generate(QTextDocument* doc);

private:
    const QAbstractItemModel &model;
    const QString title;
    const QString subtitle;
};

#endif // TABULARTEXTDOCUMENT_H_
