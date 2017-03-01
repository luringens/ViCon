#include "NetworkReceiver.h"
#include <iostream>
#include <string>

NetworkReceiver::NetworkReceiver(const char* port)
{
	// Set hints (blanking to zero)
	struct addrinfo hints, *result;
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;

	// Get address info of self
	auto addResult = getaddrinfo(nullptr, port, &hints, &result);
	if (addResult != 0)
	{
		std::cerr << "getaddrinfo err: " << addResult;
		throw;
	}

	// loop through all the results and bind to the first we can
	for (servinfo = result; servinfo != nullptr; servinfo = servinfo->ai_next)
	{
		mySocket = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
		if (mySocket == -1)
		{
			std::cerr << "socket err";
			throw;
		}

		auto bindresult = bind(mySocket, servinfo->ai_addr, servinfo->ai_addrlen);
		if (bindresult == -1)
		{
			closesocket(mySocket);
			std::cerr << "bind err";
			continue;
		}

		break;
	}

	if (servinfo == nullptr)
	{
		std::cerr << "no servinfor binded";
		throw;
	}

	// Set non blocking mode.
	u_long nonblockingmode = 1;
	ioctlsocket(mySocket, FIONBIO, &nonblockingmode);

	freeaddrinfo(result);
}

char* NetworkReceiver::Receive() const
{
	struct sockaddr_storage their_addr;
	int addr_len = sizeof their_addr;
	auto buf = new char[MAXBUFLENGTH];

	auto numbytes = recvfrom(mySocket, buf, MAXBUFLENGTH - 1, 0, reinterpret_cast<struct sockaddr*>(&their_addr), &addr_len);
	
	if (numbytes == -1) return nullptr;

	buf[numbytes] = '\0';
	return buf;
}

NetworkReceiver::~NetworkReceiver()
{
	freeaddrinfo(servinfo);
	closesocket(mySocket);
}
