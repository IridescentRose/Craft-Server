#pragma once
#include <string>

namespace Minecraft::Server::Internal::Player {
	struct Player {
		std::string username, uuid;
		uint8_t gamemode;
		uint8_t operatorLevel;

		double x, y, z;
		float yaw, pitch;
		bool onGround;
	};

	extern Player g_Player;
}