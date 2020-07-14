#pragma once

#include "Player/Player.h"
#include <glm/glm.hpp>
#include <map>
#include "Chunks/ChunkColumn.h"
#include "Entity/EntityManager.h"
#include "Inventory/Inventory.h"
#include <mclib/common/Vector.h>
using namespace Minecraft::Server::Internal::Chunks;

namespace Minecraft::Server::Internal {
	class World {
	public:
		World();
		~World();

		void init();
		void cleanup();

		void tickUpdate();
		void chunkgenUpdate();

		//TODO: GET BLOCK GLOBAL
		BlockID getBlockAtLocationAbsolute(int x, int y, int z);
		BlockID getBlockAtLocationRelative(int chunkX, int chunkY, int x, int y, int z);
		//TODO: SET BLOCK GLOBAL
		void setBlockAtLocationAbsolute(int x, int y, int z, BlockID block);
		void setBlockAtLocationRelative(int chunkX, int chunkY, int x, int y, int z, BlockID block);

		Entity::EntityManager entityManager;
		Inventory::Inventory inventory;

		glm::ivec2 lastPos;
		std::map<mc::Vector3i, ChunkColumn*> chunkMap;
	private:

	};

	extern World* g_World;
}