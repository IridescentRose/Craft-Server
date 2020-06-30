#include "Status.h"
#include "../Utilities/Config.h"

namespace Minecraft::Server::Protocol {
	int Status::request_packet_handler(PacketIn* p)
	{
		PacketsOut::send_response();
		return 0;
	}

	int Status::ping_packet_handler(PacketIn* p)
	{
		uint64_t l;
		p->buffer->ReadBEUInt64(l);

		PacketsOut::send_pong(l);
		return 0;
	}

	void Status::PacketsOut::send_response()
	{
		PacketOut* p2 = new PacketOut(512);
		p2->ID = 0x00;

		std::string st = "{\"description\":{\"text\":\"" + g_Config.motd + "\"},\"players\":{\"max\":" + std::to_string((int)g_Config.max_players) + ",\"online\":0},\"version\":{\"name\":\"1.12.2\",\"protocol\":340}}";
		p2->buffer->WriteVarUTF8String(st);


		g_NetMan->AddPacket(p2);
		g_NetMan->SendPackets();
	}

	void Status::PacketsOut::send_pong(uint64_t l)
	{
		PacketOut* p2 = new PacketOut(9);
		p2->ID = 0x01;

		p2->buffer->WriteBEUInt64(l);

		g_NetMan->AddPacket(p2);
		g_NetMan->SendPackets();
		g_NetMan->m_Socket->Close();
	}
}