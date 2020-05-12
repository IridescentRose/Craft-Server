#include "1-12-2.h"
#include "../Networking/NetworkManager2.h"
#include "../Utils.h"
#include <Utilities/JSON.h>

namespace Minecraft::Server::Protocol {

	int Handshake::handshakePacketHandler(PacketIn* p)
	{
		utilityPrint("HANDSHAKE RECEIVED!", LOGGER_LEVEL_TRACE);

		int getVersion = decodeVarInt(*p);
		utilityPrint("Protocol: " + std::to_string(getVersion), LOGGER_LEVEL_TRACE);

		decodeString(*p);
		decodeShort(*p);

		int nextStatus = decodeVarInt(*p);
		utilityPrint("Connection Switch: " + std::to_string(nextStatus), LOGGER_LEVEL_TRACE);

		g_NetMan->m_Socket->setConnectionStatus(nextStatus);

		return 0;
	}

	int Status::requestPacketHandler(PacketIn* p)
	{
		PacketOut* p2 = new PacketOut();
		p2->bytes.clear();
		p2->ID = 0x00;

		std::string st = "{\"description\":{\"text\":\"Definitely not a PSP.\"},\"players\":{\"max\":1,\"online\":0},\"version\":{\"name\":\"1.12.2\",\"protocol\":340}}";

		encodeStringLE(st, *p2);

		g_NetMan->AddPacket(p2);
		g_NetMan->SendPackets();
		return 0;
	}

	int Status::pingPacketHandler(PacketIn* p)
	{
		PacketOut* p2 = new PacketOut();
		p2->ID = 0x01;
		encodeLong(decodeLong(*p), (*p2));

		g_NetMan->AddPacket(p2);
		g_NetMan->SendPackets();
		g_NetMan->m_Socket->Close();
		return 0;
	}
}

#include <Utilities/UUID.h>

int Minecraft::Server::Protocol::Login::loginStartPacketHandler(PacketIn* p)
{
	std::string username;

	username = decodeStringLE(*p);

	utilityPrint(username + " is attempting to join", LOGGER_LEVEL_INFO);

	PacketOut* p2 = new PacketOut();
	p2->ID = 0x02;
	encodeStringLE(generateUUID(), *p2);
	encodeStringLE(username, *p2);

	g_NetMan->AddPacket(p2);
	g_NetMan->SendPackets();
	return 0;
}
