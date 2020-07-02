#include "InternalServer.h"
#include "../Utilities/Utils.h"
#include "../Protocol/Play.h"

namespace Minecraft::Server::Internal {
	InternalServer::InternalServer()
	{
		tickUpdate = NULL;
		chunkMap.clear();
		bopen = false;
		lastPos = { -1000, -1000 };
	}
	InternalServer::~InternalServer()
	{
		tickUpdate = NULL;
	}
	void InternalServer::start()
	{
		utilityPrint("Starting Internal Server!", LOGGER_LEVEL_INFO);

		tickUpdate = new Thread(tickUpdateThread);

		utilityPrint("Starting Update Thread!", LOGGER_LEVEL_DEBUG);
		//tickUpdate->Start(0);
#if CURRENT_PLATFORM == PLATFORM_PSP
		sceKernelDelayThread(50 * 1000);
#else
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
#endif
		bopen = true;

	}
	void InternalServer::stop()
	{
		bopen = false;

		//tickUpdate->Kill();
		utilityPrint("Stopping Internal Server!", LOGGER_LEVEL_INFO);
		lastPos = { -100000, -100000 };
		for (auto& [pos, chunk] : chunkMap) {
			delete chunk;
			chunkMap.erase(pos);
		}
	}
	int InternalServer::tickUpdateThread(unsigned int argc, void* argv)
	{
		while (true) {
			//Event update first then
			
			//TICK UPDATE!
#if CURRENT_PLATFORM == PLATFORM_PSP
			sceKernelDelayThread(50 * 1000);
#else
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
#endif
		}
	}

	bool InternalServer::isOpen() {
		return bopen;
	}

	void InternalServer::chunkgenUpdate()
	{

		glm::ivec2 v = { (int)(Player::g_Player.x / (16.0f)), (int)(Player::g_Player.z / (16.0f)) };

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

					for (int i = 0; i < 5; i++){
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

	InternalServer* g_InternalServer;
}
