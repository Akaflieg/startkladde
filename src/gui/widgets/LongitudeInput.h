#ifndef LONGITUDEINPUT_H
#define LONGITUDEINPUT_H

#include <QWidget>
#include "ui_LongitudeInput.h"

#include "src/Longitude.h"

class LongitudeInput: public QWidget
{
		Q_OBJECT

	public:
		LongitudeInput (QWidget *parent=NULL);
		virtual ~LongitudeInput ();

		void setLongitude (const Longitude &longitude);
		Longitude getLongitude () const;

	protected:
		virtual void changeEvent (QEvent *event);

	private:
		Ui::LongitudeInputClass ui;
};

#endif
