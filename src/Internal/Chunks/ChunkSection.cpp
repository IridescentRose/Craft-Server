#include "ChunkSection.h"
#include <malloc.h>
#include <fstream>
#include <Utilities/Logger.h>
#include "../Registry/BlockRegistry.h"
#include "../World.h"
#include <iostream>
namespace Minecraft::Server::Internal::Chunks {
	ChunkSection::ChunkSection(int y)
	{
		empty = true;
		cY = y;
		blocksChanged.clear();

		blocks = (BlockID*)malloc(8192);
	}

	ChunkSection::~ChunkSection()
	{
		free(blocks);
	}

	bool ChunkSection::isEmpty()
	{
		return empty;
	}

	BlockID ChunkSection::getBlockAt(uint8_t x, uint8_t y, uint8_t z)
	{
		return blocks[(((y * CHUNK_SECTION_LENGTH) + z) * CHUNK_SECTION_LENGTH) + x];
	}

	uint8_t ChunkSection::getLightingAt(uint8_t x, uint8_t y, uint8_t z)
	{
		return 0x0;
	}

	uint8_t ChunkSection::getSkyLightAt(uint8_t x, uint8_t y, uint8_t z)
	{
		return 0xF;
	}

	void ChunkSection::generateTestData()
	{

		for (int x = 0; x < CHUNK_SECTION_LENGTH; x++) {
			for (int y = 0; y < CHUNK_SECTION_LENGTH; y++) {
				for (int z = 0; z < CHUNK_SECTION_LENGTH; z++) {

					int idx = (((y * CHUNK_SECTION_LENGTH) + z) * CHUNK_SECTION_LENGTH) + x;

					int rY = cY * 16 + y;

					if (rY == 0) {
						blocks[idx] = Registry::g_BlockRegistry->getIDByName("minecraft:bedrock");
						empty = false;
					}
					else if (rY < 63) {
						blocks[idx] = Registry::g_BlockRegistry->getIDByName("minecraft:stone");
						empty = false;
					}
					else if (rY >= 63 && rY < 66) {
						blocks[idx] = Registry::g_BlockRegistry->getIDByName("minecraft:sand");
						empty = false;
					}
					else if (rY == 66) {
						blocks[idx] = Registry::g_BlockRegistry->getIDByName("minecraft:sand");
						empty = false;
					}
					else {
						if ((x + z - rY) % 32 == 0 && rY == 67) {
							blocks[idx] = Registry::g_BlockRegistry->getIDByName("minecraft:cactus");
							empty = false;
						}
						else {
							blocks[idx] = 0;
						}
					}
				}
			}
		}


	}
	void ChunkSection::changeBlock(int x, int y, int z, BlockID id)
	{
		int pos = ((((y % 16) * 16) + z) * 16) + x;
		blocks[pos] = id;

		//Trigger block updates

		//Save data entry.

		
		if(blocksChanged.find(mc::Vector3i(x, y, z)) != blocksChanged.end()){
			//It exists, modify it.
			blocksChanged[mc::Vector3i(x, y, z)] = id;
		}else{
			blocksChanged.emplace(mc::Vector3i(x, y, z), id);
		}
	}
	void ChunkSection::saveChanges()
	{
		
		if (blocksChanged.size() > 0) {
			std::ofstream file("world/chunks/" + std::to_string(cX) + " " + std::to_string(cY) + " " + std::to_string(cZ) + ".chk");
			g_World->chunkModifiedArray << cX << " " << cY << " " << cZ << std::endl;
			g_World->chunkModMap.emplace(mc::Vector3i(cX, cY, cZ), 1);

			for (auto& [v, id] : blocksChanged) {
				file << v.x << " " << v.y << " " << v.z << " " << " " << id << std::endl;
			}

			file.close();
		}
	}

	void ChunkSection::loadChanges()
	{
		std::ifstream file("world/chunks/" + std::to_string(cX) + " " + std::to_string(cY) + " " + std::to_string(cZ) + ".chk");
		if(file.is_open()){
			mc::Vector3i v;
			while(file >> v.x){
				empty = false;
				file >> v.y;
				file >> v.z;
				BlockID id;
				file >> id;

				blocksChanged.emplace(v, id);
				int pos = ((((v.y % 16) * 16) + v.z) * 16) + v.x;
				blocks[pos] = id;
			}
		}
	}
}
