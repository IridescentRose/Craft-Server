#include "Server.h"
#include "Utils.h"
#include <Network/NetworkDriver.h>
#include <stdexcept>
#include "Config.h"
#include "Protocol/1-12-2.h"
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

		Internal::g_InternalServer = new Internal::InternalServer();
		Internal::g_InternalServer->start();

		socket = new ServerSocket(g_Config.port);
		if (socket == nullptr) {
			throw std::runtime_error("Fatal: ServerSocket is nullptr!");
		}

		g_NetMan = new NetworkManager(socket);
		if (g_NetMan == nullptr) {
			throw std::runtime_error("Fatal: Network Manager is nullptr!");
		}

		socket->setConnectionStatus(CONNECTION_STATE_HANDSHAKE);

		thr = new Thread(update);
		thr->Start(0);
	}
	int Server::update(unsigned int, void*)
	{
		int count = 0;
		while (true) {

			if (g_Server->socket->isAlive()) {
				int pc = 0;
				
				while (g_NetMan->ReceivePacket() && pc < 50) {
					pc++;
				}

				g_NetMan->HandlePackets();

				//World Updates

				g_NetMan->SendPackets();
			}
			else {
				g_Server->socket->ListenState();
				g_Server->socket->setConnectionStatus(CONNECTION_STATE_HANDSHAKE);
			}

			count++;
			if (count % 20 == 0) {
				Protocol::Play::PacketsOut::send_keepalive(0xCAFEBABECAFEBABE);
			}


			sceKernelDelayThread(50 * 1000);
		}

		return 0;
	}

	Server* g_Server = new Server();
}