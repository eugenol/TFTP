// TFTP.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h>
#include <string>
#include <iostream>
#include <fstream>
#include "KPacket.h"
#include <vector>

#pragma comment( lib, "Ws2_32.lib" )

void DoTFTPTransfer(std::string serverIP, std::string targetFile);

//------------------------------------------------------------------------------

int main( int argc, char** argv )
{
	if ( argc < 4 )
	{
		std::cout << "Usage is: tftp [host IP address] GET [filename]\n";
		return -1;
	}
	
	std::string serverIP( argv[1] );
	std::string targetFile( argv[3] );

	WSADATA winsock_information;

	//Initilise Winsock
	if( WSAStartup( MAKEWORD( 2, 0 ), &winsock_information ) != 0 )
	{
		std::cout << "Can't find a suitable Winsock\n";
		return -1;
	}

	if( !( LOBYTE ( winsock_information.wVersion ) == 2 
		&& HIBYTE( winsock_information.wVersion ) == 0) )
	{
		std::cout << "Can't find a suitable Winsock\n";
		WSACleanup();
		return -1;
	}

	DoTFTPTransfer(serverIP, targetFile);

	WSACleanup();
	return 0;
}

//------------------------------------------------------------------------------

void DoTFTPTransfer(std::string serverIP, std::string targetFile)
{
	//Create UDP Socket
	SOCKET socketDescriptor;
	if ((socketDescriptor = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
	{
		std::cout << "Can't create socket!\n";
	}
	else
	{
		//Set server address and port
		u_short serverPort(69);
		struct sockaddr_in serverAddress;
		memset(&serverAddress, 0, sizeof(serverAddress));
		serverAddress.sin_family = AF_INET;
		serverAddress.sin_port = htons(serverPort);
		u_long serverIPAddress = inet_addr(serverIP.c_str());
		memcpy(&serverAddress.sin_addr.S_un.S_addr, &serverIPAddress, sizeof(serverAddress.sin_addr));

		using Packet = std::unique_ptr<KPacket>;

		//Send Request
		Packet rrq = KPacketFactory::MakePacket(targetFile);

		int send_count = sendto(socketDescriptor, rrq->GetPacketData(), rrq->GetPacketLength(), 0,
									reinterpret_cast<const struct sockaddr*>(&serverAddress),
									sizeof(struct sockaddr_in));

		if (send_count == SOCKET_ERROR || send_count != rrq->GetPacketLength())
		{
			std::cout << "Error sending read request\n";
			return;
		}

		//Incoming address from server
		struct sockaddr_in incomingAddress;
		std::vector<char> buffer(MAX_TFTP_PACKET_LENGTH);
		std::ofstream outfile(targetFile, std::ofstream::binary);

		//Received Packet Handler 
		KDATAPacketReceiver pktReceiver(outfile);

		//loop to receive
		while (true)
		{
			int addressSize = sizeof(struct sockaddr_in);
			int recvCount = recvfrom(socketDescriptor, buffer.data(), MAX_TFTP_PACKET_LENGTH, 0,
										 reinterpret_cast<struct sockaddr*>(&incomingAddress),
										 &addressSize);

			int retVal = pktReceiver(buffer, recvCount);

			if (retVal < 0)
			{
				break;
			}

			Packet ack = KPacketFactory::MakePacket(static_cast<short>(retVal));

			send_count = sendto(socketDescriptor, ack->GetPacketData(), ack->GetPacketLength(), 0,
									reinterpret_cast<const struct sockaddr*>(&incomingAddress),
									addressSize);

			if (send_count == SOCKET_ERROR || send_count != ack->GetPacketLength())
			{
				std::cout << "Error sending acknowledgement\n";
				break;
			}

			std::cout << pktReceiver.GetByteCount() << " bytes received in ";
			std::cout << pktReceiver.GetBlockCount();
			pktReceiver.GetBlockCount() == 1 ? std::cout <<  " block.\n" : std::cout << " blocks.\n";

			if (pktReceiver.IsTransferComplete())
			{
				break;
			}
		}
		closesocket(socketDescriptor);
	}
}