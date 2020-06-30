#pragma once
#include <Network/NetworkDriver.h>
#include <Events/Events.h>
#include "../Utilities/Utils.h"
#include "../Networking/NetworkManager2.h"
#include <Utilities/JSON.h>

using namespace Stardust::Events;
using namespace Stardust::Network;

namespace Minecraft::Server::Protocol {
	namespace Handshake {

		enum HandShakePackets {
			HANDSHAKE = 0x00
		};

		int handshake_packet_handler(PacketIn* p);

		namespace PacketsOut {
			//There are none for Handshakes
		}
	}
}