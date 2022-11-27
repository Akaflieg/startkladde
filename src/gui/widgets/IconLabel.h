#ifndef ICONLABEL_H
#define ICONLABEL_H

#include <QWidget>
#include <QStyle>

#include "src/container/Maybe.h"

#include "ui_IconLabel.h"

class QString;
class QLabel;

/**
 * A widget that displays an icon next to a label
 *
 * The widget is composed of two labels in a horizontal layout. The first (left)
 * label (the icon label) displays an icon the second (right) label (the text
 * label) displays a text.
 */
class IconLabel: public QWidget
{
	Q_OBJECT

	public:
		using QWidget::show;

		IconLabel (QWidget *parent = NULL, Qt::WindowFlags f = Qt::Widget);
		~IconLabel();

		void show (QStyle::StandardPixmap icon, const QString &text);
		void showWarning (const QString &text);
		void showCritical (const QString &text);
		void showInformation (const QString &text);
		void setIcon (QStyle::StandardPixmap icon);
		void setText (const QString &text);

		QLabel *textLabel ();

	signals:
		void linkActivated (const QString &link);
		void linkHovered   (const QString &link);

	private:
		Ui::IconLabelClass ui;
		Maybe<QStyle::StandardPixmap> icon;
};

#endif
