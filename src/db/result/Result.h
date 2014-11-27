/*
 * Result.h
 *
 *  Created on: 25.02.2010
 *      Author: Martin Herrmann
 */

#ifndef RESULT_H_
#define RESULT_H_

#include <QSqlRecord>

class Result
{
	public:
		virtual ~Result () {}

		virtual int at () const=0;
		virtual bool first ()=0;
		virtual bool isNull (int field) const=0;
		virtual bool last ()=0;
		virtual QVariant lastInsertId () const=0;
		virtual QString lastQuery () const=0;
		virtual bool next ()=0;
		virtual int numRowsAffected () const=0;
		virtual bool previous ()=0;
		virtual QSqlRecord record () const=0;
		virtual bool seek (int index, bool relative=false)=0;
		virtual int size () const=0;
		virtual QVariant value (int index) const=0;

		virtual QString type () const=0;
};

#endif
