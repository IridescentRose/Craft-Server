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

	struct Living : Entity {
		uint8_t handState;
		float health;
		uint16_t potionEffectColor;
		bool reducedParticles;
		uint16_t numArrows;
	};

	struct PlayerEntity : Entity {
		float additionalHearts;
		uint32_t score;
		uint8_t skinParts;
		uint8_t mainHand;
		char* leftShoulder;
		char* rightShoulder;
	};
}