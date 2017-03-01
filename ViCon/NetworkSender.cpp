#include "NetworkSender.h"
#include <iostream>
#include <string>

NetworkSender::NetworkSender(const char* port, const char* host)
{
	// Set hints (blanking to zero)
	struct addrinfo hints, *result;
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;

	// Get address info of recipient
	auto rv = getaddrinfo(host, port, &hints, &result);
	if (rv != 0) {
		std::cerr << "getaddrinfo err: " << rv;
		throw;
	}

	for (servinfo = result; servinfo != nullptr; servinfo = servinfo->ai_next)
	{
		mySocket = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
		if (mySocket == -1) continue;
		break;
	}

	if (servinfo == nullptr)
	{
		std::cerr << "socket err:";
		throw;
	}
}

void NetworkSender::Send(char* data, int length) const
{
	auto total = 0;        // how many bytes we've sent
	auto bytesleft = length; // how many we have left to send
	
	while (total < length)
	{
		auto numbytes = sendto(mySocket, data+total, bytesleft, 0,
			servinfo->ai_addr, servinfo->ai_addrlen);
		if (numbytes == -1) break;
		total     += numbytes;
		bytesleft -= numbytes;
	}
}

NetworkSender::~NetworkSender()
{
	freeaddrinfo(servinfo);
	closesocket(mySocket);
}
