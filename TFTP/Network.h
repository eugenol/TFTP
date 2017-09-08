#pragma once
#include <winsock.h>
#include "Packets.h"

//==============================================================================

class KWSASession
{
public:
	KWSASession();
	~KWSASession();
private:
	WSAData m_winsockData;
};

//==============================================================================

class KUDPScocket
{
public:
	KUDPScocket();
	~KUDPScocket();

	void SendTo(const std::string& address, unsigned short port, KPacket &packet);
	void SendTo(sockaddr_in& address, KPacket &packet);
	sockaddr_in RecvFrom( KDATAPacketReceiver& dataReceiver);

private:
	SOCKET m_socketDescriptor;
};

//==============================================================================