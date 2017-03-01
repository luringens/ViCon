#pragma once
#include <ws2tcpip.h>

class NetworkSender
{
public:
	NetworkSender(const char* port, const char* host);

	void Send(char* data, int length) const;

	~NetworkSender();

private:
	SOCKET mySocket;
	struct addrinfo *servinfo;
};
