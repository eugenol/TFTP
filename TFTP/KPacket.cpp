#include "stdafx.h"
#include "KPacket.h"
#include <iostream>

//------------------------------------------------------------------------------

void KPacket::ShortToChar( unsigned short in, char* out )
{
	out[0] = (in >> 8) & 0xff;
	out[1] = in & 0xff;
}

KRRQPacket::KRRQPacket( std::string fileName, std::string mode )
{
	char opcode[2] = { 0x00, static_cast<char>(PacketType::RRQ) };
	u_char padding = 0x00;

	m_packet = new char[2 + fileName.length() + 1 + mode.length() + 1];

	int messageIndex = 0;
	m_packet[messageIndex++] = opcode[0];
	m_packet[messageIndex++] = opcode[1];

	for( size_t i = 0; i < fileName.length(); ++i )
	{
		m_packet[messageIndex++] = fileName[i];
	}
	m_packet[messageIndex++] = padding;

	for( size_t i = 0; i < mode.length(); ++i )
	{
		m_packet[messageIndex++] = mode[i];
	}
	m_packet[messageIndex++] = padding;
	
	m_packetLength = messageIndex;
}

//------------------------------------------------------------------------------

KRRQPacket::~KRRQPacket()
{
	if ( m_packet )
	{
		delete[] m_packet;
	}
}

//==============================================================================

KACKPacket::KACKPacket( unsigned short block )
{
	char opcode[2] = { 0x00, static_cast<char>(PacketType::ACK) };

	m_packet = new char[4];

	int messageIndex = 0;
	m_packet[messageIndex++] = opcode[0];
	m_packet[messageIndex++] = opcode[1];

	ShortToChar(block, &m_packet[2]);

	m_packetLength = 4;
}

//------------------------------------------------------------------------------

KACKPacket::~KACKPacket()
{
	if( m_packet )
	{
		delete[] m_packet;
	}
}

//==============================================================================

KDATAPacket::KDATAPacket(unsigned short block, char* data, int dataSize)
{
	char opcode[2] = { 0x00, static_cast<char>(PacketType::DATA) };

	m_packet = new char[4 + dataSize];

	char blocknum[2];

	ShortToChar( block, blocknum );

	int messageIndex = 0;
	m_packet[messageIndex++] = opcode[0];
	m_packet[messageIndex++] = opcode[1];
	m_packet[messageIndex++] = blocknum[0];
	m_packet[messageIndex++] = blocknum[1];

	for (auto i = 0; i < dataSize; ++i)
	{
		m_packet[messageIndex++] = data[i];
	}

	m_packetLength = messageIndex;
}

//------------------------------------------------------------------------------

KDATAPacket::~KDATAPacket()
{
	if( m_packet )
	{
		delete m_packet;
	}
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

int KDATAPacketReceiver::operator()( char* data, int recv_count )
{
	if( recv_count == SOCKET_ERROR )
	{
		std::cout << "Error receiving data\n";
		return -1;
	}

	if( data[1] != static_cast<char>(PacketType::DATA) )
	{
		if( data[1] == static_cast<char>(PacketType::ERROR_PACKET) )
		{
			std::cout << "Error packet received\n";
			return -1;
		}
		std::cout << "Not data packet received\n";
		return -1;
	}

	unsigned short blockNum = CharToShort( &data[2] );

	if( ( recv_count - 4 > 0 ) && ( blockNum == m_expectedBlockNum ) )
	{
		m_byteCount += recv_count - 4;
		m_blockCount++;
		m_expectedBlockNum++;
		m_outfile.write( data, recv_count - 4);
	}

	if( recv_count != MAX_TFTP_PACKET_LENGTH )
	{
		m_trasnsferComplete = true;
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

std::unique_ptr<KPacket> KPacketFactory::MakePacket( unsigned short block, char* data, int dataSize )
{
	return std::make_unique<KDATAPacket>( block, data, dataSize );
}

//------------------------------------------------------------------------------

