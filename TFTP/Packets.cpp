#include "Packets.h"
//------------------------------------------------------------------------------

std::vector<unsigned char> KPacket::ShortToChar( unsigned short block )
{
	std::vector<unsigned char> tempVec;
	tempVec.push_back( static_cast<unsigned char>(block >> 8 & 0xff) );
	tempVec.push_back(static_cast<unsigned char>(block & 0xff) );
	return tempVec;
}

KRRQPacket::KRRQPacket( std::string fileName, std::string mode )
{
	std::vector<unsigned char> opcode{ 0x00, RRQ };
	unsigned char padding = 0x00;

	copy(opcode.begin(), opcode.end(), back_inserter(m_packet));
	copy(fileName.begin(), fileName.end(), back_inserter(m_packet));
	m_packet.push_back(padding);
	copy(mode.begin(), mode.end(), back_inserter(m_packet));
	m_packet.push_back(padding);

	m_dataPtr = reinterpret_cast<const char*>(m_packet.data());
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
	std::vector<unsigned char> temp = ShortToChar(block);
	copy( temp.begin(), temp.end(), back_inserter(m_packet));

	m_dataPtr = reinterpret_cast<const char*>(m_packet.data());
	m_packetLength = m_packet.size();
}

//------------------------------------------------------------------------------

KACKPacket::~KACKPacket()
{
}

//==============================================================================

KDATAPacket::KDATAPacket( unsigned short block, std::vector<unsigned char>& data )
{
	std::vector<unsigned char> opcode{ 0x00, DATA };

	copy(opcode.begin(), opcode.end(), back_inserter(m_packet));
	std::vector<unsigned char> temp = ShortToChar(block);
	copy(temp.begin(), temp.end(), back_inserter(m_packet));
	copy(data.begin(), data.end(), back_inserter(m_packet));

	m_dataPtr = reinterpret_cast<const char*>(m_packet.data());
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

void KDATAPacketReceiver::operator()( std::vector<unsigned char> data, int recv_count )
{
	if (data[1] != KPacket::DATA)
	{
		if (data[1] == KPacket::ERROR_PACKET)
		{
			throw KPacketReceiverException("ERROR Packet");
		}
		throw KPacketReceiverException("Unexpected Packet");
	}

	unsigned short blockNum = CharToShort(&data[2]);

	if ((recv_count - 4 > 0) && (blockNum == m_expectedBlockNum))
	{
		m_byteCount += recv_count - 4;
		m_blockCount++;
		m_expectedBlockNum++;
		m_outfile.write(reinterpret_cast<const char*>(&data[4]), recv_count - 4);
	}

	if (recv_count != MAX_TFTP_PACKET_LENGTH)
	{
		m_trasnsferComplete = true;
	}

	m_receivedBlockNum = blockNum;
}

//------------------------------------------------------------------------------

unsigned short KDATAPacketReceiver::CharToShort(unsigned char* block )
{
	return static_cast<unsigned short>((*block << 8) | *(block + 1));
}

//==============================================================================

