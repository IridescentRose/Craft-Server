#pragma once
#include <string>
#include "../Entity/EntityBase.h"

namespace Minecraft::Server::Internal::Player {
	struct Player : public Entity::PlayerEntity {
		std::string username, uuid;
		uint8_t gamemode;
		uint8_t operatorLevel;

		double x, y, z;
		float yaw, pitch;
		bool onGround;

		int currentItemSlot;
	};

	extern Player g_Player;
}