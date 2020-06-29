#pragma once
#include <stdint.h>
#include <Network/Socket.h>
#include "../Utils.h"

using namespace Stardust;

namespace Minecraft::Server {
	enum ConnectionStates {
		CONNECTION_STATE_HANDSHAKE	= 0x0,
		CONNECTION_STATE_STATUS		= 0x1,
		CONNECTION_STATE_LOGIN		= 0x2,
		CONNECTION_STATE_PLAY		= 0x3,
	};

	class ServerSocket {
	public:
		ServerSocket(uint16_t port);
		~ServerSocket();

		Network::PacketIn* Recv();
		void Send(size_t size, char* buffer);
		void ListenState();

		void Close();

		bool isAlive();

		uint8_t getConnectionStatus();
		void setConnectionStatus(uint8_t status);

		
	private:
		int m_Socketfd, m_Connection, m_PortNo;
		bool connected;
		uint8_t connectionStatus;

	};
}