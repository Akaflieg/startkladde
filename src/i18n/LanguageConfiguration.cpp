#include "LanguageConfiguration.h"

#include <iostream>

#include <QSettings>

#include "src/util/qString.h"
#include "src/i18n/notr.h"

LanguageConfiguration::LanguageConfiguration ():
	type (systemLanguage)
{
}

LanguageConfiguration::LanguageConfiguration (Type type):
	type (type)
{
}

LanguageConfiguration::LanguageConfiguration (const QString &localeName):
	type (manualSelection), localeName (localeName)
{
}

LanguageConfiguration::LanguageConfiguration (const QVariant &value)
{
	if ((QMetaType::Type)value.type ()==QMetaType::QString)
	{
		this->type=manualSelection;
		this->localeName=value.toString ();
	}
	else if ((QMetaType::Type)value.type ()==QMetaType::Int)
	{
		int type=value.toInt ();
		switch (type)
		{
			case systemLanguage: this->type=systemLanguage; break;
			case noTranslation:  this->type=noTranslation; break;
			default:
				std::cerr << notr ("Invalid language choice ") << value.toString () << notr (" in LanguageConfiguration::LanguageConfiguration") << std::endl;
				this->type=systemLanguage;
				break;
		}
	}
}

LanguageConfiguration::~LanguageConfiguration ()
{
}

void LanguageConfiguration::load (QSettings &settings)
{
	QString typeString=settings.value (notr ("type"), notr ("systemLanguage")).toString ();
	if (typeString==notr ("manualSelection"))
	{
		type=manualSelection;
		localeName=settings.value (notr ("localeName")).toString ();
	}
	else if (typeString==notr ("systemLanguage"))
	{
		type=systemLanguage;
		localeName="";
	}
	else if (typeString==notr ("noTranslation"))
	{
		type=noTranslation;
		localeName="";
	}
	else
	{
		std::cerr << notr ("Invalid language configuration type ") << typeString << notr (" in configuration") << std::endl;
		type=systemLanguage;
		localeName="";
	}
}

void LanguageConfiguration::save (QSettings &settings)
{
	switch (type)
	{
		case manualSelection:
			settings.setValue (notr ("type"), notr ("manualSelection"));
			settings.setValue (notr ("localeName"), localeName);
			break;
		case systemLanguage:
			settings.setValue (notr ("type"), notr ("systemLanguage"));
			settings.setValue (notr ("localeName"), "");
			break;
		case noTranslation:
			settings.setValue (notr ("type"), notr ("noTranslation"));
			settings.setValue (notr ("localeName"), "");
			break;
		// No default
	}
}

bool LanguageConfiguration::operator== (const LanguageConfiguration &other) const
{
	switch (type)
	{
		case manualSelection: return other.type==manualSelection && other.localeName==this->localeName; break;
		case systemLanguage : return other.type==systemLanguage;                                        break;
		case noTranslation  : return other.type==noTranslation;                                         break;
		// No default
	}

	return false;
}

bool LanguageConfiguration::operator!= (const LanguageConfiguration &other) const
{
	return !(other==*this);
}

QString LanguageConfiguration::toString () const
{
	switch (type)
	{
		case manualSelection: return qnotr ("manual selection: %1").arg (localeName); break;
		case systemLanguage : return notr ("system language"); break;
		case noTranslation  : return notr ("no translation"); break;
		// No default
	}

	// Should not happen, all types should be handled above
	return "";
}
