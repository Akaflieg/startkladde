/*
 * Network.cpp
 *
 *  Created on: 17.07.2010
 *      Author: Martin Herrmann
 */

#include "Network.h"

#include <QNetworkAccessManager>

QNetworkAccessManager *Network::networkAccessManager;

Network::Network ()
{
}

Network::~Network ()
{
}

QNetworkAccessManager *Network::getNetworkAccessManager ()
{
	if (!Network::networkAccessManager)
		Network::networkAccessManager=new QNetworkAccessManager ();

	return Network::networkAccessManager;
}
