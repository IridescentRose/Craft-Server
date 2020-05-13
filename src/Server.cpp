#include "Server.h"
#include "Utils.h"
#include <Network/NetworkDriver.h>
#include <stdexcept>
#include "Config.h"
#include "Protocol/1-12-2.h"

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
		parseServerConfig();

		m_IsRunning = true;
		utilityPrint("Starting Server...", LOGGER_LEVEL_INFO);

		if (!Network::g_NetworkDriver.Init()) {
			throw std::runtime_error("Fatal: Could not connect to Network! Check Stardust Core Logs!");
		}

#ifdef CRAFT_SERVER_DEBUG
		pspDebugScreenInit();
		pspDebugScreenClear();
		pspDebugScreenSetXY(0, 0);
#endif

		socket = new ServerSocket(g_Config.port);
		if (socket == nullptr) {
			throw std::runtime_error("Fatal: ServerSocket is nullptr!");
		}

		g_NetMan = new NetworkManager(socket);
		if (g_NetMan == nullptr) {
			throw std::runtime_error("Fatal: Network Manager is nullptr!");
		}

		socket->setConnectionStatus(CONNECTION_STATE_HANDSHAKE);
	}
	void Server::update()
	{
		if (socket->isAlive()) {
			//Receive a max of 50 packets
			g_NetMan->ReceivePacket();
			g_NetMan->HandlePackets();


			//World Updates



			g_NetMan->SendPackets();
		}
		else {
			socket->ListenState();
			g_NetMan->ClearPacketHandlers();
			g_NetMan->AddPacketHandler(Protocol::Handshake::HANDSHAKE, Protocol::Handshake::handshakePacketHandler);
		}
	}
}