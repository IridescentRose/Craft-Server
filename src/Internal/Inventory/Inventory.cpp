#include "Inventory.h"
#include <Utilities/JSON.h>
#include "../Registry/ItemRegistry.h"
#include "../../Protocol/Play.h"
namespace Minecraft::Server::Internal::Inventory {
	Inventory::Inventory()
	{
		for (int i = 0; i < 46; i++) {
			inventorySpots[i] = { false, (uint32_t)0, (uint8_t)0, nullptr };
		}
	}
	Inventory::~Inventory()
	{
	}
	void Inventory::init()
	{
		std::ifstream file("world/inventory.dat");

		if(file.is_open()){
			for(int i = 0; i < 46; i++){
				int id, item_count;
				bool present;

				file >> present;
				file >> id;
				file >> item_count;

				inventorySpots[i] = { present, (uint32_t)id, (uint8_t)item_count, nullptr };
			}
		}
	}
	void Inventory::cleanup()
	{
		std::ofstream file("world/inventory.dat");
		

		for (int i = 0; i < 46; i++) {
			file << (int)inventorySpots[i].present << " " << (int)inventorySpots[i].id << " " << (int)inventorySpots[i].item_count << " " << std::endl;
		}

		for (int i = 0; i < 46; i++) {
			inventorySpots[i] = { false, (uint32_t)-1, (uint8_t)-1, nullptr };
		}
	}
	Slot* Inventory::getSlot(uint8_t idx)
	{
		if (idx < 46) {
			return &inventorySpots[idx];
		}
		else {
			return NULL;
		}
	}

	void Inventory::update()
	{
	}

	bool Inventory::addItem(Slot slt)
	{
		int slot = -1;

		//Priority search
		for(int i = 36; i < 45; i++){
			if(inventorySpots[i].present){
				if(inventorySpots[i].id == slt.id){
					//Check the stack
					if(inventorySpots[i].item_count < 64){
						//Fine!
						slot = i;
						break;
					}
				}
			}
		}

		//None suitable yet, search inv.
		if(slot == -1){
			for (int i = 9; i < 36; i++) {
				if (inventorySpots[i].present) {
					if (inventorySpots[i].id == slt.id) {
						//Check the stack
						if (inventorySpots[i].item_count < 64) {
							//Fine!
							slot = i;
							break;
						}
					}
				}
			}

			if(slot == -1){
				//Find next empty

				//Prio search
				for (int i = 36; i < 45; i++) {
					if (!inventorySpots[i].present) {
						slot = i;
						break;
					}
				}

				if (slot == -1) {
					//Okay... one last try
					for (int i = 9; i < 36; i++) {
						if (!inventorySpots[i].present) {
							slot = i;
							break;
						}
					}
				}

			}
		}

		if(slot == -1){
			return false;
		}else{
			if(inventorySpots[slot].present){
				inventorySpots[slot].item_count++;
			}else{
				inventorySpots[slot].present = true;
				inventorySpots[slot].id = slt.id;
				inventorySpots[slot].item_count = 1;
			}

			Protocol::Play::PacketsOut::send_set_slot(0, slot, &inventorySpots[slot]);
			return true;
		}
	}
}