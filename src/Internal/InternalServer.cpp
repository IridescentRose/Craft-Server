#include "InternalServer.h"
#include <iostream>
#include "../Utilities/Utils.h"
#include "../Protocol/Play.h"
#include "Registry/ItemRegistry.h"
#include "Registry/BlockRegistry.h"

namespace Minecraft::Server::Internal {
	InternalServer::InternalServer()
	{
		tickUpdate = NULL;
		bopen = false;
		g_World = new World();
		Registry::g_ItemRegistry = new Registry::ItemRegistry();
		Registry::g_ItemRegistry->registerVanillaItems();
		Registry::g_BlockRegistry = new Registry::BlockRegistry();
		Registry::g_BlockRegistry->registerVanillaBlocks();
	}
	InternalServer::~InternalServer()
	{
		tickUpdate = NULL;
	}
	void InternalServer::start()
	{
		utilityPrint("Starting Internal Server!", LOGGER_LEVEL_INFO);
		bopen = true;
		Player::g_Player.currentItemSlot = 0;
		g_World->init();
	}
	void InternalServer::stop()
	{
		
		bopen = false;

		g_World->cleanup();
		utilityPrint("Stopping Internal Server!", LOGGER_LEVEL_INFO);
	}
	int InternalServer::tickUpdateThread(unsigned int argc, void* argv)
	{
		while (true) {
			//Event update first then
			
			//TICK UPDATE!
			Platform::delayForMS(50);
		}
	}

	bool InternalServer::isOpen() {
		return bopen;
	}

	InternalServer* g_InternalServer;
}
