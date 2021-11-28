#ifndef CHOICE_DIALOG_H
#define CHOICE_DIALOG_H

#include "src/gui/SkDialog.h"
#include "ui_ChoiceDialog.h"

#include <QRadioButton>
#include <QList>

/**
 * A dialog that lets the user choose one of several options
 *
 * Example use:
 *     ChoiceDialog dialog (parent);
 *
 *     dialog.setWindowTitle ("Choice");
 *     dialog.setText ("Choose:");
 *     dialog.addOption ("&Foo");    // Add some options
 *     dialog.addOption ("&Bar");
 *     dialog.addOption ("B&az");
 *     dialog.setSelectedOption (0); // Select the first option
 *
 *     int result=dialog.exec ();    // Show the dialog modally
 *     if (result==QDialog::Accepted)
 *         dialog.getSelectedOption ();  // Get the selected option
 *
 * Alternatively, you can use the static choose method.
 */
class ChoiceDialog: public SkDialog<Ui::ChoiceDialogClass>
{
	public:
		ChoiceDialog (QWidget *parent = 0, Qt::WindowFlags f={});
		~ChoiceDialog();

		static int choose (const QString &title, const QString &text,
				const QStringList &options, int defaultIndex=0,
                           QWidget *parent=NULL, Qt::WindowFlags flags={});

		void setText (const QString &text);
		void addOption (const QString &text);

		void setSelectedOption (int option);
		int getSelectedOption ();

	protected:
		QRadioButton *addRadioButton ();

		QList<QRadioButton *> radioButtons;
};

#endif
