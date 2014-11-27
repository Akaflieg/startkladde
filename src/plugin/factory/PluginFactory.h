#ifndef PLUGINFACTORY_H_
#define PLUGINFACTORY_H_

#include <QString>
#include <QList>
#include <QHash>
#include <QUuid>

#include "src/plugin/info/InfoPlugin.h"       // required for    InfoPlugin::Descriptor
#include "src/plugin/weather/WeatherPlugin.h" // required for WeatherPlugin::Descriptor

/**
 * Registers the given plugin by creating an instance of the corresponding
 * Registration class.
 *
 * Use this macro in the cpp file of the plugin implementation.
 */
#define REGISTER_PLUGIN(pluginKlass, klass) PluginFactory::Registration<pluginKlass> klass ## Descriptor \
	(new pluginKlass::DefaultDescriptor<klass> ());


/**
 * A factory for creating Plugin instances
 *
 * This class is a singleton.
 *
 * Plugins are identified by their ID.
 */
class PluginFactory
{
		// ***********
		// ** Types **
		// ***********

	public:
		/**
		 * RAAI registration of plugins
		 *
		 * This allow registration of plugins by creating a static
		 * instance in a compilation unit.
		 *
		 * This class is intended to be used via the REGISTER_PLUGIN
		 * macro.
		 */
		template<class T> class Registration
		{
			public:
				/**
				 * Registers a plugin
				 *
				 * @param descriptor the descriptor of the plugin to register
				 */
				Registration (typename T::Descriptor *descriptor);
		};

		// ******************
		// ** Construction **
		// ******************

	public: virtual ~PluginFactory ();
	private: PluginFactory ();

		// ***************
		// ** Singleton **
		// ***************

	public: static PluginFactory &getInstance ();
	private: static PluginFactory *instance;


		// *************
		// ** Factory **
		// *************

	public:
		template<class T> QList<const typename T::Descriptor *> getDescriptors ();
		template<class T> T *createPlugin (const QUuid &id) const;

		// Special factory
		WeatherPlugin *createWeatherPlugin (const QUuid &id, const QString &externalCommand);

	private:
		template<class T> void addDescriptor (typename T::Descriptor *descriptor);
		template<class T> const typename T::Descriptor *findPlugin (const QUuid &id) const;

		template<class T> const QHash<QUuid, const typename T::Descriptor *> &getDescriptorHash () const;
		template<class T>       QHash<QUuid, const typename T::Descriptor *> &getDescriptorHash ();

		QHash<QUuid, const InfoPlugin   ::Descriptor *>    infoPluginDescriptors;
		QHash<QUuid, const WeatherPlugin::Descriptor *> weatherPluginDescriptors;
};

#endif
