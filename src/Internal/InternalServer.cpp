#include "InternalServer.h"
#include "../Utils.h"

namespace Minecraft::Server::Internal {
	InternalServer::InternalServer()
	{
		tickUpdate = NULL;
	}
	InternalServer::~InternalServer()
	{
		tickUpdate = NULL;
	}
	void InternalServer::start()
	{
		utilityPrint("Starting Internal Server!", LOGGER_LEVEL_INFO);

		tickUpdate = new Thread(tickUpdateThread);

		utilityPrint("Starting Update Thread!", LOGGER_LEVEL_DEBUG);
		tickUpdate->Start(0);
		sceKernelDelayThread(50 * 1000);
	}
	void InternalServer::stop()
	{
		tickUpdate->Kill();
		utilityPrint("Stopping Internal Server!", LOGGER_LEVEL_INFO);
	}
	int InternalServer::tickUpdateThread(unsigned int argc, void* argv)
	{
		while (true) {
			//Event update first then
			
			//TICK UPDATE!
			sceKernelDelayThread(50 * 1000);
		}
	}

	InternalServer* g_InternalServer;
}
