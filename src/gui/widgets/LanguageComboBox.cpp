#include "LanguageComboBox.h"

#include <iostream>

#include <QList>
#include <QEvent>

LanguageComboBox::LanguageComboBox (QWidget *parent):
	SkComboBox (parent)
{
}

LanguageComboBox::~LanguageComboBox ()
{
}

void LanguageComboBox::setLanguageItems (const QList<TranslationManager::Language> &languages)
{
	clear ();

	addItem ("", LanguageConfiguration::systemLanguage);
	foreach (const TranslationManager::Language &language, languages)
		addItem (language.languageName, language.localeName);
	addItem ("", LanguageConfiguration::noTranslation);

	setupText ();
}

void LanguageComboBox::setupText ()
{
	setItemTextByItemData (LanguageConfiguration::systemLanguage, tr ("Automatic (use system language)"));
	setItemTextByItemData (LanguageConfiguration::noTranslation, tr ("No translation"));
}

void LanguageComboBox::setCurrentItem (const LanguageConfiguration &languageConfiguration)
{
	switch (languageConfiguration.getType ())
	{
		case LanguageConfiguration::manualSelection:
			setCurrentItemByItemData (languageConfiguration.getLocaleName ());
			break;
		case LanguageConfiguration::noTranslation :
			setCurrentItemByItemData (LanguageConfiguration::noTranslation);
			break;
		case LanguageConfiguration::systemLanguage:
			setCurrentItemByItemData (LanguageConfiguration::systemLanguage);
			break;
		// No default
	}
}

LanguageConfiguration LanguageComboBox::getLanguageConfiguration (int index)
{
	return LanguageConfiguration (itemData (index));
}

LanguageConfiguration LanguageComboBox::getSelectedLanguageConfiguration ()
{
	return getLanguageConfiguration (currentIndex ());
}


// ************
// ** Events **
// ************

void LanguageComboBox::changeEvent (QEvent *event)
{
	if (event->type () == QEvent::LanguageChange)
	{
		setupText ();
	}
	else
		SkComboBox::changeEvent (event);
}
