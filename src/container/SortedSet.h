#ifndef SORTEDSET_H_
#define SORTEDSET_H_

#include <QMap>
#include <QList>

/**
 * A generic sorted set, based on QMap
 *
 * Entries are unique.
 *
 * Complexity:
 *   - contains: O(log n)
 *   - insert: O(log n)
 *   - remove: unknown (O(n)?)
 *   - toQList: O(n) (O(1) for repeated calls without changes to the set)
 *
 * This class is not thread safe
 *
 * Files calling methods of a SortedSet (as opposed to only declaring instances
 * of SortedSet) must also include SortedSet_impl.h.
 */
template<typename T> class SortedSet
{
	public:
		SortedSet (): listValid (false) {}
		virtual ~SortedSet () {}

		// *** Data access
		bool clear ();
		bool contains (const T &value) const;
		bool insert (const T &value);
		bool isEmpty () const;
		bool remove (const T &value);
		int size () const;

		// *** QList
		QList<T> toQList () const;
		SortedSet<T> &operator= (const QList<T> &list);

	protected:
		// *** List
		void invalidateList ();
		QList<T> &generateList () const;

	private:
		QMap<T, int> data;

		mutable QList<T> generatedList;
		mutable bool listValid;
};

#endif
