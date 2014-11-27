#include "MonitorDialog.h"

#include <QPushButton>

/*
 * TODO:
 *   - keeping the dialog open for multiple operations
 */

#include "src/concurrent/monitor/SignalOperationMonitor.h"

MonitorDialog::MonitorDialog (SignalOperationMonitor &monitor, QWidget *parent):
	SkDialog<Ui::MonitorDialogClass> (parent), theMonitor (monitor)
{
	ui.setupUi (this);

//	setModal (true);

	QObject::connect (&theMonitor, SIGNAL (ended ())                  , this, SLOT (accept ()          ));
	QObject::connect (&theMonitor, SIGNAL (progressChanged (int, int)), this, SLOT (progress (int, int)));
	QObject::connect (&theMonitor, SIGNAL (statusChanged (QString))   , this, SLOT (status (QString)   ));
}

MonitorDialog::~MonitorDialog()
{

}

/**
 * Displays the dialog, shows the progress and status and displays a button to
 * cancel the operation.
 *
 * This method is properly synchronized. It will return immediately (without
 * showing the dialog) if the task already ended; it will also display the
 * current progress and status on startup if they have been set previously.
 *
 * @param monitor the operation monitor to monitor
 * @param title the title of the window; if no status has been set yet, this is
 *              also used as the initial status
 * @param parent the parent window
 */
void MonitorDialog::monitor (SignalOperationMonitor &monitor, const QString &title, QWidget *parent)
{
	MonitorDialog dialog (monitor, parent);
	dialog.setWindowTitle (title);
	dialog.ui.statusLabel->setText (title);

	// Check after the signals have been connected - the monitor may be updated
	// from a different thread
	if (monitor.getEnded ()) return;

	const QString &status=monitor.getStatus ();
	if (!status.isNull ())
		dialog.ui.statusLabel->setText (status);

	dialog.progress (monitor.getProgress (), monitor.getMaxProgress ());

	dialog.exec ();
}

void MonitorDialog::reject ()
{
	ui.statusLabel->setText (tr ("Canceling..."));
	theMonitor.cancel ();
}

void MonitorDialog::progress (int progress, int maxProgress)
{
	// For progress 0/1, show the busy indicator
	if (maxProgress==1 && progress==0)
		maxProgress=0;

	// It seems like we can get an erroneous "1%" indication if we set the
	// value before the maximum.
	if (maxProgress>=0)
		ui.progressBar->setMaximum (maxProgress);
	ui.progressBar->setValue (progress);
}

void MonitorDialog::status (QString status)
{
	ui.statusLabel->setText (status);
}
