#include "ChunkSection.h"

namespace Minecraft::Server::Internal::Chunks {
	ChunkSection::ChunkSection(int y)
	{
		empty = true;
		cY = y;

		for (int x = 0; x < CHUNK_SECTION_LENGTH; x++) {
			for (int y = 0; y < CHUNK_SECTION_LENGTH; y++) {
				for (int z = 0; z < CHUNK_SECTION_LENGTH; z++) {
					blocks[x][y][z] = 0;
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
		return blocks[x][y][z];
	}

	uint8_t ChunkSection::getLightingAt(uint8_t x, uint8_t y, uint8_t z)
	{
		return 0xF;
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
					blocks[x][y][z] = 0b000000010000;
				}
			}
		}


	}
}
