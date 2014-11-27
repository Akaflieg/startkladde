#ifndef QHASH_H_
#define QHASH_H_

#include <QHash>

/**
 * Clears a QHash whose values are pointers and deletes all pointers
 */
template<class K, class V> void clearAndDelete (QHash<K, V *> &hash)
{
	foreach (V *value, hash)
		delete value;

	hash.clear ();
}

/**
 * Removes a value from the hash whose values are pointers and deletes it
 *
 * If the item does not exist in the hash, nothing happens. If there are
 * multiple items for the key in the hash, all of them will be deleted and
 * removed.
 */
template<class K, class V> void removeAndDeleteIfExists (QHash<K, V *> &hash, const K &key)
{
	// Delete all values for that key
	QList<V *> values=hash.values (key);
	foreach (V *value, values)
		delete value;

	// Remove all values for that key
	hash.remove (key);
}

#endif
