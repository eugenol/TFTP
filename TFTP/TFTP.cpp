// TFTP.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h>
#include <string>
#include <iostream>
#include <fstream>
#include "KPacket.h"

#pragma comment(lib, "Ws2_32.lib")

int main(int argc, char** argv)
{
	WSADATA winsock_information;

	std::string serverIP("146.230.194.143");
	u_short serverPort(69);
	std::string targetFile("test.txt");

	//if (argc != 3)
	//{
	//	std::cout << "Usage is: tftp [hostname] [filename]\n";
	//	return 1;
	//}
	//else
	//{
	//	
	//}

	//Initilise Winsock
	if(WSAStartup(MAKEWORD(2,0), &winsock_information) != 0)
	{
		std::cout << "Can't find a suitable Winsock\n";
		return 1;
	}

	if (!(LOBYTE(winsock_information.wVersion) == 2 && HIBYTE(winsock_information.wVersion) == 0)) \
	{
		std::cout << "Can't find a suitable Winsock\n";
		WSACleanup();
		return 1;
	}

	//Main body here
	SOCKET sd;
	struct sockaddr_in server_address;

	memset(&server_address, 0, sizeof(server_address));
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(serverPort);
	u_long serverIPAddress = inet_addr(serverIP.c_str());
	memcpy(&server_address.sin_addr.S_un.S_addr, &serverIPAddress, sizeof(server_address.sin_addr));

	if((sd = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
	{
		std::cout << "Can't create socket!\n";
	}
	else
	{
		//Send Request
		std::unique_ptr<KPacket> rrq = KPacketFactory::MakePacket(targetFile);
		
		int send_count = sendto(sd, reinterpret_cast<const char*>(rrq->GetPacket()), rrq->GetPacketLength(), 0, reinterpret_cast<const struct sockaddr*>(&server_address), sizeof(struct sockaddr_in));
		
		if(send_count == SOCKET_ERROR || send_count!= rrq->GetPacketLength())
		{
			std::cout << "Error sending read request\n";
			return -1;
		}

		struct sockaddr_in incoming_address;
		int address_size;
		int recv_count;
		const int MAX_TFTP_PACKET_LENGTH = 516;
		char buffer[MAX_TFTP_PACKET_LENGTH];
		std::ofstream outfile(targetFile, std::ofstream::binary);

		KDATAPacketReceiver pktReceiver(outfile);

		//loop to receive
		while(true)
		{
			address_size = sizeof(struct sockaddr_in);
			recv_count = recvfrom(sd, buffer, MAX_TFTP_PACKET_LENGTH, 0, reinterpret_cast<struct sockaddr*>(&incoming_address), &address_size);

			int retval = pktReceiver(buffer, recv_count);

			if (retval < 0)
			{
				break;
			}
			
			std::unique_ptr<KPacket> ack = KPacketFactory::MakePacket(static_cast<short>(retval));

			send_count = sendto(sd, reinterpret_cast<const char*>(ack->GetPacket()), ack->GetPacketLength(), 0, reinterpret_cast<const struct sockaddr*>(&incoming_address), address_size);

			if (send_count == SOCKET_ERROR || send_count != ack->GetPacketLength())
			{
				std::cout << "Error sending acknowledgement\n";
				break;
			}

			std::cout << pktReceiver.GetByteCount() << " bytes received over " << pktReceiver.GetBlockCount() << " blocks.\n";

			if(pktReceiver.IsTransferComplete())
			{
				break;
			}		
		}

		closesocket(sd);
	}

	WSACleanup();
    return 0;
}

