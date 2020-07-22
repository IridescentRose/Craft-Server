#include "Server.h"
#include <Network/NetworkDriver.h>
#include <stdexcept>

#include "Utilities/Utils.h"
#include "Utilities/Config.h"

#include "Protocol/Play.h"
#include "Internal/InternalServer.h"
#include <iostream>

namespace Minecraft::Server {
	Server::Server()
	{
		m_IsRunning = false;
		socket = nullptr;
	}
	Server::~Server()
	{
	}
	void Server::init()
	{
		parseServerConfig();

		m_IsRunning = true;
		utilityPrint("Starting Server...", LOGGER_LEVEL_INFO);

		std::cout << "B" << std::endl;
		if (!Network::g_NetworkDriver.Init()) {
			throw std::runtime_error("Fatal: Could not connect to Network! Check Stardust Core Logs!");
		}
		std::cout << "B" << std::endl;

#ifdef CRAFT_SERVER_DEBUG
#if CURRENT_PLATFORM == PLATFORM_PSP
		pspDebugScreenInit();
		pspDebugScreenClear();
		pspDebugScreenSetXY(0, 0);
#endif
#endif

		socket = new ServerSocket(g_Config.port);
		socket->SetBlock(false);
		if (socket == nullptr) {
			throw std::runtime_error("Fatal: ServerSocket is nullptr!");
		}

		g_NetMan = new NetworkManager(socket);
		g_NetMan->setConnectionStatus(CONNECTION_STATE_HANDSHAKE);
		if (g_NetMan == nullptr) {
			throw std::runtime_error("Fatal: Network Manager is nullptr!");
		}

		Internal::g_InternalServer = new Internal::InternalServer();
	}

	int count = 0;

	void Server::update()
	{
		if (g_Server->socket->isAlive()) {

			if (g_NetMan->getConnectionStatus() == CONNECTION_STATE_PLAY) {
				if (!Internal::g_InternalServer->isOpen()) {
					Internal::g_InternalServer->start();
				}
			}
			int pc = 0;

			if (g_NetMan->getConnectionStatus() == CONNECTION_STATE_PLAY) {
				Internal::g_World->chunkgenUpdate();
			}
			Internal::g_World->tickUpdate();

			while (g_NetMan->ReceivePacket() && pc < 50) {
				pc++;
			}

			g_NetMan->HandlePackets();

			//World Updates

			g_NetMan->SendPackets();

			count++;
			if (count % 20 == 0) {
				Protocol::Play::PacketsOut::send_keepalive(0xCAFEBABECAFEBABE);
				Protocol::Play::PacketsOut::send_time_update2(Internal::g_World->timedata);
			}
		}
		else {
			g_NetMan->setConnectionStatus(CONNECTION_STATE_HANDSHAKE);
			if (Internal::g_InternalServer->isOpen()) {
				Internal::g_InternalServer->stop();
			}
			g_Server->socket->ListenState();
		}
	}

	Server* g_Server = new Server();
}