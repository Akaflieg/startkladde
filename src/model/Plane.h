#ifndef PLANE_H_
#define PLANE_H_

#include <QApplication>
#include <QString>
#include <QList>
#include <QMetaType>

#include "src/model/objectList/ObjectModel.h"
#include "src/i18n/notr.h"

/*
 * Improvements:
 *   - addObject attribute "is towplane"
 */

#include "src/model/Entity.h"
/*
 * Aircraft: everything
 * Airplanes: motorized fixed-wing
 * Motorized aircraft: airplanes and helicopters
 * So this class should be called Aircraft
 */
class Plane: public Entity
{
	public:
		// *** Types
		enum Category { categoryNone, categoryAirplane, categoryGlider, categoryMotorglider, categoryUltralight, categoryOther };

		class DefaultObjectModel: public ObjectModel<Plane>
		{
			public:
				virtual int columnCount () const;
				virtual QVariant displayHeaderData (int column) const;
				virtual QVariant displayData (const Plane &object, int column) const;
		};


		// *** Construction
		Plane ();
		Plane (dbId id);


		// *** Data
		QString registration;
		QString club;
		int numSeats;
		QString type;
		Category category;
		QString callsign;
		QString flarmId;


		// *** Property access
		virtual bool selfLaunchOnly () const;
	    static QString defaultRegistrationPrefix () { return qApp->translate ("Plane", "N", "default prefix"); }
	    virtual QString fullRegistration (const QString &fullTemplate=notr ("%1 (%2)")) const;
	    virtual QString registrationWithType () const;


		// *** Formatting
		virtual QString toString () const;
	    static bool clubAwareLessThan (const Plane &p1, const Plane &p2);
		virtual QString getDisplayName () const;


	    // *** Category methods
		static QList<Category> listCategories (bool include_invalid);
		static QString categoryText (Plane::Category category);
		static Plane::Category categoryFromRegistration (QString registration);
		static int categoryMaxSeats (Plane::Category category);


		// *** ObjectListWindow/ObjectEditorWindow helpers
		static QString objectTypeDescription () { return qApp->translate ("Plane", "plane"); }
		static QString objectTypeDescriptionDefinite () { return qApp->translate ("Plane", "the plane"); }
		static QString objectTypeDescriptionPlural () { return qApp->translate ("Plane", "planes"); }


		// SQL interface
		static QString dbTableName ();
		static QString selectColumnList ();
		static Plane createFromResult (const Result &result);
        static Plane createFromDataMap(const QMap<QString,QString> map);
		static QString insertColumnList ();
		static QString insertPlaceholderList ();
		virtual void bindValues (Query &q) const;
		static QList<Plane> createListFromResult (Result &query);
		// Enum mappers
		static QString  categoryToDb   (Category category);
		static Category categoryFromDb (QString  category);

	private:
		void initialize ();
};

Q_DECLARE_METATYPE (Plane);

#endif

