#pragma once
#include "ChunkDefines.h"

namespace Minecraft::Server::Internal::Chunks {
	class ChunkSection {
	public:
		ChunkSection(int y);
		~ChunkSection();

		bool isEmpty();

		BlockID getBlockAt(uint8_t x, uint8_t y, uint8_t z);
		uint8_t getLightingAt(uint8_t x, uint8_t y, uint8_t z);
		uint8_t getSkyLightAt(uint8_t x, uint8_t y, uint8_t z);

		void generateTestData();

		inline int getY() {
			return cY;
		}

		BlockID *blocks;
	private:
		bool empty;
		int cY;
	};
}