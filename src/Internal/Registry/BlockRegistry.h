#pragma once
#include <string>
#include <map>
#include "../Blocks/BlockData.h"
#include "InverseMap.h"

namespace Minecraft::Server::Internal::Registry {
	using namespace Blocks;

	class BlockRegistry {
	public:
		BlockRegistry();
		~BlockRegistry();

		void registerVanillaBlocks();
		BlockProtocolID getIDByName(std::string namespaceid);
		std::string getNameByID(BlockProtocolID protocolid);

	private:
		std::map<std::string, BlockProtocolID> registryMap;
		std::map<BlockProtocolID, std::string> reverseMap;
	};

	extern BlockRegistry* g_BlockRegistry;
}