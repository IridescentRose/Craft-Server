#include "Socket2.h"
#include <sys/socket.h>

namespace Minecraft::Server {
	ServerSocket::ServerSocket(uint16_t port)
	{
		m_Socketfd = socket(AF_INET, SOCK_STREAM, 0);

		if (m_Socketfd == -1) {
			throw std::runtime_error("Fatal: Could not open socket! Errno: " + std::to_string(errno));
		}
		m_PortNo = port;

		sockaddr_in sockaddr;
		sockaddr.sin_family = AF_INET;
		sockaddr.sin_addr.s_addr = INADDR_ANY;
		sockaddr.sin_port = htons(port);

		if (bind(m_Socketfd, (struct sockaddr*) & sockaddr, sizeof(sockaddr)) < 0) {
			throw std::runtime_error("Fatal: Could not bind socket address! Port: " + std::to_string(port) + ". Errno: " + std::to_string(errno));
		}

		if (listen(m_Socketfd, 1) < 0){
			throw std::runtime_error("Fatal: Could not listen on socket. Errno: " + std::to_string(errno));
		}

		auto addrlen = sizeof(sockaddr);
		m_Connection = accept(m_Socketfd, (struct sockaddr*)&sockaddr, (socklen_t*)&addrlen);

		if (m_Connection < 0) {
			throw std::runtime_error("Fatal: Could not accept connection. Errno: " + std::to_string(errno));
		}

		utilityPrint("New Connection from " + std::to_string(inet_ntoa(sockaddr.sin_addr)) + " on port " + std::to_string(ntohs(sockaddr.sin_port)), LOGGER_LEVEL_INFO );
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
				else {
					sceKernelDelayThread(300);
				}
			}
			len.push_back(newByte);

			//We now have the length stored in len
			int length = Network::decodeVarInt(len);

			utilityPrint("LENGTH: " + std::to_string(length), Utilities::LOGGER_LEVEL_DEBUG);

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
				else {
					sceKernelDelayThread(300);
				}
			}


			for (int i = 0; i < length; i++) {
				pIn->bytes.push_back(b[i]);
			}

			pIn->pos = 0;

			pIn->ID = Network::decodeShort(*pIn);

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
}