/*
 * Downloader.h
 *
 *  Created on: 23.07.2010
 *      Author: martin
 */

#ifndef DOWNLOADER_H_
#define DOWNLOADER_H_

#include <QObject>
#include <QNetworkReply> // Required for QNetworkReply::NetworkError

class QString;
class QUrl;

/**
 * A helper for downloading data from the internet, returning to the event loop
 * after each request
 *
 * The interface of the Downloader consists mainly of the startDownload method
 * and the replyFinished and replyError signals. A numeric state is passed to
 * the download method. The same state is passed in the signals.
 *
 * Each Downloader instance only handles a single download at a time.
 *
 * You need the following slots:
 *    void downloadSucceeded (int state, QNetworkReply *reply);
 *    void downloadFailed    (int state, QNetworkReply *reply, QNetworkReply::NetworkError code);
 */
// TODO should allow access to the reply after the download finished, not just via the signals
// TODO should handle redirects
class Downloader: public QObject
{
		Q_OBJECT

	public:
		Downloader (QObject *parent=NULL);
		virtual ~Downloader ();

		void connectSignals (QObject *receiver);

	public slots:
		void startDownload (int state, const QUrl &url);
		void startDownload (int state, const QString &url);
		void abort ();

	signals:
		/**
		 * Emitted when a download succeeded
		 *
		 * @param state the state passed to startDownload
		 * @param reply the QNetworkReply used for the download. Do not delete.
		 */
		void downloadSucceeded (int state, QNetworkReply *reply);

		/**
		 * Emitted when a download failed
		 *
		 * @param state the state passed to startDownload
		 * @param reply the QNetworkReply used for the download. Do not delete.
		 */
		void downloadFailed    (int state, QNetworkReply *reply, QNetworkReply::NetworkError code);

	private slots:
		void replyFinished ();
		void replyError (QNetworkReply::NetworkError code);

	private:
		QNetworkReply *reply;
		int currentState;
};

#endif
