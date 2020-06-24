#include "ChunkColumn.h"

namespace Minecraft::Server::Internal::Chunks {
	ChunkColumn::ChunkColumn(int x, int z)
	{
		cX = x;
		cZ = z;
		sections.clear();

		for (int xx = 0; xx < CHUNK_SECTION_LENGTH; xx++) {
			for (int yy = 0; yy < CHUNK_SECTION_LENGTH; yy++) {
				biomes[xx][yy] = 1;
			}
		}
	}

	ChunkColumn::~ChunkColumn()
	{
	}

	ChunkSection* ChunkColumn::getSection(uint8_t y)
	{
		for (auto chnk : sections) {
			if (chnk->getY() == y) {
				return chnk;
			}
		}
		return NULL;
	}

	void ChunkColumn::addSection(ChunkSection* chnks)
	{
		sections.push_back(chnks);
	}

}