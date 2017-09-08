/*Bascic TFTP program*/

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include "Network.h"
#include "Packets.h"

#pragma comment( lib, "Ws2_32.lib" )

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
	try
	{
		KWSASession WSASession;
		KUDPScocket socket;

		std::ofstream outfile(targetFile, std::ofstream::binary);

		//Construct Receive Request
		KRRQPacket rrq(targetFile);
		socket.SendTo(serverIP, 69, rrq);

		//Received Packet Handler 
		KDATAPacketReceiver pktReceiver(outfile);

		while (!pktReceiver.IsTransferComplete())
		{
			struct sockaddr_in incomingAddress = socket.RecvFrom(pktReceiver);

			KACKPacket ack(pktReceiver.GetLastBlockNum());

			socket.SendTo(incomingAddress, ack);

			std::cout << pktReceiver.GetByteCount() << " bytes received in ";
			std::cout << pktReceiver.GetBlockCount();
			pktReceiver.GetBlockCount() == 1 ? std::cout << " block.\n"
																			 : std::cout << " blocks.\n";

		}
		std::cout << "Transfer Complete\n";
	}
	catch (std::system_error& e)
	{
		std::cout << e.what();
	}
	catch (KPacketReceiverException &e)
	{
		std::cout << e.what();
	}

	return 0;
}

//------------------------------------------------------------------------------