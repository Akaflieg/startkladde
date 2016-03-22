/*
 * PluginFactory.cpp
 *
 *  Created on: 04.07.2010
 *      Author: Martin Herrmann
 */

#include "PluginFactory.h"

#include <iostream>

#include <QList>

#include "src/util/qList.h"
#include "src/util/qString.h"
#include "src/i18n/notr.h"

#include "src/plugins/weather/ExternalWeatherPlugin.h"

// ******************
// ** Construction **
// ******************

PluginFactory::PluginFactory ()
{
}

PluginFactory::~PluginFactory ()
{
	QList<const    InfoPlugin::Descriptor *>    infoPluginList=   infoPluginDescriptors.values (); deleteList (   infoPluginList);
	QList<const WeatherPlugin::Descriptor *> weatherPluginList=weatherPluginDescriptors.values (); deleteList (weatherPluginList);
}


// ***************
// ** Singleton **
// ***************

PluginFactory *PluginFactory::instance;

/**
 * Returns the singleton instance of PluginFactory
 *
 * @return the singleton instance of PluginFactory
 */
PluginFactory &PluginFactory::getInstance ()
{
	if (!PluginFactory::instance)
		PluginFactory::instance=new PluginFactory;

	return *PluginFactory::instance;
}


// *************
// ** Factory **
// *************

/**
 * Registers a plugin by adding its descriptor to the list of know
 * plugin descriptors
 *
 * @param descriptor the descriptor for the plugin to add
 */
template<class T> void PluginFactory::addDescriptor (typename T::Descriptor *descriptor)
{
	QHash<QUuid, const typename T::Descriptor *> &descriptorHash=getDescriptorHash<T> ();

	if (descriptor->getId ().isNull ())
	{
		QString message=qnotr ("Error: invalid UUID for plugin %1").arg (descriptor->getName ());
		std::cerr << message << std::endl;
	}
	else if (descriptorHash.contains (descriptor->getId ()))
	{
		QString message=qnotr ("Error: duplicate UUID %1 for plugins %2 and %3").arg (
                descriptor->getId ().toString(),
				descriptor->getName (),
				descriptorHash.value (descriptor->getId ())->getName ());

		std::cerr << message << std::endl;
	}
	else
	{
		descriptorHash.insert (descriptor->getId (), descriptor);
	}
}

/**
 * Returns the list of all known plugin descriptors
 *
 * @return a list of pointers to plugin descriptors. The caller may not
 *         delete any of the pointers.
 */
template<class T> QList<const typename T::Descriptor *> PluginFactory::getDescriptors ()
{
	return getDescriptorHash<T> ().values ();
}

/**
 * Finds the descriptor for a plugin with the given ID
 *
 * If no plugin with the given ID has been registered, NULL is returned. If
 * multiple plugins with the same ID have been registered, an arbitrary one of
 * them is returned.
 *
 * This method is private in order to minimize the chance of accidently
 * deleting a descriptor. #create should be used instead where applicable.
 *
 * @param id the ID of the plugin to find
 * @return a pointer to the descriptor for the plugin, or NULL
 */
template<class T> const typename T::Descriptor *PluginFactory::findPlugin (const QUuid &id) const
{
	return getDescriptorHash<T> ().value (id);
}

/**
 * Creates a plugin with a given ID
 *
 * If no plugin with the given ID has been registered, NULL is returned.
 *
 * The caller takes ownership of the returned Plugin instance.
 *
 * @param id the ID of the plugin to create
 * @return a Plugin instance, or NULL
 * @see #find
 * @see #createWeatherPlugin
 */
template<class T> T *PluginFactory::createPlugin (const QUuid &id) const
{
	const typename T::Descriptor *descriptor=findPlugin<T> (id);
	if (!descriptor) return NULL;

	return descriptor->create ();
}

/**
 * As a special case of createPlugin, creates the plugin with the given ID and
 * sets its command property to the given command if it is an
 * ExternalWeatherPlugin
 *
 * As with createPlugin, the caller takes ownershiop of the returned Plugin
 * instance.
 *
 * @param id the ID of the plugin to create
 * @param externalCommand the command to set if the plugin is an
 *        ExternalWeatherPlugin
 * @return a Plugin instance, or NULL
 * @see #createPlugin
 */
WeatherPlugin *PluginFactory::createWeatherPlugin (const QUuid &id, const QString &externalCommand)
{
	WeatherPlugin *weatherPlugin=createPlugin<WeatherPlugin> (id);

	ExternalWeatherPlugin *externalWeatherPlugin=dynamic_cast<ExternalWeatherPlugin *> (weatherPlugin);
	if (externalWeatherPlugin)
		externalWeatherPlugin->setCommand (externalCommand);

	return weatherPlugin;
}


// *********************************
// ** PluginFactory::Registration **
// *********************************

template<class T> PluginFactory::Registration<T>::Registration (typename T::Descriptor *descriptor)
{
	PluginFactory::getInstance ().addDescriptor<T> (descriptor);
}


// ***************
// ** Templates **
// ***************

#define specializeTemplates(T, t) \
	template<> const QHash<QUuid, const T::Descriptor *> &PluginFactory::getDescriptorHash<T> () const \
	{ return t ## Descriptors; } \
	template<>       QHash<QUuid, const T::Descriptor *> &PluginFactory::getDescriptorHash<T> () \
	{ return t ## Descriptors; } \
	// Empty line

specializeTemplates (   InfoPlugin,    infoPlugin);
specializeTemplates (WeatherPlugin, weatherPlugin);


#define instantiateTemplates(T) \
	template void                          PluginFactory::addDescriptor <T> (T::Descriptor *descriptor); \
	template QList<const T::Descriptor *>  PluginFactory::getDescriptors<T> ();                          \
	template const T::Descriptor          *PluginFactory::findPlugin    <T> (const QUuid &id) const;     \
	template T                            *PluginFactory::createPlugin      (const QUuid &id) const;     \
	template class                         PluginFactory::Registration<T>;                               \
	// Empty line

instantiateTemplates (   InfoPlugin);
instantiateTemplates (WeatherPlugin);
