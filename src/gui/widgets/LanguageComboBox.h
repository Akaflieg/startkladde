#ifndef LANGUAGECOMBOBOX_H_
#define LANGUAGECOMBOBOX_H_

#include "src/gui/widgets/SkComboBox.h"
#include "src/i18n/TranslationManager.h"
#include "src/i18n/LanguageConfiguration.h"

template<class T> class QList;

class LanguageComboBox: public SkComboBox
{
		Q_OBJECT

	public:
		LanguageComboBox (QWidget *parent);
		virtual ~LanguageComboBox ();

		void setLanguageItems (const QList<TranslationManager::Language> &languages);
		void setupText ();

		void setCurrentItem (const LanguageConfiguration &languageConfiguration);
		LanguageConfiguration getLanguageConfiguration (int index);
		LanguageConfiguration getSelectedLanguageConfiguration ();

	protected:
		virtual void changeEvent (QEvent *event);

};

#endif
