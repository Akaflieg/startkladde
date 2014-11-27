/*
 * qString.h
 *
 *  Created on: 21.02.2010
 *      Author: Martin Herrmann
 */

#ifndef QSTRING_H_
#define QSTRING_H_

#include <iostream>
#include <string>

#include <QString>

std::ostream &operator<< (std::ostream &s, const QString &c);
QString std2q (const std::string &string);
std::string q2std (const QString &string);

#endif
