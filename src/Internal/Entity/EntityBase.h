#pragma once
#include "../Inventory/Slot.h"
#include <cstdint>
#include <string>

namespace Minecraft::Server::Internal::Entity {

	struct Object{
		double x, y, z;
		float yaw, pitch;
		uint16_t vx, vy, vz;
	};

	struct Entity {
		int id;
		uint8_t flags;
		uint16_t air;
		std::string customName;
		bool isCustomNameAvailable;
		bool silent;
		bool noGravity;
		Object* objData;
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