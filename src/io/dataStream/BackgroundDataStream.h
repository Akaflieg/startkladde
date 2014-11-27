#ifndef BACKGROUNDDATASTREAM_H_
#define BACKGROUNDDATASTREAM_H_

#include <QObject>

#include "src/io/dataStream/DataStream.h"

/**
 * A wrapper that executes a DataStream on a background thread
 *
 * Pass the data stream to the constructor. It will automatically be
 * de-parented and moved to a background thread. If streamOwned is true, the
 * stream will be deleted when the wrapper is deleted.
 *
 * Note that DataStream implementations may not be thread safe. In this case,
 * the data stream may not be accessed directly after the wrapper has been
 * created. In particular, it is necessary to make all calls to configuration
 * methods (such as setting the remote host for a TCP data stream) before
 * creating the wrapper.
 */
class BackgroundDataStream: public QObject
{
	Q_OBJECT

	public:
		BackgroundDataStream (QObject *parent, DataStream *stream, bool streamOwned);
		virtual ~BackgroundDataStream ();

		DataStream *getStream () const;

		DataStream::State getState () const;
		QString getErrorMessage () const;

	public slots:
		// Public interface
		virtual void open ();
		virtual void close ();
		virtual void setOpen (bool o);
		virtual void applyParameters ();

	signals:
		// Public interface
		/** Same as DataStream::stateChanged */
		void stateChanged (DataStream::State state);
		/** Same as DataStream::stateChanged */
		void dataReceived (QByteArray data);
		/** Same as DataStream::connectionBecameAvailable */
		void connectionBecameAvailable ();

		// Signals to the underlying stream
		/** Implementation detail. Please disregard. */
		void open_stream ();
		/** Implementation detail. Please disregard. */
		void close_stream ();
		/** Implementation detail. Please disregard. */
		void setOpen_stream (bool o);
		/** Implementation detail. Please disregard. */
		void applyParameters_stream ();

	private slots:
		// Signals from the underlying stream. Most of the signals are simply
		// re-emitted.
		void stream_stateChanged (DataStream::State state);


	private:
		QThread *_thread;

		DataStream *_stream;
		bool _streamOwned;

		DataStream::State _state;
};

#endif
