#ifndef SKPROCESS_H_
#define SKPROCESS_H_

#include <QObject>
#include <QProcess> // Required for QProcess::ExitStatus

/**
 * A wrapper for QProcess that handles termination and line reading
 */
class SkProcess: public QObject
{
		Q_OBJECT

	public:
		SkProcess (QObject *parent=NULL);
		virtual ~SkProcess ();

		virtual void start (const QString &command);
        virtual void start (const QString &command, const QStringList& arguments);
		virtual bool startAndWait (const QString &command);
		virtual void stop ();

		static void splitCommand (QString &commandProper, QString &parameters, const QString &commandWithParameters);

		QProcess *getProcess () { return process; }

	signals:
        void errorLineReceived(const QString &line);
		void lineReceived (const QString &line);
		void exited (int exitCode, QProcess::ExitStatus exitStatus);

	protected slots:
		void outputAvailable ();
        void errorOutputAvailable();
		void processFinished (int exitCode, QProcess::ExitStatus exitStatus);


	private:
		QProcess *process;
};

#endif
