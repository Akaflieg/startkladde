#include "AcpiWidget.h"

/**
  * AcpiWidget dummy implementation (for systems without libacpi)
  */

AcpiWidget::AcpiWidget (QWidget* parent): SkLabel (parent),
	timer (NULL)
{
}

bool AcpiWidget::valid ()
{
	return false;
}

void AcpiWidget::slotTimer()
{
}

void AcpiWidget::changeEvent (QEvent *event)
{
	(void)event;
}
