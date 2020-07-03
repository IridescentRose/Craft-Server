#include "Socket2.h"
#if CURRENT_PLATFORM == PLATFORM_PSP || (CURRENT_PLATFORM == PLATFORM_NIX)
#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#else
#define WIN32_LEAN_AND_MEAN 1
#include <winsock2.h>
#include <windows.h>
#include <Ws2tcpip.h>
#endif

#if CURRENT_PLATFORM != PLATFORM_PSP
#include <thread>
#endif

#include "NetworkManager2.h"
#include "../Protocol/Play.h"
#include "../Protocol/Login.h"
#include "../Protocol/Handshake.h"
#include "../Protocol/Status.h"


namespace Minecraft::Server {
	ServerSocket::ServerSocket(uint16_t port)
	{
		m_Socketfd = socket(AF_INET, SOCK_STREAM, 0);

		if (m_Socketfd == -1) {
			throw std::runtime_error("Fatal: Could not open socket! Errno: " + std::to_string(errno));
		}
		m_PortNo = port;



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

#if CURRENT_PLATFORM == PLATFORM_PSP
		int yes = 1;
		if (setsockopt(m_Connection, IPPROTO_TCP, TCP_NODELAY, &yes, sizeof(yes)) == -1) {
			throw std::runtime_error("Fatal: Could not set no delay! Errno " + std::to_string(errno));
		}
#endif

		utilityPrint("New Connection from " + std::string(inet_ntoa(sockaddr.sin_addr)) + " on port " + std::to_string(ntohs(sockaddr.sin_port)), LOGGER_LEVEL_INFO );
		SetBlocking(false);

		connected = true;

	}

	ServerSocket::~ServerSocket()
	{
#if CURRENT_PLATFORM == PLATFORM_PSP || (CURRENT_PLATFORM == PLATFORM_NIX)
		close(m_Connection);
#else
		closesocket(m_Connection);
#endif

#if CURRENT_PLATFORM == PLATFORM_PSP || (CURRENT_PLATFORM == PLATFORM_NIX)
		close(m_Socketfd);
#else
		closesocket(m_Socketfd);
#endif
	}

	Network::PacketIn* ServerSocket::Recv()
	{
		std::vector<byte> len;
		byte newByte;
		int res = recv(m_Connection, &newByte, 1, MSG_PEEK);

		if (res > 0) {
			char data[5] = { 0 };
			size_t dataLen = 0;
			do {
				size_t totalReceived = 0;
				while (1 > totalReceived) {
					size_t received = recv(m_Connection, &data[dataLen] + totalReceived, 1 - totalReceived, 0);
					if (received <= 0) {
#if CURRENT_PLATFORM == PLATFORM_PSP
						sceKernelDelayThread(300 * 1000);
#else
						std::this_thread::sleep_for(std::chrono::milliseconds(300));
#endif
					}
					else {
						totalReceived += received;
					}
				}
			} while ((data[dataLen++] & 0x80) != 0);

			int readed = 0;
			int result = 0;
			char read;
			do {
				read = data[readed];
				int value = (read & 0b01111111);	
				result |= (value << (7 * readed));
				readed++;
			} while ((read & 0b10000000) != 0);


			int length = result;

			PacketIn* pIn = new PacketIn(length);
			Utilities::detail::core_Logger->log("LENGTH: " + std::to_string(length), Utilities::LOGGER_LEVEL_DEBUG);

			int totalTaken = 0;

			byte* b = new byte[length];
			for (int i = 0; i < length; i++) {
				b[i] = 0;
			}

			while (totalTaken < length) {
				int res = recv(m_Connection, b, length, 0);
				if (res > 0) {
					totalTaken += res;
				}
				else {
#if CURRENT_PLATFORM == PLATFORM_PSP
					sceKernelDelayThread(300 * 1000);
#else
					std::this_thread::sleep_for(std::chrono::milliseconds(300));
#endif
				}
			}


			for (int i = 0; i < length; i++) {
				pIn->buffer->WriteBEUInt8(b[i]);
			}

			if (pIn != NULL && pIn->buffer->GetUsedSpace() > 0) {
				uint8_t t = 0;
				pIn->buffer->ReadBEUInt8(t);
				pIn->ID = t;
			}
			else {
				pIn->ID = -1;
			}

			Utilities::detail::core_Logger->log("Received Packet!", Utilities::LOGGER_LEVEL_DEBUG);
			Utilities::detail::core_Logger->log("Packet ID: " + std::to_string(pIn->ID), Utilities::LOGGER_LEVEL_DEBUG);

			return pIn;
		}
		else {
			return NULL;
		}
	}

	void ServerSocket::Send(size_t size, char* buffer)
	{
		int res = send(m_Connection, buffer, size, 0);
		if (res < 0) {
			utilityPrint("Failed to send a packet!", Utilities::LOGGER_LEVEL_WARN);
			utilityPrint("PACKET ERROR (errno): " + std::to_string((int)errno), Utilities::LOGGER_LEVEL_WARN);
			utilityPrint("Packet Size: " + std::to_string(size), Utilities::LOGGER_LEVEL_WARN);

			utilityPrint("Socket connection closed!", Utilities::LOGGER_LEVEL_WARN);
			connected = false;
#if CURRENT_PLATFORM == PLATFORM_PSP || (CURRENT_PLATFORM == PLATFORM_NIX)
			//close(m_Connection);
#else
			closesocket(m_Connection);
#endif
		}
	}
	void ServerSocket::ListenState()
	{
#if CURRENT_PLATFORM == PLATFORM_PSP || (CURRENT_PLATFORM == PLATFORM_NIX)
		//close(m_Connection);
#else
		closesocket(m_Connection);
#endif
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

		utilityPrint("New Connection from " + std::string((inet_ntoa(sockaddr.sin_addr))) + " on port " + std::to_string(ntohs(sockaddr.sin_port)), LOGGER_LEVEL_INFO);
		SetBlocking(false);
		connected = true;

	}
	void ServerSocket::SetBlocking(bool val)
	{
#if CURRENT_PLATFORM == PLATFORM_PSP || (CURRENT_PLATFORM == PLATFORM_NIX)
		if (val) {
			int flags = fcntl(m_Connection, F_GETFL, 0);
			flags &= ~O_NONBLOCK;
			fcntl(m_Connection, F_SETFL, flags);
		}
		else {
			fcntl(m_Connection, F_SETFL, O_NONBLOCK);
		}
#else
		unsigned long iMode = 1;
		int iResult = ioctlsocket(m_Connection, FIONBIO, &iMode);
		if (iResult != NO_ERROR) {
			throw std::runtime_error("ERROR SETTING NONBLOCKING");
		}
#endif
	}

	void ServerSocket::Close()
	{
		connected = false; 
#if CURRENT_PLATFORM == PLATFORM_PSP || (CURRENT_PLATFORM == PLATFORM_NIX)
			close(m_Connection);
#else
		closesocket(m_Connection);
#endif
	}
	bool ServerSocket::isAlive()
	{
		char buffer[32] = { 0 };
		int res = recv(m_Connection, buffer, sizeof(buffer), MSG_PEEK);

		if (res == 0) {
			utilityPrint("Socket connection closed!", Utilities::LOGGER_LEVEL_WARN); 
#if CURRENT_PLATFORM == PLATFORM_PSP || (CURRENT_PLATFORM == PLATFORM_NIX)
				close(m_Connection);
#else
			closesocket(m_Connection);
#endif
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
		utilityPrint("Setting Connection Status to: " + std::to_string((int)status), LOGGER_LEVEL_INFO);
		connectionStatus = status;

		g_NetMan->ClearPacketHandlers();

		switch (connectionStatus) {
		case CONNECTION_STATE_HANDSHAKE: {
			g_NetMan->AddPacketHandler(Protocol::Handshake::HANDSHAKE, Protocol::Handshake::handshake_packet_handler);
			break;
		}

		case CONNECTION_STATE_STATUS: {
			g_NetMan->AddPacketHandler(Protocol::Status::REQUEST, Protocol::Status::request_packet_handler);
			g_NetMan->AddPacketHandler(Protocol::Status::PING, Protocol::Status::ping_packet_handler);
			break;
		}

		case CONNECTION_STATE_LOGIN: {
			g_NetMan->AddPacketHandler(Protocol::Login::LOGIN_START, Protocol::Login::login_start_packet_handler);
			break;
		}

		case CONNECTION_STATE_PLAY: {
			g_NetMan->AddPacketHandler(Protocol::Play::TELEPORT_CONFIRM, Protocol::Play::teleport_confirm_handler);
			g_NetMan->AddPacketHandler(Protocol::Play::TAB_COMPLETE, Protocol::Play::tab_complete_handler);
			g_NetMan->AddPacketHandler(Protocol::Play::CHAT_MESSAGE, Protocol::Play::chat_message_handler);
			g_NetMan->AddPacketHandler(Protocol::Play::CLIENT_STATUS, Protocol::Play::client_status_handler);
			g_NetMan->AddPacketHandler(Protocol::Play::CLIENT_SETTINGS, Protocol::Play::client_settings_handler);
			g_NetMan->AddPacketHandler(Protocol::Play::CONFIRM_TRANSACTION, Protocol::Play::confirm_transaction_handler);
			g_NetMan->AddPacketHandler(Protocol::Play::ENCHANT_ITEM, Protocol::Play::enchant_item_handler);
			g_NetMan->AddPacketHandler(Protocol::Play::CLICK_WINDOW, Protocol::Play::click_window_handler);
			g_NetMan->AddPacketHandler(Protocol::Play::CLOSE_WINDOW, Protocol::Play::close_window_handler);
			g_NetMan->AddPacketHandler(Protocol::Play::PLUGIN_MESSAGE, Protocol::Play::plugin_message_handler);
			g_NetMan->AddPacketHandler(Protocol::Play::USE_ENTITY, Protocol::Play::use_entity_handler);
			g_NetMan->AddPacketHandler(Protocol::Play::KEEP_ALIVE, Protocol::Play::keep_alive_handler);
			g_NetMan->AddPacketHandler(Protocol::Play::PLAYER, Protocol::Play::player_handler);
			g_NetMan->AddPacketHandler(Protocol::Play::PLAYER_POSITION, Protocol::Play::player_position_handler);
			g_NetMan->AddPacketHandler(Protocol::Play::PLAYER_POSITION_AND_LOOK, Protocol::Play::player_position_and_look_handler);
			g_NetMan->AddPacketHandler(Protocol::Play::PLAYER_LOOK, Protocol::Play::player_look_handler);
			g_NetMan->AddPacketHandler(Protocol::Play::VEHICLE_MOVE, Protocol::Play::vehicle_move_handler);
			g_NetMan->AddPacketHandler(Protocol::Play::STEER_BOAT, Protocol::Play::steer_boat_handler);
			g_NetMan->AddPacketHandler(Protocol::Play::CRAFT_RECIPE_REQUEST, Protocol::Play::craft_recipe_request_handler);
			g_NetMan->AddPacketHandler(Protocol::Play::PLAYER_ABILITIES, Protocol::Play::player_abilities_handler);
			g_NetMan->AddPacketHandler(Protocol::Play::PLAYER_DIGGING, Protocol::Play::player_digging_handler);
			g_NetMan->AddPacketHandler(Protocol::Play::ENTITY_ACTION, Protocol::Play::entity_action_handler);
			g_NetMan->AddPacketHandler(Protocol::Play::STEER_VEHICLE, Protocol::Play::steer_vehicle_handler);
			g_NetMan->AddPacketHandler(Protocol::Play::CRAFTING_BOOK_DATA, Protocol::Play::crafting_book_data_handler);
			g_NetMan->AddPacketHandler(Protocol::Play::RESOURCE_PACK_STATUS, Protocol::Play::resource_pack_status_handler);
			g_NetMan->AddPacketHandler(Protocol::Play::ADVANCEMENT_TAB, Protocol::Play::advancement_tab_handler);
			g_NetMan->AddPacketHandler(Protocol::Play::HELD_ITEM_CHANGE, Protocol::Play::held_item_change_handler);
			g_NetMan->AddPacketHandler(Protocol::Play::CREATIVE_INVENTORY_ACTION, Protocol::Play::creative_inventory_action_handler);
			g_NetMan->AddPacketHandler(Protocol::Play::UPDATE_SIGN, Protocol::Play::update_sign_handler);
			g_NetMan->AddPacketHandler(Protocol::Play::ANIMATION, Protocol::Play::animation_handler);
			g_NetMan->AddPacketHandler(Protocol::Play::SPECTATE, Protocol::Play::spectate_handler);
			g_NetMan->AddPacketHandler(Protocol::Play::PLAYER_BLOCK_PLACEMENT, Protocol::Play::player_block_placement_handler);
			g_NetMan->AddPacketHandler(Protocol::Play::USE_ITEM, Protocol::Play::use_item_handler);
			break;
		}
		}
	}
}