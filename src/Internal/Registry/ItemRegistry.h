#pragma once
#include <string>
#include <map>

namespace Minecraft::Server::Internal::Registry {
	typedef uint16_t ItemProtocolID;

	class ItemRegistry {
	public:
		ItemRegistry();
		~ItemRegistry();

		void registerVanillaItems();
		ItemProtocolID getIDByName(std::string namespaceid);

	private:
		std::map<std::string, ItemProtocolID> registryMap;
	};

	extern ItemRegistry* g_ItemRegistry;
}