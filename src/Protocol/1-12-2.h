#pragma once
#include <Network/NetworkDriver.h>

using namespace Stardust::Network;

namespace Minecraft::Server::Protocol {
	namespace Handshake {

		enum HandShakePackets {
			HANDSHAKE = 0x00
		};

		int handshakePacketHandler(PacketIn* p);
	}

	namespace Status {
		enum StatusPackets {
			REQUEST	= 0x00,
			PING	= 0x01
		};

		int requestPacketHandler(PacketIn* p);
		int pingPacketHandler(PacketIn* p);
	}
}