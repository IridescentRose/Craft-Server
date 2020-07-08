#pragma once
#include <queue>
#include <map>
#include <Network/NetworkDriver.h>
#include <Network/Socket.h>
#include "../Protocol/Login.h"
#include "../Protocol/Handshake.h"
#include "../Protocol/Status.h"
#include "../Protocol/Play.h"

using namespace Stardust::Network;

namespace Minecraft::Server {
	
	enum ConnectionStates {
		CONNECTION_STATE_HANDSHAKE = 0x0,
		CONNECTION_STATE_STATUS = 0x1,
		CONNECTION_STATE_LOGIN = 0x2,
		CONNECTION_STATE_PLAY = 0x3,
	};

	class NetworkManager {
	public:

		NetworkManager(ServerSocket* socket);

		void AddPacket(PacketOut* p);
		void ClearPacketQueue();
		void SendPackets();

		bool ReceivePacket();
		void HandlePackets();

		void AddPacketHandler(int id, PacketHandler h);
		void ClearPacketHandlers();

		std::queue<PacketOut*> packetQueue;
		std::queue<PacketIn*> unhandledPackets;
		std::map<int, PacketHandler> packetHandlers;

		ServerSocket* m_Socket;

		bool compression;

		inline uint8_t getConnectionStatus() {
			return connectionStatus;
		}
		void setConnectionStatus(uint8_t status);

	private:
		uint8_t connectionStatus;
	};

	extern NetworkManager* g_NetMan;
}