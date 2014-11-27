/*
 * AccessDeniedException.cpp
 *
 *  Created on: 02.03.2010
 *      Author: Martin Herrmann
 */

#include "AccessDeniedException.h"

AccessDeniedException::AccessDeniedException (const QSqlError &error):
	ConnectionFailedException (error)
{
}

AccessDeniedException *AccessDeniedException::clone () const
{
	return new AccessDeniedException (error);
}

void AccessDeniedException::rethrow () const
{
	throw AccessDeniedException (error);
}
