#include "Handshake.h"
#include <iostream>
namespace Minecraft::Server::Protocol {
	int Handshake::handshake_packet_handler(PacketIn* p)
	{
		g_NetMan->compression = false;

		uint32_t getVersion;
		p->buffer->ReadVarInt32(getVersion);

		std::string ip;
		p->buffer->ReadVarUTF8String(ip);

		uint16_t port;
		p->buffer->ReadBEUInt16(port);

		uint8_t state;
		p->buffer->ReadBEUInt8(state);

		g_NetMan->setConnectionStatus(state);
		return 0;
	}
}