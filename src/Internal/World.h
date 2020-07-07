#pragma once

#include "Player/Player.h"
#include <glm/glm.hpp>
#include <map>
#include "Chunks/ChunkColumn.h"
#include <mclib/common/Vector.h>
using namespace Minecraft::Server::Internal::Chunks;

namespace Minecraft::Server::Internal {
	class World {
	public:
		World();
		~World();

		void init();
		void cleanup();

		void chunkgenUpdate();

		//TODO: GET BLOCK GLOBAL
		//TODO: SET BLOCK GLOBAL

		glm::ivec2 lastPos;
		std::map<mc::Vector3i, ChunkColumn*> chunkMap;
	private:

	};

	extern World* g_World;
}