#include "Network.h"
#include <system_error>

//==============================================================================

KWSASession::KWSASession()
{
	int retVal = WSAStartup(MAKEWORD(2, 0), &m_winsockData);

	if (retVal!= 0)
	{
		throw std::system_error(WSAGetLastError(), std::system_category(),
														"WSAStartup Failed");
	}

	if (!(LOBYTE(m_winsockData.wVersion) == 2
		&& HIBYTE(m_winsockData.wVersion) == 0))
	{
		throw std::system_error(WSAGetLastError(), std::system_category(),
			"WSAStartup Failed");
	}
}

KWSASession::~KWSASession()
{
	WSACleanup();
}

//==============================================================================

KUDPScocket::KUDPScocket()
{
	m_socketDescriptor = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(m_socketDescriptor == INVALID_SOCKET)
	{
		throw std::system_error(WSAGetLastError(), std::system_category(),
														"Error opening socket");
	}
}

//------------------------------------------------------------------------------

KUDPScocket::~KUDPScocket()
{
	closesocket(m_socketDescriptor);
}

//------------------------------------------------------------------------------

void KUDPScocket::SendTo( const std::string& address, unsigned short port,
													KPacket &packet)
{
	struct sockaddr_in serverAddress;
	memset(&serverAddress, 0, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(port);
	unsigned long serverIPAddress = inet_addr(address.c_str());
	memcpy(&serverAddress.sin_addr.S_un.S_addr, &serverIPAddress,
				sizeof(serverAddress.sin_addr));

	SendTo( serverAddress, packet );
}

//------------------------------------------------------------------------------

void KUDPScocket::SendTo( sockaddr_in& address, KPacket &packet)
{
	int send_count = sendto(m_socketDescriptor, packet.GetPacketData(),
													packet.GetPacketLength(), 0,
													reinterpret_cast<const struct sockaddr*>(&address),
													sizeof(struct sockaddr_in));

	if (send_count == SOCKET_ERROR || send_count != packet.GetPacketLength())
	{
		throw std::system_error(WSAGetLastError(), std::system_category(),
														"sendto failed");
	}
}

//------------------------------------------------------------------------------

sockaddr_in KUDPScocket::RecvFrom( KDATAPacketReceiver& dataReceiver )
{
	struct sockaddr_in incomingAddress;
	int addressSize = sizeof(struct sockaddr_in);
	std::vector<unsigned char> buffer(MAX_TFTP_PACKET_LENGTH);

	int recvCount = recvfrom(m_socketDescriptor, reinterpret_cast<char*>(buffer.data()),
														MAX_TFTP_PACKET_LENGTH, 0,
														reinterpret_cast<struct sockaddr*>(&incomingAddress),
														&addressSize);

	if (recvCount == SOCKET_ERROR)
	{
		throw std::system_error(WSAGetLastError(), std::system_category(),
			"recvfrom failed");
	}

	dataReceiver(buffer, recvCount);

	return incomingAddress;

}

//==============================================================================