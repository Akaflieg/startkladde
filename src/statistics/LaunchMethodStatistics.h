/*
 * LaunchMethodStatistics.h
 *
 *  Created on: Aug 18, 2009
 *      Author: Martin Herrmann
 */

#ifndef LAUNCHMETHODSTATISTICS_H_
#define LAUNCHMETHODSTATISTICS_H_

#include <QAbstractTableModel>
#include <QString>
#include <QList>

class Cache;
class Flight;

// TODO: consider basing this (and other statistics classes) on ObjectListModel
class LaunchMethodStatistics: public QAbstractTableModel
{
		Q_OBJECT

	public:
		class Entry
		{
			public:
				Entry ();
				virtual ~Entry ();

				QString name;
				int num;
		};

		LaunchMethodStatistics (QObject *parent=NULL);
		virtual ~LaunchMethodStatistics ();

		static LaunchMethodStatistics *createNew (const QList<Flight> &flights, Cache &cache);

		// QAbstractTableModel methods
		virtual int rowCount (const QModelIndex &index) const;
		virtual int columnCount (const QModelIndex &index) const;
		virtual QVariant data (const QModelIndex &index, int role = Qt::DisplayRole) const;
		virtual QVariant headerData (int section, Qt::Orientation orientation, int role=Qt::DisplayRole) const;

	private:
		QList<Entry> entries;
};

#endif
