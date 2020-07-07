#pragma once
#include <Network/NetworkDriver.h>
#include <Events/Events.h>
#include <Utilities/JSON.h>
#include "../Internal/Chunks/ChunkColumn.h"

namespace Minecraft::Server::Protocol {

	extern Json::Value playerJSON;
	extern Json::Value banned;
	extern bool downfall;

	namespace Login {
		enum LoginPackets {
			LOGIN_START = 0x00
		};

		int login_start_packet_handler(PacketIn* p);

		namespace PacketsOut {
			//TODO: ENCRYPTION!!!
			void send_compression_packet();
			void send_login_success(std::string username);
			void send_disconnect(std::string reason, std::string color);
		}
	}
}