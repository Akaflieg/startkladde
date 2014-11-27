#ifndef WIDGETFADER_H_
#define WIDGETFADER_H_

#include <stdint.h>

#include <QObject>

class QWidget;

class WidgetFader: public QObject
{
		Q_OBJECT

	public:
		virtual ~WidgetFader ();

		static WidgetFader *fadeOutAndClose (QWidget *widget, uint32_t fadeTime, uint32_t waitTime=0);

	public slots:
		void start ();
		void startFade ();

	private:
		WidgetFader (QWidget *widget);

		QWidget *_widget;

		// In milliseconds
		uint32_t _waitTime;
		uint32_t _fadeTime;

		bool _fadeInProgress;
};

#endif
