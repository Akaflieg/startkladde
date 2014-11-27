#ifndef FLARMNETHANDLER_H
#define FLARMNETHANDLER_H

#include <QtCore/QObject>
#include <QtNetwork/QNetworkReply>

#include "src/net/Downloader.h"

class QByteArray;
class QFile;
class QWidget;

class DbManager;
class FlarmNetRecord;
class MonitorDialog;
class OperationMonitorInterface;
class SignalOperationMonitor;

class FlarmNetHandler: public QObject
{
	Q_OBJECT

	public:
		// *** Construction
		FlarmNetHandler (DbManager &dbManager, QWidget *parent);
		~FlarmNetHandler ();

		// *** Importing
		void interactiveImportFromFile ();
		void interactiveImportFromWeb ();

		// *** Converting
		void interactiveDecodeFile ();
		void interactiveEncodeFile ();

	protected:
		void interactiveImport (const QByteArray &data);
		void interactiveImport (QList<FlarmNetRecord> &records);

		void decode (QFile &inputFile, QFile &outputFile);
		void encode (QFile &inputFile, QFile &outputFile);

	protected slots:
		void downloadSucceeded (int state, QNetworkReply *reply);
		void downloadFailed    (int state, QNetworkReply *reply, QNetworkReply::NetworkError code);
		void abort ();

	private:
		void finishProgress ();

		QWidget *parent;
		DbManager &dbManager;
		Downloader downloader;

		SignalOperationMonitor *monitor;
		OperationMonitorInterface *operationMonitorInterface;

		bool downloadSuccess;
		bool downloadAborted;
		bool downloadFailure;
		QByteArray downloadData;
		QString downloadErrorString;
};

#endif
