#include "stdafx.h"
#include "KPacket.h"
#include <iostream>

//------------------------------------------------------------------------------

std::vector<char> KPacket::ShortToChar( unsigned short in )
{
  std::vector<char> temp;
  temp.push_back( in >> 8 & 0xff );
  temp.push_back( in & 0xff );
  return temp;
}

KRRQPacket::KRRQPacket( std::string fileName, std::string mode )
{
	std::vector<char> opcode{ 0x00, RRQ };
	char padding = 0x00;

	copy(opcode.begin(), opcode.end(), back_inserter(m_packet));
	copy(fileName.begin(), fileName.end(), back_inserter(m_packet));
	m_packet.push_back(padding);
	copy(mode.begin(), mode.end(), back_inserter(m_packet));
	m_packet.push_back(padding);

	m_dataPtr = m_packet.data();
	m_packetLength = m_packet.size();
}

//------------------------------------------------------------------------------

KRRQPacket::~KRRQPacket()
{

}

//==============================================================================

KACKPacket::KACKPacket( unsigned short block )
{
	std::vector<char> opcode{ 0x00,  ACK };

	copy(opcode.begin(), opcode.end(), back_inserter(m_packet));
	std::vector<char> temp = ShortToChar(block);
	copy( temp.begin(), temp.end(), back_inserter(m_packet));

	m_dataPtr = m_packet.data();
	m_packetLength = m_packet.size();
}

//------------------------------------------------------------------------------

KACKPacket::~KACKPacket()
{
}

//==============================================================================

KDATAPacket::KDATAPacket( unsigned short block, std::vector<char>& data )
{
	std::vector<char> opcode{ 0x00, DATA };

	copy(opcode.begin(), opcode.end(), back_inserter(m_packet));
	std::vector<char> temp = ShortToChar(block);
	copy(temp.begin(), temp.end(), back_inserter(m_packet));
	copy(data.begin(), data.end(), back_inserter(m_packet));

	m_dataPtr = m_packet.data();
	m_packetLength = m_packet.size();
}

//------------------------------------------------------------------------------

KDATAPacket::~KDATAPacket()
{

}

//==============================================================================

KDATAPacketReceiver::KDATAPacketReceiver( std::ofstream& outfile )
  : m_outfile( outfile )
{
}

//------------------------------------------------------------------------------

KDATAPacketReceiver::~KDATAPacketReceiver()
{
}

//------------------------------------------------------------------------------

int KDATAPacketReceiver::operator()( std::vector<char> data, int recv_count )
{
	if (recv_count == SOCKET_ERROR)
	{
		std::cout << "Error receiving data\n";
		return -1;
	}

	if (data[1] != KPacket::DATA)
	{
		if (data[1] == KPacket::ERROR_PACKET)
		{
			std::cout << "Error packet received\n";
			return -1;
		}
		std::cout << "Not data packet received\n";
		return -1;
	}

	unsigned short blockNum = CharToShort(&data[2]);

	if ((recv_count - 4 > 0) && (blockNum == m_expectedBlockNum))
	{
		m_byteCount += recv_count - 4;
		m_blockCount++;
		m_expectedBlockNum++;
		m_outfile.write(&data[0], recv_count - 4);
	}

	if (recv_count != MAX_TFTP_PACKET_LENGTH)
	{
	
	}

	return blockNum;
}

//------------------------------------------------------------------------------

unsigned short KDATAPacketReceiver::CharToShort( char msg[] )
{
	return (static_cast<unsigned short>(msg[0]) << 8) | static_cast<unsigned short>(msg[1]);
}

//==============================================================================

std::unique_ptr<KPacket> KPacketFactory::MakePacket( std::string fileName, std::string mode )
{
	return std::make_unique<KRRQPacket>( fileName, mode );
}

//------------------------------------------------------------------------------

std::unique_ptr<KPacket> KPacketFactory::MakePacket( unsigned short block )
{
	return std::make_unique<KACKPacket>( block );
}

//------------------------------------------------------------------------------

std::unique_ptr<KPacket> KPacketFactory::MakePacket(unsigned short block, std::vector<char>& data)
{
	return std::make_unique<KDATAPacket>( block, data );
}

//------------------------------------------------------------------------------

