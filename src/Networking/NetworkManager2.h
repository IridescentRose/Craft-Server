#pragma once
#include <queue>
#include <map>
#include <Network/NetworkDriver.h>
#include "Socket2.h"

using namespace Stardust::Network;

namespace Minecraft::Server {

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

	private:
		ServerSocket* m_Socket;

	};
}