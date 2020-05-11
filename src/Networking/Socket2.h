#pragma once
#include <stdint.h>
#include <Network/Socket.h>
#include "../Utils.h"

using namespace Stardust;

namespace Minecraft::Server {
	class ServerSocket {
	public:
		ServerSocket(uint16_t port);
		~ServerSocket();

		Network::PacketIn* Recv();
		void Send(size_t size, Network::byte* buffer);

		bool isAlive();

	private:
		int m_Socketfd, m_Connection, m_PortNo;
		bool connected;

	};
}