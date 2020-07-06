#include "Server.h"
#include <Network/NetworkDriver.h>
#include <stdexcept>

#include "Utilities/Utils.h"
#include "Utilities/Config.h"

#include "Protocol/Play.h"
#include "Internal/InternalServer.h"

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

		if (!Network::g_NetworkDriver.Init()) {
			throw std::runtime_error("Fatal: Could not connect to Network! Check Stardust Core Logs!");
		}

#ifdef CRAFT_SERVER_DEBUG
#if CURRENT_PLATFORM == PLATFORM_PSP
		pspDebugScreenInit();
		pspDebugScreenClear();
		pspDebugScreenSetXY(0, 0);
#endif
#endif

		socket = new ServerSocket(g_Config.port);
		if (socket == nullptr) {
			throw std::runtime_error("Fatal: ServerSocket is nullptr!");
		}

		g_NetMan = new NetworkManager(socket);
		if (g_NetMan == nullptr) {
			throw std::runtime_error("Fatal: Network Manager is nullptr!");
		}

		Internal::g_InternalServer = new Internal::InternalServer();

		socket->setConnectionStatus(CONNECTION_STATE_HANDSHAKE);
	}

	int count = 0;

	void Server::update()
	{
		if (g_Server->socket->isAlive()) {

			if (g_NetMan->m_Socket->getConnectionStatus() == CONNECTION_STATE_PLAY) {
				if (!Internal::g_InternalServer->isOpen()) {
					Internal::g_InternalServer->start();

					Internal::g_InternalServer->lastPos = { -1000, -1000 };
				}
			}
			int pc = 0;

			if (g_NetMan->m_Socket->getConnectionStatus() == CONNECTION_STATE_PLAY) {
				Internal::g_InternalServer->chunkgenUpdate();
			}

			while (g_NetMan->ReceivePacket() && pc < 50) {
				pc++;
			}

			g_NetMan->HandlePackets();

			//World Updates

			g_NetMan->SendPackets();

			count++;
			if (count % 20 == 0) {
				Protocol::Play::PacketsOut::send_keepalive(0xCAFEBABECAFEBABE);
			}
		}
		else {
			g_Server->socket->Close();
			Internal::g_InternalServer->stop();
			g_Server->socket->ListenState();
			g_Server->socket->setConnectionStatus(CONNECTION_STATE_HANDSHAKE);
		}
	}

	Server* g_Server = new Server();
}