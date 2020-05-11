#include "Server.h"
#include "Utils.h"
#include <Network/NetworkDriver.h>
#include <stdexcept>

namespace Minecraft::Server {
	Server::Server()
	{
		m_IsRunning = false;
		socket = nullptr;

		netman = nullptr;
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

		netman = new NetworkManager(socket);
		if (netman == nullptr) {
			throw std::runtime_error("Fatal: Network Manager is nullptr!");
		}
	}
	void Server::update()
	{
		if (socket->isAlive()) {
			//Receive a max of 50 packets
			int packetsRecv = 0;
			while (netman->ReceivePacket() && packetsRecv < 50) {
				packetsRecv++;
			}

			netman->HandlePackets();


			//World Updates



			netman->SendPackets();
		}
		else {
			delete socket;
			m_IsRunning = false;
		}
	}
}