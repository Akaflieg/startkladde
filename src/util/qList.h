#ifndef QLIST_H_
#define QLIST_H_

#include <QList>

template<class T> void deleteList (QList<T *> &list)
{
	while (!list.isEmpty ())
		delete list.takeLast ();
}

template<class T> void deleteList (QList<const 	T *> &list)
{
	while (!list.isEmpty ())
		delete list.takeLast ();
}

template<class T> void appendUnlessNull (QList<T *> &list, T *t)
{
	if (t)
		list.append (t);
}

template<class T, class F> QList<T> convertType (const QList<F> &list)
{
	QList<T> result;
	foreach (const F &object, list)
		result.append (object);

	return result;
}

#endif
