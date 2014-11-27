#ifndef SERIALPORTLIST_H_
#define SERIALPORTLIST_H_

#include <QObject>
#include <QSet>
#include <QString>

class QMutex;

/**
 * A wrapper around the serial library's port enumeration facility, but thread
 * safe and with added functionality.
 *
 * When the port list changes, the portsChanged signal is emitted with the new
 * port list. Additionally, the portAdded and portRemoved signals are emitted
 * for every port added or removed, respectively.
 *
 * The first access to this class MUST be made from the GUI thread. The
 * createInstance method can be used if no functionality is required at this
 * point.
 */
class SerialPortList: public QObject
{
		Q_OBJECT;

	public:
		// Construction
		virtual ~SerialPortList ();

		// Singleton
		static SerialPortList *instance ();
		static void createInstance ();

		// Port inquiry
		QSet<QString> availablePorts ();
		bool isPortAvailable (const QString &port, Qt::CaseSensitivity caseSensitivity);
		QString getDescription (const QString &deviceName);

	signals:
		/** Emitted when the available port list changes */
		void portsChanged (QSet<QString> ports);
		/** Emitted when a port is added */
		void portAdded (QString port);
		/** Emitted when a port is removed */
		void portRemoved (QString port);

	private:
		// Construction
		SerialPortList ();

		// Singleton
		static SerialPortList *_instance;
		static QMutex *_instanceMutex;

		// Ports
		QSet<QString> _availablePorts;

		QMutex *_mutex;

	private slots:
		void availablePortsChanged (const QStringList &ports);
};

#endif
