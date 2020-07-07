#include "World.h"

#include "../Utilities/Utils.h"
#include "../Protocol/Play.h"

namespace Minecraft::Server::Internal{
	World::World()
	{
	}
	World::~World()
	{
	}

	void World::init()
	{
		chunkMap.clear();
		lastPos = { -1000, -1000 };

		entityManager.init();
	}

	void World::cleanup()
	{

		entityManager.cleanup();

		if (chunkMap.size() > 0) {
			for (auto& [pos, chunk] : chunkMap) {
				if (chunk != nullptr) {
					delete chunk;
					chunk = nullptr;
				}
				chunkMap.erase(pos);
			}
		}
	}

	void World::tickUpdate()
	{
		entityManager.update();
	}

	void World::chunkgenUpdate()
	{
		glm::ivec2 v = { (float)((int)(Player::g_Player.x / (16.0f))) - 0.5f, (float)((int)(Player::g_Player.z / (16.0f))) - 0.5f };

		if (v != lastPos) {
			//WORLD MANAGEMENT
			glm::vec2 topLeft = { v.x - 3, v.y - 3 };
			glm::vec2 botRight = { v.x + 3, v.y + 3 };

			std::vector <mc::Vector3i> needed;
			needed.clear();
			std::vector<mc::Vector3i> excess;
			excess.clear();

			for (int x = topLeft.x; x <= botRight.x; x++) {
				for (int z = topLeft.y; z <= botRight.y; z++) {
					needed.push_back({ x, z, 0 });
				}
			}

			for (auto& [pos, chunk] : chunkMap) {
				bool need = false;
				for (auto& v : needed) {
					if (v == pos) {
						//Is needed
						need = true;
					}
				}
				

				if (!need) {
					excess.push_back(pos);
				}
			}

			//DIE OLD ONES!
			for (auto chk : excess) {
				Protocol::Play::PacketsOut::send_unload_chunk(chunkMap[chk]->getX(), chunkMap[chk]->getZ());
				delete chunkMap[chk];
				chunkMap.erase(chk);
			}

			//Make new
			for (auto chk : needed) {
				if (chunkMap.find(chk) == chunkMap.end()) {
					ChunkColumn* chunk = new ChunkColumn(chk.x, chk.y);

					for (int i = 0; i < 5; i++) {
						ChunkSection* chks = new ChunkSection(i);
						chks->generateTestData();
						chunk->addSection(chks);
					}

					Protocol::Play::PacketsOut::send_chunk(chunk, true);

					chunkMap.emplace(chk, std::move(chunk));
				}
			}
			lastPos = v;
		}

	}

	World* g_World = NULL;
}
