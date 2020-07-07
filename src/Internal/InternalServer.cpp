#include "InternalServer.h"
#include <iostream>
#include "../Utilities/Utils.h"
#include "../Protocol/Play.h"

namespace Minecraft::Server::Internal {
	InternalServer::InternalServer()
	{
		tickUpdate = NULL;
		bopen = false;
		g_World = new World();
	}
	InternalServer::~InternalServer()
	{
		tickUpdate = NULL;
	}
	void InternalServer::start()
	{
		utilityPrint("Starting Internal Server!", LOGGER_LEVEL_INFO);

		//tickUpdate = new Thread(tickUpdateThread);

		utilityPrint("Starting Update Thread!", LOGGER_LEVEL_DEBUG);
		//tickUpdate->Start(0);
#if CURRENT_PLATFORM == PLATFORM_PSP
		sceKernelDelayThread(50 * 1000);
#else
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
#endif
		bopen = true;
		g_World->init();
	}
	void InternalServer::stop()
	{
		bopen = false;

		//tickUpdate->Kill();

		g_World->cleanup();
		utilityPrint("Stopping Internal Server!", LOGGER_LEVEL_INFO);
	}
	int InternalServer::tickUpdateThread(unsigned int argc, void* argv)
	{
		while (true) {
			//Event update first then
			
			//TICK UPDATE!
#if CURRENT_PLATFORM == PLATFORM_PSP
			sceKernelDelayThread(50 * 1000);
#else
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
#endif
		}
	}

	bool InternalServer::isOpen() {
		return bopen;
	}

	InternalServer* g_InternalServer;
}
