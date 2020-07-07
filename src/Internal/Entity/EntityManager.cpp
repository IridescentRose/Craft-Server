#include "EntityManager.h"

namespace Minecraft::Server::Internal::Entity {
	EntityManager::EntityManager()
	{
	}
	EntityManager::~EntityManager()
	{

	}

	int EntityManager::addEntity(Entity* entity)
	{
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
		if (entities.find(id) != entities.end()) {
			delete entities[id];
			entities.erase(id);
		}
	}

	void EntityManager::update()
	{
		//Do something
	}
}


