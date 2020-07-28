#include "InternalServer.h"
#include <iostream>
#include "../Utilities/Utils.h"
#include "../Protocol/Play.h"
#include "Registry/ItemRegistry.h"
#include "Registry/BlockRegistry.h"
#include "Registry/BlockDataRegistry.h"

namespace Minecraft::Server::Internal {
	InternalServer::InternalServer()
	{
		bopen = false;
		g_World = new World();
		Registry::g_ItemRegistry = new Registry::ItemRegistry();
		Registry::g_ItemRegistry->registerVanillaItems();
		Registry::g_BlockRegistry = new Registry::BlockRegistry();
		Registry::g_BlockRegistry->registerVanillaBlocks();
		Registry::g_BlockDataRegistry = new Registry::BlockDataRegistry();
		Registry::g_BlockDataRegistry->registerVanillaBlockData();
	}
	InternalServer::~InternalServer()
	{

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

	bool InternalServer::isOpen() {
		return bopen;
	}

	void InternalServer::tickUpdate()
	{
		g_World->tickUpdate();
	}

	InternalServer* g_InternalServer;
}
