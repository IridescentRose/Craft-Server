#pragma once
#include <map>
#include "ChunkDefines.h"

namespace Minecraft::Server::Internal::Chunks {
	class BlockPalette {
	public:
		BlockPalette(uint8_t bitsPerBlock);
		uint8_t getBitsPerBlock();
		void addMapping(BlockID id);
		std::map<uint32_t, BlockID> paletteMap;
	private:
		uint16_t numberOfRegistries;
		uint8_t bpb;
	};
}