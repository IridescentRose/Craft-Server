#include "Socket2.h"
#include <sys/socket.h>
#include <fcntl.h>
#include "NetworkManager2.h"
#include "../Protocol/1-12-2.h"
namespace Minecraft::Server {
	ServerSocket::ServerSocket(uint16_t port)
	{
		m_Socketfd = socket(AF_INET, SOCK_STREAM, 0);

		if (m_Socketfd == -1) {
			throw std::runtime_error("Fatal: Could not open socket! Errno: " + std::to_string(errno));
		}
		m_PortNo = port;


		int flags = fcntl(m_Socketfd, F_GETFL, 0);
		if (flags == -1) {
			throw std::runtime_error("Fatal: Socket Has Invalid Flags! Errno: " + std::to_string(errno));
		}
		flags = true ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK);
		int res = fcntl(m_Socketfd, F_SETFL, flags);

		if (res != 0) {
			throw std::runtime_error("Fatal: Socket isn't non-blocking! Errno: " + std::to_string(errno));
		}

		utilityPrint("Socket Created!", LOGGER_LEVEL_DEBUG);

		sockaddr_in sockaddr;
		sockaddr.sin_family = AF_INET;
		sockaddr.sin_addr.s_addr = INADDR_ANY;
		sockaddr.sin_port = htons(port);

		if (bind(m_Socketfd, (struct sockaddr*) & sockaddr, sizeof(sockaddr)) < 0) {
			throw std::runtime_error("Fatal: Could not bind socket address! Port: " + std::to_string(port) + ". Errno: " + std::to_string(errno));
		}

		utilityPrint("Socket Bound!", LOGGER_LEVEL_DEBUG);

		utilityPrint("Listening on socket...", LOGGER_LEVEL_DEBUG);
		if (listen(m_Socketfd, 1) < 0){
			throw std::runtime_error("Fatal: Could not listen on socket. Errno: " + std::to_string(errno));
		}

		auto addrlen = sizeof(sockaddr);
		m_Connection = accept(m_Socketfd, (struct sockaddr*)&sockaddr, (socklen_t*)&addrlen);
		utilityPrint("Found potential connection...", LOGGER_LEVEL_DEBUG);

		if (m_Connection < 0) {
			throw std::runtime_error("Fatal: Could not accept connection. Errno: " + std::to_string(errno));
		}

		utilityPrint("New Connection from " + std::to_string(inet_ntoa(sockaddr.sin_addr)) + " on port " + std::to_string(ntohs(sockaddr.sin_port)), LOGGER_LEVEL_INFO );
		connected = true;

	}

	ServerSocket::~ServerSocket()
	{
		close(m_Connection);
		close(m_Socketfd);
	}

	Network::PacketIn* ServerSocket::Recv()
	{
		Network::PacketIn* pIn = new Network::PacketIn();

		std::vector<Network::byte> len;
		Network::byte newByte;
		int res = recv(m_Connection, &newByte, 1, 0);

		if (res > 0) {

			while (newByte & 128) {
				if (res > 0) {
					len.push_back(newByte);
					res = recv(m_Connection, &newByte, 1, 0);
				}
				else if (res == 0) {
					utilityPrint("Socket connection closed!", Utilities::LOGGER_LEVEL_WARN);
					connected = false;
					return NULL;
				}
				else {
					sceKernelDelayThread(300);
				}
			}
			len.push_back(newByte);

			//We now have the length stored in len
			int length = Network::decodeVarInt(len);

			int totalTaken = 0;

			Network::byte* b = new Network::byte[length];
			for (int i = 0; i < length; i++) {
				b[i] = 0;
			}

			while (totalTaken < length) {
				int res = recv(m_Connection, b, length, 0);
				if (res > 0) {
					totalTaken += res;
				}
				else if (res == 0) {
					utilityPrint("Socket connection closed!", Utilities::LOGGER_LEVEL_WARN);
					connected = false;
					return NULL;
				}
				else {
					sceKernelDelayThread(300);
				}
			}


			for (int i = 0; i < length; i++) {
				pIn->bytes.push_back(b[i]);
			}

			pIn->pos = 0;

			pIn->ID = Network::decodeByte(*pIn);

			utilityPrint("Received Packet!", Utilities::LOGGER_LEVEL_DEBUG);
			utilityPrint("Packet ID: " + std::to_string(pIn->ID), Utilities::LOGGER_LEVEL_DEBUG);

			return pIn;
		}
		else {
			return NULL;
		}
	}

	void ServerSocket::Send(size_t size, Network::byte* buffer)
	{
		int res = send(m_Connection, buffer, size, 0);
		if (res < 0) {
			utilityPrint("Failed to send a packet!", Utilities::LOGGER_LEVEL_WARN);
			utilityPrint("Packet Size: " + std::to_string(size), Utilities::LOGGER_LEVEL_WARN);
		}
	}
	void ServerSocket::ListenState()
	{
		close(m_Connection);
		setConnectionStatus(CONNECTION_STATE_HANDSHAKE);

		sockaddr_in sockaddr;
		sockaddr.sin_family = AF_INET;
		sockaddr.sin_addr.s_addr = INADDR_ANY;
		sockaddr.sin_port = htons(m_PortNo);

		utilityPrint("Listening on socket...", LOGGER_LEVEL_DEBUG);
		if (listen(m_Socketfd, 1) < 0) {
			throw std::runtime_error("Fatal: Could not listen on socket. Errno: " + std::to_string(errno));
		}

		auto addrlen = sizeof(sockaddr);
		m_Connection = accept(m_Socketfd, (struct sockaddr*) & sockaddr, (socklen_t*)&addrlen);
		utilityPrint("Found potential connection...", LOGGER_LEVEL_DEBUG);

		if (m_Connection < 0) {
			throw std::runtime_error("Fatal: Could not accept connection. Errno: " + std::to_string(errno));
		}

		utilityPrint("New Connection from " + std::to_string(inet_ntoa(sockaddr.sin_addr)) + " on port " + std::to_string(ntohs(sockaddr.sin_port)), LOGGER_LEVEL_INFO);
		connected = true;

	}
	void ServerSocket::Close()
	{
		connected = false;
		close(m_Connection);
	}
	bool ServerSocket::isAlive()
	{
		char buffer[32] = { 0 };
		int res = recv(m_Connection, buffer, sizeof(buffer), MSG_PEEK | MSG_DONTWAIT);

		if (res == 0) {
			utilityPrint("Socket connection closed!", Utilities::LOGGER_LEVEL_WARN);
			connected = false;
		}

		return connected;
	}
	uint8_t ServerSocket::getConnectionStatus()
	{
		return connectionStatus;
	}
	void ServerSocket::setConnectionStatus(uint8_t status)
	{
		connectionStatus = status;

		g_NetMan->ClearPacketHandlers();

		switch (connectionStatus) {
		case CONNECTION_STATE_HANDSHAKE: {
			g_NetMan->AddPacketHandler(Protocol::Handshake::HANDSHAKE, Protocol::Handshake::handshakePacketHandler);
			break;
		}

		case CONNECTION_STATE_STATUS: {
			g_NetMan->AddPacketHandler(Protocol::Status::REQUEST, Protocol::Status::requestPacketHandler);
			g_NetMan->AddPacketHandler(Protocol::Status::PING, Protocol::Status::pingPacketHandler);
			break;
		}

		case CONNECTION_STATE_LOGIN: {

			break;
		}

		case CONNECTION_STATE_PLAY: {

			break;
		}
		}
	}
}