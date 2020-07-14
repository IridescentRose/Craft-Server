#include "ChunkColumn.h"
#include <iostream>

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
		for (auto chnk : sections) {
			chnk->saveChanges();
			delete chnk;
		}
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
		chnks->cX = cX;
		chnks->cZ = cZ;

		chnks->loadChanges();

		sections.push_back(chnks);
	}

	void ChunkColumn::clearSections()
	{
		sections.clear();
	}

}