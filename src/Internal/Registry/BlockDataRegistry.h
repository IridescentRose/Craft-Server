#pragma once
#include <string>
#include <map>
#include "../Blocks/BlockData.h"
#include "InverseMap.h"

namespace Minecraft::Server::Internal::Registry {
	using namespace Blocks;

	class BlockDataRegistry {
	public:
		BlockDataRegistry();
		~BlockDataRegistry();

		void registerVanillaBlockData();
		BlockData* getDataByName(std::string namespaceid);

	private:
		std::map<std::string, BlockData*> registryMap;
	};

	extern BlockDataRegistry* g_BlockDataRegistry;
}