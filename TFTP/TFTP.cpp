// TFTP.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h>
#include <string>
#include <iostream>
#include <fstream>
#include "KPacket.h"

#pragma comment( lib, "Ws2_32.lib" )

using Packet = std::unique_ptr<KPacket>;

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
		return 1;
	}

	if( !( LOBYTE ( winsock_information.wVersion ) == 2 
      && HIBYTE( winsock_information.wVersion ) == 0) )
	{
		std::cout << "Can't find a suitable Winsock\n";
		WSACleanup();
		return 1;
	}

	//Create UDP Socket
	u_short serverPort( 69 );
	SOCKET socketDescriptor;
	struct sockaddr_in serverAddress;

	memset (&serverAddress, 0, sizeof( serverAddress ) );
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons( serverPort );
	u_long serverIPAddress = inet_addr( serverIP.c_str() );
	memcpy(&serverAddress.sin_addr.S_un.S_addr, &serverIPAddress, sizeof( serverAddress.sin_addr ) );

	if( ( socketDescriptor = socket( AF_INET, SOCK_DGRAM, 0 ) ) == INVALID_SOCKET )
	{
		std::cout << "Can't create socket!\n";
	}
	else
	{
		//Send Request
    Packet rrq = KPacketFactory::MakePacket( targetFile );
		
		int send_count = sendto( socketDescriptor, reinterpret_cast<const char*>( rrq->GetPacket() ),
							               rrq->GetPacketLength(), 0, reinterpret_cast<const struct sockaddr*>( &serverAddress ),
                             sizeof( struct sockaddr_in ) );
		
		if( send_count == SOCKET_ERROR || send_count != rrq->GetPacketLength() )
		{
			std::cout << "Error sending read request\n";
			return -1;
		}

    //Income address from server
		struct sockaddr_in incoming_address;
		int addressSize;
		int recvCount;
		char buffer[MAX_TFTP_PACKET_LENGTH];
		std::ofstream outfile( targetFile, std::ofstream::binary );

    //Received Packet Handler 
		KDATAPacketReceiver pktReceiver(outfile);

		//loop to receive
		while(true)
		{
			addressSize = sizeof( struct sockaddr_in );
			recvCount = recvfrom( socketDescriptor, buffer, MAX_TFTP_PACKET_LENGTH, 0,
                            reinterpret_cast<struct sockaddr*>( &incoming_address ),
                            &addressSize);

			int retVal = pktReceiver(buffer, recvCount);

			if (retVal < 0 )
			{
				break;
			}
			
      Packet ack = KPacketFactory::MakePacket( static_cast<short>( retVal ) );

			send_count = sendto( socketDescriptor, reinterpret_cast<const char*>( ack->GetPacket() ),
                           ack->GetPacketLength(), 0, reinterpret_cast<const struct sockaddr*>( &incoming_address ),
                           addressSize );

			if( send_count == SOCKET_ERROR || send_count != ack->GetPacketLength() )
			{
				std::cout << "Error sending acknowledgement\n";
				break;
			}

			std::cout << pktReceiver.GetByteCount() << " bytes received in "
		            << pktReceiver.GetBlockCount() << " blocks.\n";

			if(pktReceiver.IsTransferComplete())
			{
				break;
			}		
		}
		closesocket(socketDescriptor);
	}

	WSACleanup();
  return 0;
}

