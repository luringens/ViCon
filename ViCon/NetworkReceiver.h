#pragma once
#include <ws2tcpip.h>

class NetworkReceiver
{
public:
	explicit NetworkReceiver(const char* iport);

	char* Receive() const;

	~NetworkReceiver();

private:
	SOCKET mySocket;
	struct addrinfo *servinfo;
	static const int MAXBUFLENGTH = 100;
};
