#ifndef STATISTICSWINDOW_H
#define STATISTICSWINDOW_H

#include "ui_StatisticsWindow.h"

#include "src/gui/SkDialog.h"

class QAbstractTableModel;

class StatisticsWindow: public SkDialog<Ui::StatisticsWindowClass>
{
		Q_OBJECT

	public:
		StatisticsWindow (QAbstractTableModel *model, bool modelOwned, const char *ntr_title, QWidget *parent=0);
		~StatisticsWindow ();

		static void display (QAbstractTableModel *model, bool modelOwned, const char *ntr_title, QWidget *parent=0);

	protected:
		void languageChanged ();
		void setupText ();

	private:
		QAbstractTableModel *model;
		bool modelOwned;
		const char *ntr_title;
};

#endif
