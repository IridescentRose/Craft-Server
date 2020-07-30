#include "EntityManager.h"
#include "../../Protocol/Play.h"
#include "../World.h"
#include "../../Utilities/Utils.h"
#include <iostream>

namespace Minecraft::Server::Internal::Entity {
	EntityManager::EntityManager()
	{
	}
	EntityManager::~EntityManager()
	{

	}

	void generateBaseObjectMeta(ByteBuffer* meta, Entity* obj){
		//IDX
		meta->WriteBEInt8(0);
		//TYPE
		meta->WriteBEInt8(0);
		//VAL
		meta->WriteBEInt8(obj->flags);

		//IDX
		meta->WriteBEInt8(1);
		//TYPE
		meta->WriteBEInt8(1);
		//VAL
		meta->WriteVarInt32(obj->air);

		//IDX
		meta->WriteBEInt8(2);
		//TYPE
		meta->WriteBEInt8(5);
		//VAL
		if (obj->customName == "") {
			meta->WriteBool(false);
		}else{
			meta->WriteBool(true);
			meta->WriteVarUTF8String(obj->customName);
		}

		//IDX
		meta->WriteBEInt8(3);
		//TYPE
		meta->WriteBEInt8(7);
		//VAL
		meta->WriteBool(obj->isCustomNameAvailable);

		//IDX
		meta->WriteBEInt8(4);
		//TYPE
		meta->WriteBEInt8(7);
		//VAL
		meta->WriteBool(obj->silent);

		//IDX
		meta->WriteBEInt8(5);
		//TYPE
		meta->WriteBEInt8(7);
		//VAL
		meta->WriteBool(obj->noGravity);
	}

	void generateEndMeta(ByteBuffer* meta) {
		//IDX
		meta->WriteBEInt8(0xFF);
	}

	void generateItemMeta(ByteBuffer* meta, Inventory::Slot* slot){
		//IDX
		meta->WriteBEInt8(6);
		//TYPE
		meta->WriteBEInt8(6);
		//SLOT DATA
		meta->WriteBool(slot->present);
		if(slot->present){
			meta->WriteVarInt32(slot->id);
			meta->WriteBEInt8(slot->item_count);
			meta->WriteBEInt8(0);
		}
	}

	int EntityManager::addEntity(Entity* entity)
	{
		
		if (entity->objData != NULL) {
			//Spawn
			Protocol::Play::PacketsOut::send_spawn_object(entityCounter, entityCounter, entityCounter, entity->id, entity->objData->x, entity->objData->y, entity->objData->z, entity->objData->pitch, entity->objData->yaw, 1, entity->objData->vx, entity->objData->vy, entity->objData->vz);

			//Metadata
			ByteBuffer* meta = new ByteBuffer(128);

			//Meta
			generateBaseObjectMeta(meta, entity);
			if(entity->id == 2){
				generateItemMeta(meta, &((ItemEntity*)entity)->item);
			}
			generateEndMeta(meta);

			Protocol::Play::PacketsOut::send_entity_metadata(entityCounter, *meta);

			//Velocity
			Protocol::Play::PacketsOut::send_entity_velocity(entityCounter, 0, 0, 0);
		}

		entities.emplace(entityCounter, entity);
		return entityCounter++;
	}

	void EntityManager::clearEntity()
	{
		//Probably should send client some info if this is called!
		//Probably should free ram too...
		entityCounter = 0;
		entities.clear();
	}

	void EntityManager::init()
	{
		entityCounter = 0;
		entities.clear();
	}

	void EntityManager::cleanup()
	{
		//TODO: DELETE!
		entityCounter = 0;
		entities.clear();
	}

	void EntityManager::deleteEntity(int id)
	{
		Protocol::Play::PacketsOut::send_destroy_entities({ id });
		if (entities.find(id) != entities.end()) {
			delete entities[id];
			entities[id] = NULL;
			entities.erase(id);
		}
	}

	void EntityManager::update()
	{
		//Do something
		for (int i = 0; i < entities.size(); i++) {
			auto e = entities[i];
			if (e != NULL)
			{
				if (e->id == 2) {
					if (Player::g_Player.x > e->objData->x - 1.0f && Player::g_Player.x < e->objData->x + 1.0f) {
						if (Player::g_Player.z > e->objData->z - 1.0f && Player::g_Player.z < e->objData->z + 1.0f) {
							if (Player::g_Player.y > e->objData->y - 2.75f && Player::g_Player.y < e->objData->y + 0.75f) {
								if (g_World->inventory.addItem(((ItemEntity*)e)->item)) {
									deleteEntity(i);
								}
								continue;
							}
						}
					}
				}
			}
		}
	}
}


