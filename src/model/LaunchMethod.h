#ifndef LAUNCHMETHOD_H_
#define LAUNCHMETHOD_H_

#include <QApplication>
#include <QString>
#include <QStringList>
#include <QMetaType>

#include "src/model/Entity.h"
#include "src/model/objectList/ObjectModel.h"

class QSqlQuery;

class LaunchMethod: public Entity
{
	public:
		// *** Types
		enum Type { typeWinch, typeAirtow, typeSelf, typeOther };

		class DefaultObjectModel: public ObjectModel<LaunchMethod>
		{
			public:
				virtual int columnCount () const;
				virtual QVariant displayHeaderData (int column) const;
				virtual QVariant displayData (const LaunchMethod &object, int column) const;
		};


		// *** Construction
		LaunchMethod ();
		LaunchMethod (dbId id);
		static LaunchMethod parseConfigLine (QString line);


		// *** Data
		QString name; // The name displayed in the GUI
		QString shortName; // The name displayed in tables
		QString logString; // The value for the pilot log
		QString keyboardShortcut; // Should be unique
		Type type;
		QString towplaneRegistration; // For type==typeAirtow
		bool personRequired;


		// *** Property access
		virtual bool isAirtow () const { return type==typeAirtow; }
		virtual bool towplaneKnown () const { return !towplaneRegistration.isEmpty (); }


		// *** Formatting
		virtual QString toString () const;
		virtual QString nameWithShortcut () const;
		static bool nameLessThan (const LaunchMethod &l1, const LaunchMethod &l2) { return l1.name<l2.name; }
		virtual QString getDisplayName () const;


		// *** Type methods
		static QString typeString (Type type);
		static QList<Type> listTypes ();


		// *** ObjectListWindow/ObjectEditorWindow helpers
		static QString objectTypeDescription () { return qApp->translate ("LaunchMethod", "launch method"); }
		static QString objectTypeDescriptionDefinite () { return qApp->translate ("LaunchMethod", "the launch method"); }
		static QString objectTypeDescriptionPlural () { return qApp->translate ("LaunchMethod", "launch methods"); }


		// *** SQL interface
		static QString dbTableName ();
		static QString selectColumnList ();
		static LaunchMethod createFromResult (const Result &result);
        static LaunchMethod createFromDataMap(const QMap<QString,QString> map);
		static QString insertColumnList ();
		static QString insertPlaceholderList ();
		virtual void bindValues (Query &q) const;
		static QList<LaunchMethod> createListFromResult (Result &query);

		// Enum mappers
		static QString typeToDb   (Type    type);
		static Type    typeFromDb (QString type);

	private:
		void initialize ();
};

Q_DECLARE_METATYPE (LaunchMethod);

#endif
