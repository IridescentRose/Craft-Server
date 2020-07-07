#pragma once
#include "EntityBase.h"
#include <map>

namespace Minecraft::Server::Internal::Entity {
	class EntityManager {
	public:
		EntityManager();
		~EntityManager();

		//TODO: ADD INTERACTIONS 
		int addEntity(Entity* entity);
		void clearEntity();

		void deleteEntity(int id);

		void update();

	private:
		std::map<int, Entity*> entities;
		int entityCounter;
	};
}