#include "ChunkSection.h"

namespace Minecraft::Server::Internal::Chunks {
	ChunkSection::ChunkSection(int y)
	{
		empty = true;
		cY = y;

		for (int x = 0; x < CHUNK_SECTION_LENGTH; x++) {
			for (int y = 0; y < CHUNK_SECTION_LENGTH; y++) {
				for (int z = 0; z < CHUNK_SECTION_LENGTH; z++) {
					int idx = (((y * CHUNK_SECTION_LENGTH) + z) * CHUNK_SECTION_LENGTH) + x;
					blocks[idx] = 0;
				}
			}
		}
	}

	ChunkSection::~ChunkSection()
	{

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
					if (y == 0) {
						blocks[idx] = 7 << 4;
					}
					else if (y < 11) {
						blocks[idx] = 1 << 4;
					}
					else if (y >= 11 && y < 14) {
						blocks[idx] = 3 << 4;
					}
					else if (y == 14) {
						blocks[idx] = 2 << 4;
					}
					else {
						if ((x + z - y)*3 % 2 == 0) {
							blocks[idx] = 0;
						}
						else {
							blocks[idx] = (31 << 4) + 1;
						}
					}
				}
			}
		}


	}
}
