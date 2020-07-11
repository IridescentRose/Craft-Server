#include "Inventory.h"
#include <Utilities/JSON.h>
#include "../Registry/ItemRegistry.h"

namespace Minecraft::Server::Internal::Inventory {
	Inventory::Inventory()
	{
		for (int i = 0; i < 46; i++) {
			inventorySpots[i] = { false, (uint32_t)-1, (uint8_t)-1, nullptr };
		}
	}
	Inventory::~Inventory()
	{
	}
	void Inventory::init()
	{
		Json::Value vv = Stardust::Utilities::JSON::openJSON("./test.json");

		for (int i = 0; i < 9; i++) {
			inventorySpots[36 + i] = { true, (uint32_t)Registry::g_ItemRegistry->getIDByName(vv["item-id"].asString()), (uint8_t)64, nullptr };
		}
	}
	void Inventory::cleanup()
	{
		for (int i = 0; i < 46; i++) {
			inventorySpots[i] = { false, (uint32_t)-1, (uint8_t)-1, nullptr };
		}
	}
	Slot Inventory::getSlot(uint8_t idx)
	{
		if (idx < 46) {
			return inventorySpots[idx];
		}
		else {
			return Slot();
		}
	}

	void Inventory::update()
	{
	}
}