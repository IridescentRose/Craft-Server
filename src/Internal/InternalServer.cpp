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
	}
	void InternalServer::stop()
	{
		tickUpdate->Kill();
	}
	int InternalServer::tickUpdateThread(unsigned int argc, void* argv)
	{
		return 0;
	}

	InternalServer* g_InternalServer;
}
