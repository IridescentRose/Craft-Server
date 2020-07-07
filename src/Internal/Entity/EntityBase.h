#pragma once
#include "../Inventory/Slot.h"
#include <cstdint>
#include <string>

namespace Minecraft::Server::Internal::Entity {
	struct Entity {
		uint8_t flags;
		uint16_t air;
		std::string customName;
		bool isCustomNameAvailable;
		bool silent;
		bool noGravity;
	};

	struct ItemEntity : Entity {
		Inventory::Slot item;
	};
}