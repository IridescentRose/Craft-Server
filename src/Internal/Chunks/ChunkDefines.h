#pragma once
#include <cstdint>

namespace Minecraft::Server::Internal::Chunks {
	const int CHUNK_SECTION_WIDTH = 16;
	const int CHUNK_SECTION_HEIGHT = 16;
	const int CHUNK_SECTION_LENGTH = 16;

	const int CHUNK_COLUMN_NUM_SECTIONS = 16;

	const int CHUNK_SECTION_TOTAL_BLOCKS = 4096;

	const int CHUNK_COLUMN_TOTAL_BLOCKS = 4096 * 16;

	typedef uint16_t BlockID;
}