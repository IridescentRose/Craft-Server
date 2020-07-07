#pragma once
#include <cstdint>
namespace Minecraft::Server::Internal::Inventory {
	struct Slot {
		bool present;
		uint32_t id;
		uint8_t item_count;
		char* NBT;
	};
}