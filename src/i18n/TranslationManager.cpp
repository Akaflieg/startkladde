#include "TranslationManager.h"

#include <iostream>

#include <QApplication>
#include <QLibraryInfo>
#include <QLocale>
#include <QRegExp>
#include <QStringList>
#include <QDir>
#include <QTranslator>

#include "src/i18n/notr.h"
#include "src/util/qString.h"

// TODO: when switching from "automatic" to, e. g., "German" in a German
// environment, the translation is still reloaded, because the old locale is
// "de_DE" and the new one is "de". This could, for example, be solved by
// storing the name of the translation file instead of the locale name (or could
// it?).


// ****************************
// ** Construction/singleton **
// ****************************

TranslationManager *TranslationManager::theInstance;

TranslationManager::TranslationManager ()
{
	// Load Qt translations from the Qt translations directory.
	translationPath << QLibraryInfo::location (QLibraryInfo::TranslationsPath);

	// Load application translations from one of two paths:
	//   - the translations directory in the application directory
	//   - the associated "share/startkladde/translations" directory for the
	//     application directory. This also covers
	//     /usr/share/startkladde/translations for the default installation.
	// Note that this is unsatisfactory: there should be only one application
	// translations path, configured by the build system.
	QString appDir=QCoreApplication::applicationDirPath ();
	translationPath << appDir+notr ("/translations");
	translationPath << appDir+notr ("/../share/startkladde/translations");
}

TranslationManager::~TranslationManager ()
{
}

TranslationManager &TranslationManager::instance ()
{
	if (!theInstance)
		theInstance=new TranslationManager ();

	return *theInstance;
}


// ***********
// ** Setup **
// ***********

void TranslationManager::install (QApplication *application)
{
	application->installTranslator (&appTranslator);
	application->installTranslator (&qtTranslator);
}


// ***********************
// ** Translation files **
// ***********************

/**
 * Determines the locale name from a given filename
 *
 * @param filename the filename without path, e. g. startkladde_de.qm
 * @return the locale name, or an empty string if the filename does not
 *         represent a known translation file pattern
 */
QString TranslationManager::localeNameFromFilename (const QString &filename)
{
	QRegExp regexp (notr ("startkladde_(.*)\\.qm"));
	if (regexp.exactMatch (filename))
		return regexp.cap (1);
	else
		return QString ();
}

QString TranslationManager::filenameForLocaleName (const QString &localeName)
{
	return qnotr ("startkladde_%1.ts").arg (localeName);
}

QStringList TranslationManager::listTranslationFiles ()
{
	QStringList filenames;

	QStringList nameFilters;
	nameFilters << notr ("startkladde_*.qm");

	foreach (const QDir &dir, translationPath)
		filenames.append (dir.entryList (nameFilters, QDir::Files));

	return filenames;
}

bool TranslationManager::loadTranslation (QTranslator &translator, const QString &prefix, const QString &localeName)
{
	QString filename=prefix+notr ("_")+localeName+notr (".qm");

	// Look in all directories of the translation path until the translation
	// has been found
	foreach (const QDir &dir, translationPath)
	{
		// Output a message. The actual filename may be different from the
		// filename specified because the translator tries to shorten the
		// filename if the file does not exist, e. g. from startkladde_de_DE.qm
		// to startkladde_de.qm.
//		std::cout << qnotr ("Trying to load %1 from %2...").arg (filename, dir.path ());
		if (translator.load (filename, dir.path ()))
		{
			// Loading succeeded. Return success.
//			std::cout << notr ("OK") << std::endl;
			return true;
		}
		else
		{
			// Loading failed. Continue trying.
//			std::cout << notr ("failed") << std::endl;
		}
	}

	// Loading did not succeed. Return failure.
	return false;
}


// **********************
// ** Language loading **
// **********************

/**
 * Unloads the translation, if any is loaded
 *
 * If no translation is loaded, nothing happens, unless force is true.
 *
 * @return true, indicating success (unloading cannot fail)
 */
bool TranslationManager::unload (bool force)
{
	if (currentLocale!="" || force)
	{
		// Output a message
//		std::cout << notr ("Unloading translation") << std::endl;

		appTranslator.load ("");
		qtTranslator .load ("");

		currentLocale="";
	}

	return true;
}

/**
 * Loads the translation for the given locale name
 *
 * If the translation is already loaded, nothing happens, unless force is true.
 *
 * @return true if the application translation could be loaded, regardless of
 *         whether the Qt translation could be loaded; false otherwise
 */
bool TranslationManager::loadForLocale (const QString &localeName, bool force)
{
	// Nothing to do if the locale is already loaded, unless force is true
	if (currentLocale==localeName && !force)
		return true;

	// Output a message
//	std::cout << notr ("Loading translation for ") << localeName << std::endl;

	// Load the application translation
	if (loadTranslation (appTranslator, notr ("startkladde"), localeName))
	{
		// Loading the application translation succeeded, load the Qt
		// translation (ignore failure)
		loadTranslation (qtTranslator, notr ("qt"), localeName);

		// Store the loaded locale name
		currentLocale=localeName;

		// Return success
		return true;
	}
	else
	{
		// Return failure
		return false;
	}
}

/**
 * Loads the translation for the current locale
 *
 * If the translation is already loaded, nothing happens, unless force is true.
 *
 * @return true on success (see loadForLocale), false on failure
 */
bool TranslationManager::loadForCurrentLocale (bool force)
{
	return loadForLocale (QLocale::system ().name (), force);
}

/**
 * Loads the translation specified by the given configuration
 *
 * If the translation is already loaded, nothing nothing happens, unless the
 * force parameter is true.
 *
 * @param configuration the language configuration to load
 * @param force load the language even if it is already loaded
 * @return true on success (see loadForLocale), false on failure
 */
// FIXME what happens if it is invalid (e. g. invalid value in configuration)?
bool TranslationManager::load (const LanguageConfiguration &configuration, bool force)
{
	switch (configuration.getType ())
	{
		case LanguageConfiguration::manualSelection:
			return loadForLocale (configuration.getLocaleName (), force);
			break;
		case LanguageConfiguration::noTranslation :
			return unload (force);
			break;
		case LanguageConfiguration::systemLanguage:
			return loadForCurrentLocale (force);
			break;
		// No default
	}

	// Should never be reached, all cases must be handled above
	return false;
}

void TranslationManager::toggleLanguage ()
{
	if (appTranslator.isEmpty ())
		loadForCurrentLocale ();
	else
		unload ();
}


// **********
// ** Misc **
// **********

/**
 * Determines the name of a language, in that language
 *
 * This is done by loading the given translation locale (without using it for the
 * application) and reading the language name from the translator.
 */
QString TranslationManager::determineLanguageNameForLocale (const QString &localeName)
{
	QTranslator translator;
	loadTranslation (translator, notr ("startkladde"), localeName);

	struct { const char *source; const char *comment; } languageString=
		QT_TRANSLATE_NOOP3("Translation", " ",
			"Replace with the name of the translation language, in that language");

	return translator.translate (notr ("Translation"), languageString.source);
}



QList<TranslationManager::Language> TranslationManager::listLanguages ()
{
	QList<Language> languages;
	QList<Language> languagesWithoutName;

	foreach (const QString &filename, listTranslationFiles ())
	{
		QString localeName=localeNameFromFilename (filename);
		if (!localeName.isEmpty ())
		{
			QString languageName=determineLanguageNameForLocale (localeName);

			// If the language name is empty, use the locale name instead. This
			// may happen if there is a language which does not translate its own
			// name.
			// TODO this functionality should be in the caller, but callers
			// would forget to handle it. There should be a parameter for what
			// to do in this case: ignore it (return empty name, the caller
			// handles it), ignore the language (don't include it in the list),
			// use locale name.
			if (languageName.trimmed ().isEmpty ())
				languagesWithoutName.append (Language (localeName, localeName));
			else
				languages.append (Language (localeName, languageName));
		}
	}

//	std::cout << notr ("Found languages:") << std::endl;
//	foreach (const Language &language, languages)
//		std::cout << qnotr ("  * %1 (%2)").arg (language.languageName, language.localeName) << std::endl;
//	foreach (const Language &language, languagesWithoutName)
//		std::cout << qnotr ("  * %1").arg (language.localeName) << std::endl;

	return languages+languagesWithoutName;
}
