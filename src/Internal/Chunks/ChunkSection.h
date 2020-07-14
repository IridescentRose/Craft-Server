#pragma once
#include "ChunkDefines.h"
#include <mclib/common/Vector.h>
#include <map>

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

		void changeBlock(int x, int y, int z, BlockID);

		void saveChanges();
		void loadChanges();

		BlockID *blocks;
		int cX, cZ;
		bool empty;
	private:
		int cY;


		std::map<mc::Vector3i, BlockID> blocksChanged;
	};
}