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
		std::string getNameByID(ItemProtocolID protocolid);

	private:
		std::map<std::string, ItemProtocolID> registryMap;
		std::map<ItemProtocolID, std::string> reverseMap;
	};

	extern ItemRegistry* g_ItemRegistry;
}