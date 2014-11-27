#ifndef PERSONMODEL_H_
#define PERSONMODEL_H_

#include <QVariant>

#include "src/model/objectList/ObjectModel.h"

class Person;

class PersonModel: public ObjectModel<Person>
{
	public:
		PersonModel ();
		virtual ~PersonModel ();

		virtual int columnCount () const;

	protected:
		virtual QVariant displayHeaderData (int column) const;
		virtual QVariant displayData (const Person &person, int column) const;
};

#endif
