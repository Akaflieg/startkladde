#include "AcpiWidget.h"

#include <QDebug>
#include <QPalette>
#include <QLayout>
#include <QTimer>
#include <QEvent>

#include "src/i18n/notr.h"
//#include "src/gui/windows/NotificationWindow.h"

extern "C"
{
	#include <libacpi.h>
}

/**
  * AcpiWidget implementation using libacpi
  */


/**
  * the global structure is _the_ acpi structure here
  */
static global_t global_acpi;

AcpiWidget::AcpiWidget (QWidget* parent)
	:SkLabel (parent)
{
//	setAutoFillBackground (true);
	timer = new QTimer (this);
	// 10 second period
	timer->start (10000);
	connect (timer, SIGNAL(timeout()), this, SLOT (slotTimer()));

        //alertBox = NULL;
        //ignoreCounter = 0;
                
        capacity = new QProgressBar (this);
        capacity->setRange (0, 100);
        capacity->setOrientation (Qt::Horizontal);
        capacity->setMaximumHeight (18);

	// show widget immediatly (cannot call this directly from the constructor)
	QTimer::singleShot (0, this, SLOT (slotTimer ()));
}

bool AcpiWidget::valid ()
{
	if(check_acpi_support() == NOT_SUPPORTED){
		qDebug () << notr ("No acpi support for your system?") << endl;
		return false;
	}
	if (init_acpi_batt(&global_acpi) != SUCCESS && init_acpi_acadapt(&global_acpi) != SUCCESS)
		return false;
	return true;
}

void AcpiWidget::slotTimer()
{
	adapter_t *ac = &global_acpi.adapt;

	QString message;
	QPalette palette;

	if(check_acpi_support() == NOT_SUPPORTED){
		qDebug () << notr ("No acpi support for your system?") << endl;
		return;
	}

	/* initialize battery, thermal zones, fans and ac state */
	int battstate = init_acpi_batt(&global_acpi);
	int acstate = init_acpi_acadapt(&global_acpi);

	// TODO improve

	if(acstate == SUCCESS && ac->ac_state == P_BATT) {
		message = tr ("Battery: ", "With traling space");
		palette.setColor(QPalette::Highlight, Qt::red);
		//batteryAlert ();
	}
	else if(acstate == SUCCESS && ac->ac_state == P_AC) {
		message = tr ("External ", "With trailing space");
		palette.setColor (QPalette::Window, Qt::green);
		//ignoreCounter = 0;
		//if (alertBox) {
		        //qDebug () << "close alert box" << endl;
		//        alertBox->close ();
		//        alertBox = NULL;
		//}

	}
	else {
		message = tr ("Unknown ", "With trailing space");
		palette.setColor(QPalette::Highlight, Qt::red);
                // for test on pc only
                //batteryAlert ();
	}
	capacity->setPalette(palette);

	if(battstate == SUCCESS){
		for(int i=0;i<global_acpi.batt_count;i++){
			battery_t *binfo = &batteries[i];
			/* read current battery values */
			read_acpi_batt(i);

			if(binfo->present)
			{
				if (ac->ac_state == P_BATT)
					message += qnotr ("%1% %2:%3\n").arg(binfo->percentage).arg(binfo->remaining_time/60).arg(binfo->remaining_time%60, 2, 10, QChar('0'));
				else
					message += qnotr ("%1%\n").arg(binfo->percentage);
                                capacity->setValue (binfo->percentage);
                                
				QString tooltip = message;
				tooltip += qnotr ("%1: %2mAh\n").arg(binfo->name).arg(binfo->design_cap);
				tooltip += qnotr("%1mAh / %2mAh\n").arg(binfo->last_full_cap).arg(binfo->remaining_cap);
				tooltip += qnotr("%1mV / %2mV\n").arg(binfo->design_voltage).arg(binfo->present_voltage);
/*
				printf("\n%s:\tpresent: %d\n"
						"\tdesign capacity: %d\n"
						"\tlast full capacity: %d\n"
						"\tdesign voltage: %d\n"
						"\tpresent rate: %d\n"
						"\tremaining capacity: %d\n"
						"\tpresent voltage: %d\n"
						"\tcharge state: %d\n"
						"\tbattery state: %d\n"
						"\tpercentage: %d%%\n"
						"\tremaining charge time: %02d:%02d h\n"
						"\tremaining time: %02d:%02d h\n",
						binfo->name, binfo->present, binfo->design_cap,
						binfo->last_full_cap, binfo->design_voltage,
						binfo->present_rate, binfo->remaining_cap,
						binfo->present_voltage, binfo->charge_state,
						binfo->batt_state, binfo->percentage,
						binfo->charge_time / 60, binfo->charge_time % 60,
						binfo->remaining_time / 60, binfo->remaining_time % 60);
*/
				setToolTip (tooltip);
			}
		}
	}
	else {
		message += tr ("Unknown ", "With trailing space");
                capacity->setValue (100);
                setToolTip (message);
        }
}

void AcpiWidget::changeEvent (QEvent *event)
{
	if (event->type () == QEvent::LanguageChange)
	{
		slotTimer ();
	}
	else
		SkLabel::changeEvent (event);
}
