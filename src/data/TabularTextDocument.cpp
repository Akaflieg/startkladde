#include "TabularTextDocument.h"

#include "src/gui/views/DateDelegate.h"
#include <QAbstractItemModel>
#include <QTextDocument>
#include <QTextCursor>
#include <QTextTable>
#include <QTextTableFormat>

TabularTextDocument::TabularTextDocument(const QAbstractItemModel &model, const QString title, const QString subtitle)
    : model(model), title(title), subtitle(subtitle)
{

}

QString TabularTextDocument::generateHtml() {
    int rows = model.rowCount();
    int columns = model.columnCount();

    DateDelegate itemDelegate;

    QString titleStyle ("font-size: 12px; font-weight: bold;");
    QString subtitleStyle ("font-size: 9px;");
    QString tableStyle ("font-size: 7px; width: 100%; border-collapse: collapse;");
    QString headerCellStyle = ("font-weight:bold; border-bottom: 2px solid black; padding: 1px 3px;");
    QString bodyCellStyle = ("border-bottom: 1px solid black; padding: 1px 3px;");

    QString result;

    result.append(QString("<div style='%1'>%2</div>").arg(titleStyle).arg(title));
    result.append(QString("<div style='%1'>%2</div><br/>").arg(subtitleStyle).arg(subtitle));
    result.append(QString("<table width='100%' style='%1'>").arg(tableStyle));

    result.append("<tr>");
    for (int column=0; column<columns; ++column)
    {
        QString alignment = "text-align: left;";
        if (rows > 1 && model.data(model.index(0, column)).type() == QVariant::Int) {
            alignment = "text-align: right;";
        }

        QVariant value=model.headerData (column, Qt::Horizontal);
        result.append(QString("<td style='%1%2'>").arg(headerCellStyle).arg(alignment))
              .append(itemDelegate.displayText(value, QLocale()))
              .append("</td>");

    }
    result.append("</tr>");

    for (int row=0; row<rows; ++row)
    {
        result.append("<tr>");
        for (int column=0; column<columns; ++column)
        {
            QString alignment = "text-align: left;";
            if (model.data(model.index(row, column)).type() == QVariant::Int) {
                alignment = "text-align: right;";
            }

            QVariant value=model.data (model.index (row, column));
            result.append(QString("<td style='%1%2'>").arg(bodyCellStyle).arg(alignment))
                  .append(itemDelegate.displayText(value, QLocale()))
                  .append("</td>");
        }
        result.append("</tr>");
    }

    result.append("</table>");
    return result;
}

void TabularTextDocument::generate(QTextDocument* doc) {

    doc->setHtml(generateHtml());

    /*QTextCursor cursor (doc);

    QTextTable* tab = cursor.insertTable(rows + 1, columns);
    QTextTableFormat tabFormat;
    tabFormat.setBorderStyle(QTextFrameFormat::BorderStyle_Solid);
    tabFormat.setCellPadding(2);
    tabFormat.setCellSpacing(0);
    tabFormat.setBorderCollapse(true);
    tabFormat.setWidth(QTextLength(QTextLength::PercentageLength, 100));
    QBrush b = tabFormat.borderBrush();
    b.setColor(QColor(0, 0, 0));
    tabFormat.setBorderBrush(b);
    tab->setFormat(tabFormat);

    QFont f = doc->defaultFont();
    f.setPointSizeF(4.5);
    doc->setDefaultFont(f);

    DateDelegate itemDelegate;

    for (int column=0; column<columns; ++column)
    {
        QVariant value=model.headerData (column, Qt::Horizontal);
        tab->cellAt(0, column).firstCursorPosition().insertText(itemDelegate.displayText(value, QLocale()));
    }

    for (int row=0; row<rows; ++row)
    {
        for (int column=0; column<columns; ++column)
        {
            QVariant value=model.data (model.index (row, column));
            tab->cellAt(row + 1, column).firstCursorPosition().insertText(itemDelegate.displayText(value, QLocale()));
        }
    }*/

}
