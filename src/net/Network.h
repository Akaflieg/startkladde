#ifndef NETWORK_H_
#define NETWORK_H_

class QNetworkAccessManager;

class Network
{
	public:
		Network ();
		virtual ~Network ();

		static QNetworkAccessManager *getNetworkAccessManager ();

	private:
		static QNetworkAccessManager *networkAccessManager;
};

#endif
