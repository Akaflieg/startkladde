#ifndef ACPIWIDGET_H_
#define ACPIWIDGET_H_

/**
* ACPI battery status display
* eggert.ehmke@berlin.de
* 10.09.2008
*
* Generalized: Martin Herrmann, 3/2010
* added QProgressBar as indicator: Eggert Ehmke 8/2012
*/

#include <QProgressBar>
#include "src/gui/widgets/SkLabel.h"
//#include "src/gui/windows/NotificationWindow.h"

class QTimer;
class QEvent;

class AcpiWidget: public SkLabel
{
	Q_OBJECT

	public:
		AcpiWidget (QWidget* parent);

		static bool valid ();

	protected:
		virtual void changeEvent (QEvent *event);

	private:
		QTimer* timer;
		QProgressBar* capacity;
		//void batteryAlert ();
		//uint ignoreCounter;
  
	private slots:
		void slotTimer();
};

#endif
