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

		fcntl(m_Connection, F_SETFL, O_NONBLOCK);

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
		fcntl(m_Connection, F_SETFL, O_NONBLOCK);
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