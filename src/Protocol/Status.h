#pragma once

#include <Network/NetworkDriver.h>
#include <Events/Events.h>
#include "../Utilities/Utils.h"
#include "../Networking/NetworkManager2.h"
#include <Utilities/JSON.h>

using namespace Stardust::Events;
using namespace Stardust::Network;

namespace Minecraft::Server::Protocol {
	namespace Status {
		enum StatusPackets {
			REQUEST = 0x00,
			PING = 0x01
		};

		int request_packet_handler(PacketIn* p);
		int ping_packet_handler(PacketIn* p);

		namespace PacketsOut {
			void send_response();
			void send_pong(uint64_t l);
		}
	}
}