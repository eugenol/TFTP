#pragma once
#include <windows.h>
#include <string>
#include <fstream>
#include <memory>
#include <vector>

//------------------------------------------------------------------------------

const int MAX_TFTP_PACKET_LENGTH = 516;

//==============================================================================

class KPacket
{
public:
	KPacket() {}
	virtual ~KPacket() {}

	virtual const char* GetPacketData() const = 0;
	virtual int	GetPacketLength() const = 0;
	static std::vector<char> ShortToChar( unsigned short in );

	enum PacketType : char { RRQ = 1, WRQ, DATA, ACK, ERROR_PACKET };
};

//==============================================================================

class KRRQPacket : public KPacket
{
public:
	KRRQPacket( std::string fileName, std::string mode = "octet" );
	~KRRQPacket();

	const char* GetPacketData() const override { return m_dataPtr; }
	int	GetPacketLength() const override { return m_packetLength; }

private:
	std::vector<char> m_packet;
	const char* m_dataPtr = nullptr;
	int m_packetLength = 0;
};

//==============================================================================

class KACKPacket : public KPacket
{
public:
	KACKPacket( unsigned short block );
	~KACKPacket();

	const char* GetPacketData() const override { return m_dataPtr; }
	int	GetPacketLength() const override { return m_packetLength; }

private:
	std::vector<char> m_packet;
	const char* m_dataPtr = nullptr;
	int m_packetLength = 0;
};

//==============================================================================

class KDATAPacket : public KPacket
{
public:
	KDATAPacket( unsigned short block, std::vector<char>& data );
	~KDATAPacket();

	const char* GetPacketData() const override { return m_dataPtr; }
	int	GetPacketLength() const override { return m_packetLength; }

private:
	std::vector<char> m_packet;
	const char* m_dataPtr = nullptr;
	int m_packetLength = 0;
};

//==============================================================================

class KDATAPacketReceiver
{
public:
	KDATAPacketReceiver( std::ofstream& outfile );
	~KDATAPacketReceiver();

	int operator()( std::vector<char> data, int recv_count);
	long GetBlockCount() const { return m_blockCount; }
	long GetByteCount() const { return m_byteCount;  }
	bool IsTransferComplete() const { return m_trasnsferComplete; }

private:
	unsigned short CharToShort( char msg[] );

	std::ofstream& m_outfile;
	unsigned short m_expectedBlockNum = 1;
	long m_blockCount = 0;
	long m_byteCount = 0;
	bool m_isValid = false;
	bool m_trasnsferComplete = false;
};

//==============================================================================

class KPacketFactory
{
public:
	KPacketFactory() = delete;
	~KPacketFactory() = delete;

	static std::unique_ptr<KPacket> MakePacket( std::string fileName, std::string mode = "octet" );
	static std::unique_ptr<KPacket> MakePacket( unsigned short block );
	static std::unique_ptr<KPacket> MakePacket( unsigned short block, std::vector<char>& data);
};

//==============================================================================