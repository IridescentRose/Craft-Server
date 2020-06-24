#pragma once
#include "ChunkSection.h"
#include <vector>

namespace Minecraft::Server::Internal::Chunks {
	class ChunkColumn {
	public:
		ChunkColumn(int x, int z);
		~ChunkColumn();

		ChunkSection* getSection(uint8_t y);
		void addSection(ChunkSection* chnks);
		
		inline int getX() {
			return cX;
		}
		inline int getZ() {
			return cZ;
		}

		inline uint8_t getBiomeAt(uint8_t x, uint8_t z) {
			return biomes[x][z];
		}

	private:
		std::vector<ChunkSection*> sections;
		uint8_t biomes[CHUNK_SECTION_LENGTH][CHUNK_SECTION_WIDTH];
		int cX, cZ;
	};
}