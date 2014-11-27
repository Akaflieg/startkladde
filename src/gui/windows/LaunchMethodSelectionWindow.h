#ifndef LAUNCHMETHODSELECTIONWINDOW_H
#define LAUNCHMETHODSELECTIONWINDOW_H

#include "src/db/dbId.h"
#include "src/gui/SkDialog.h"

#include "ui_LaunchMethodSelectionWindow.h"

class Cache;

class LaunchMethodSelectionWindow: public SkDialog<Ui::LaunchMethodSelectionWindowClass>
{
    Q_OBJECT

	public:
		LaunchMethodSelectionWindow (QWidget *parent=NULL);
		~LaunchMethodSelectionWindow ();

        static bool select (Cache &cache, dbId &value, bool &loadPreselection, QWidget *parent=NULL);

	protected:
		void languageChanged ();
};

#endif
