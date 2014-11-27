#include "PersonModel.h"

#include <QApplication>

#include "src/model/Person.h"

PersonModel::PersonModel ()
{
}

PersonModel::~PersonModel ()
{
}

int PersonModel::columnCount () const
{
	return 5;
}

QVariant PersonModel::displayHeaderData (int column) const
{
	switch (column)
	{
		case 0: return qApp->translate ("PersonModel", "Last name");
		case 1: return qApp->translate ("PersonModel", "First name");
		case 2: return qApp->translate ("PersonModel", "Club");
		case 3: return qApp->translate ("PersonModel", "Comments");
		case 4: return qApp->translate ("PersonModel", "ID");
	}

	return QVariant ();
}

QVariant PersonModel::displayData (const Person &person, int column) const
{
	switch (column)
	{
		case 0: return person.lastName;
		case 1: return person.firstName;
		case 2: return person.club;
		case 3: return person.comments;
		case 4: return person.getId ();
	}

	return QVariant ();
}
