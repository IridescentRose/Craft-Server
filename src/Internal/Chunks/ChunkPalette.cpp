#include "ChunkPalette.h"

Minecraft::Server::Internal::Chunks::BlockPalette::BlockPalette(uint8_t bitsPerBlock)
{
	numberOfRegistries = 0;
	bpb = bitsPerBlock;
	paletteMap.clear();
}

uint8_t Minecraft::Server::Internal::Chunks::BlockPalette::getBitsPerBlock()
{
	return bpb;
}

void Minecraft::Server::Internal::Chunks::BlockPalette::addMapping(BlockID id)
{
	paletteMap.emplace(numberOfRegistries++, id);
}
