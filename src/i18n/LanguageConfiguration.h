#ifndef LANGUAGECONFIGURATION_H_
#define LANGUAGECONFIGURATION_H_

#include <QString>
#include <QVariant>

class QSettings;

class LanguageConfiguration
{
	public:
		enum Type { manualSelection, systemLanguage, noTranslation };

		LanguageConfiguration ();
		LanguageConfiguration (Type type);
		LanguageConfiguration (const QString &localeName);
		LanguageConfiguration (const QVariant &value);
		virtual ~LanguageConfiguration ();

		virtual void load (QSettings &settings);
		virtual void save (QSettings &settings);

		Type getType () const { return type; }
		QString getLocaleName () const { return localeName; }

		bool operator== (const LanguageConfiguration &other) const;
		bool operator!= (const LanguageConfiguration &other) const;

		QString toString () const;

	private:
		Type type;
		QString localeName; // Only for manualLanguage
};

#endif
