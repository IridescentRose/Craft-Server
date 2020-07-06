#include "ChunkSection.h"
#include <malloc.h>
namespace Minecraft::Server::Internal::Chunks {
	ChunkSection::ChunkSection(int y)
	{
		empty = true;
		cY = y;

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
		empty = false;
		for (int x = 0; x < CHUNK_SECTION_LENGTH; x++) {
			for (int y = 0; y < CHUNK_SECTION_LENGTH; y++) {
				for (int z = 0; z < CHUNK_SECTION_LENGTH; z++) {

					int idx = (((y * CHUNK_SECTION_LENGTH) + z) * CHUNK_SECTION_LENGTH) + x;

					int rY = cY * 16 + y;

					if (rY == 0) {
						blocks[idx] = 7 << 4;
					}
					else if (rY < 63) {
						blocks[idx] = 1 << 4;
					}
					else if (rY >= 63 && rY < 66) {
						blocks[idx] = 3 << 4;
					}
					else if (rY == 66) {
						blocks[idx] = 2 << 4;
					}
					else {
						if ((x + z - rY)*3 % 2 == 0 && rY == 67) {
							blocks[idx] = (31 << 4) + 1;
						}
						else {
							blocks[idx] = 0;
						}
					}
				}
			}
		}


	}
}
