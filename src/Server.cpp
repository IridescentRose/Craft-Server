#include "Server.h"
#include "Utils.h"
#include <Network/NetworkDriver.h>
#include <stdexcept>

namespace Minecraft::Server {
	Server::Server()
	{
		m_IsRunning = false;
		socket = nullptr;


	}
	Server::~Server()
	{
	}
	void Server::run()
	{
		m_IsRunning = true;
		utilityPrint("Starting Server...", LOGGER_LEVEL_INFO);

		if (!Network::g_NetworkDriver.Init()) {
			throw std::runtime_error("Fatal: Could not connect to Network! Check Stardust Core Logs!");
		}

#ifdef CRAFT_SERVER_DEBUG
		pspDebugScreenSetXY(0, 0);
#endif

		socket = new ServerSocket(25565);
		if (socket == nullptr) {
			throw std::runtime_error("Fatal: ServerSocket is nullptr!");
		}
	}
	void Server::update()
	{
		if (socket->isAlive()) {
			//World Updates
		}
		else {
			delete socket;
			socket = NULL;
			socket = new ServerSocket(25565);
		}
	}
}