/*
 * DefaultResult.cpp
 *
 *  Created on: 25.02.2010
 *      Author: Martin Herrmann
 */

#include "DefaultResult.h"

#include <QVariant>

DefaultResult::DefaultResult (QSqlQuery& query): query (query) {}
DefaultResult::~DefaultResult () {}

int        DefaultResult::at              ()          const          { return query.at              ();                }
bool       DefaultResult::first           ()                         { return query.first           ();                }
bool       DefaultResult::isNull          (int field) const          { return query.isNull          (field);           }
bool       DefaultResult::last            ()                         { return query.last            ();                }
QVariant   DefaultResult::lastInsertId    ()          const          { return query.lastInsertId    ();                }
QString    DefaultResult::lastQuery       ()          const          { return query.lastQuery       ();                }
bool       DefaultResult::next            ()                         { return query.next            ();                }
int        DefaultResult::numRowsAffected ()          const          { return query.numRowsAffected ();                }
bool       DefaultResult::previous        ()                         { return query.previous        ();                }
QSqlRecord DefaultResult::record          ()          const          { return query.record          ();                }
bool       DefaultResult::seek            (int index, bool relative) { return query.seek            (index, relative); }
int        DefaultResult::size            ()          const          { return query.size            ();                }
QVariant   DefaultResult::value           (int index) const          { return query.value           (index);           }

//QSqlQuery &DefaultResult::getQuery () { return query; }
