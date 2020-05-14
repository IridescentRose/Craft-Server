#include "1-12-2.h"
#include "../Networking/NetworkManager2.h"
#include "../Utils.h"
#include <Utilities/JSON.h>
#include "../Config.h"

namespace Minecraft::Server::Protocol {

	int Handshake::handshake_packet_handler(PacketIn* p)
	{
		utilityPrint("HANDSHAKE RECEIVED!", LOGGER_LEVEL_TRACE);

		int getVersion = decodeVarInt(*p);
		utilityPrint("Protocol: " + std::to_string(getVersion), LOGGER_LEVEL_TRACE);

		decodeString(*p);
		decodeShort(*p);

		int nextStatus = decodeVarInt(*p);
		utilityPrint("Connection Switch: " + std::to_string(nextStatus), LOGGER_LEVEL_TRACE);

		g_NetMan->m_Socket->setConnectionStatus(nextStatus);

		return 0;
	}

	int Status::request_packet_handler(PacketIn* p)
	{
		PacketsOut::send_response();
		return 0;
	}

	int Status::ping_packet_handler(PacketIn* p)
	{
		uint64_t l = decodeLong(*p);
		PacketsOut::send_pong(l);
		return 0;
	}

	void Status::PacketsOut::send_response()
	{
		PacketOut* p2 = new PacketOut();
		p2->bytes.clear();
		p2->ID = 0x00;

		std::string st = "{\"description\":{\"text\":\"" + g_Config.motd + "\"},\"players\":{\"max\":" + std::to_string((int)g_Config.max_players) + ",\"online\":0},\"version\":{\"name\":\"1.12.2\",\"protocol\":340}}";

		encodeStringLE(st, *p2);

		g_NetMan->AddPacket(p2);
		g_NetMan->SendPackets();
	}

	void Status::PacketsOut::send_pong(uint64_t l)
	{
		PacketOut* p2 = new PacketOut();
		p2->ID = 0x01;
		encodeLong(l, (*p2));

		g_NetMan->AddPacket(p2);
		g_NetMan->SendPackets();
		g_NetMan->m_Socket->Close();
	}

#include <Utilities/UUID.h>
	using namespace Stardust::Utilities;

	int Login::login_start_packet_handler(PacketIn* p)
	{
		std::string username;

		username = decodeStringLE(*p);

		utilityPrint(username + " is attempting to join", LOGGER_LEVEL_INFO);

		return 0;
	}

	void Login::PacketsOut::send_login_success(std::string username)
	{
		PacketOut* p2 = new PacketOut();
		p2->ID = 0x02;
		encodeStringLE(generateUUID(), *p2);
		encodeStringLE(username, *p2);

		g_NetMan->AddPacket(p2);
		g_NetMan->SendPackets();
	}

	int Play::teleport_confirm_handler(PacketIn* p) { utilityPrint("TELEPORT_CONFIRM Triggered!", LOGGER_LEVEL_WARN); return 0; }
	int Play::tab_complete_handler(PacketIn* p) { utilityPrint("TAB_COMPLETE Triggered!", LOGGER_LEVEL_WARN); return 0; }
	int Play::chat_message_handler(PacketIn* p) { utilityPrint("CHAT_MESSAGE Triggered!", LOGGER_LEVEL_WARN); return 0; }
	int Play::client_status_handler(PacketIn* p) { utilityPrint("CLIENT_STATUS Triggered!", LOGGER_LEVEL_WARN); return 0; }
	int Play::client_settings_handler(PacketIn* p) { utilityPrint("CLIENT_SETTINGS Triggered!", LOGGER_LEVEL_WARN); return 0; }
	int Play::confirm_transaction_handler(PacketIn* p) { utilityPrint("CONFIRM_TRANSACTION Triggered!", LOGGER_LEVEL_WARN); return 0; }
	int Play::enchant_item_handler(PacketIn* p) { utilityPrint("ENCHANT_ITEM Triggered!", LOGGER_LEVEL_WARN); return 0; }
	int Play::click_window_handler(PacketIn* p) { utilityPrint("CLICK_WINDOW Triggered!", LOGGER_LEVEL_WARN); return 0; }
	int Play::close_window_handler(PacketIn* p) { utilityPrint("CLOSE_WINDOW Triggered!", LOGGER_LEVEL_WARN); return 0; }
	int Play::plugin_message_handler(PacketIn* p) { utilityPrint("PLUGIN_MESSAGE Triggered!", LOGGER_LEVEL_WARN); return 0; }
	int Play::use_entity_handler(PacketIn* p) { utilityPrint("USE_ENTITY Triggered!", LOGGER_LEVEL_WARN); return 0; }
	int Play::keep_alive_handler(PacketIn* p) { utilityPrint("KEEP_ALIVE Triggered!", LOGGER_LEVEL_WARN); return 0; }
	int Play::player_handler(PacketIn* p) { utilityPrint("PLAYER Triggered!", LOGGER_LEVEL_WARN); return 0; }
	int Play::player_position_handler(PacketIn* p) { utilityPrint("PLAYER_POSITION Triggered!", LOGGER_LEVEL_WARN); return 0; }
	int Play::player_position_and_look_handler(PacketIn* p) { utilityPrint("PLAYER_POSITION_AND_LOOK Triggered!", LOGGER_LEVEL_WARN); return 0; }
	int Play::player_look_handler(PacketIn* p) { utilityPrint("PLAYER_LOOK Triggered!", LOGGER_LEVEL_WARN); return 0; }
	int Play::vehicle_move_handler(PacketIn* p) { utilityPrint("VEHICLE_MOVE Triggered!", LOGGER_LEVEL_WARN); return 0; }
	int Play::steer_boat_handler(PacketIn* p) { utilityPrint("STEER_BOAT Triggered!", LOGGER_LEVEL_WARN); return 0; }
	int Play::craft_recipe_request_handler(PacketIn* p) { utilityPrint("CRAFT_RECIPE_REQUEST Triggered!", LOGGER_LEVEL_WARN); return 0; }
	int Play::player_abilities_handler(PacketIn* p) { utilityPrint("PLAYER_ABILITIES Triggered!", LOGGER_LEVEL_WARN); return 0; }
	int Play::player_digging_handler(PacketIn* p) { utilityPrint("PLAYER_DIGGING Triggered!", LOGGER_LEVEL_WARN); return 0; }
	int Play::entity_action_handler(PacketIn* p) { utilityPrint("ENTITY_ACTION Triggered!", LOGGER_LEVEL_WARN); return 0; }
	int Play::steer_vehicle_handler(PacketIn* p) { utilityPrint("STEER_VEHICLE Triggered!", LOGGER_LEVEL_WARN); return 0; }
	int Play::crafting_book_data_handler(PacketIn* p) { utilityPrint("CRAFTING_BOOK_DATA Triggered!", LOGGER_LEVEL_WARN); return 0; }
	int Play::resource_pack_status_handler(PacketIn* p) { utilityPrint("RESOURCE_PACK_STATUS Triggered!", LOGGER_LEVEL_WARN); return 0; }
	int Play::advancement_tab_handler(PacketIn* p) { utilityPrint("ADVANCEMENT_TAB Triggered!", LOGGER_LEVEL_WARN); return 0; }
	int Play::held_item_change_handler(PacketIn* p) { utilityPrint("HELD_ITEM_CHANGE Triggered!", LOGGER_LEVEL_WARN); return 0; }
	int Play::creative_inventory_action_handler(PacketIn* p) { utilityPrint("CREATIVE_INVENTORY_ACTION Triggered!", LOGGER_LEVEL_WARN); return 0; }
	int Play::update_sign_handler(PacketIn* p) { utilityPrint("UPDATE_SIGN Triggered!", LOGGER_LEVEL_WARN); return 0; }
	int Play::animation_handler(PacketIn* p) { utilityPrint("ANIMATION Triggered!", LOGGER_LEVEL_WARN); return 0; }
	int Play::spectate_handler(PacketIn* p) { utilityPrint("SPECTATE Triggered!", LOGGER_LEVEL_WARN); return 0; }
	int Play::player_block_placement_handler(PacketIn* p) { utilityPrint("PLAYER_BLOCK_PLACEMENT Triggered!", LOGGER_LEVEL_WARN); return 0; }
	int Play::use_item_handler(PacketIn* p) { utilityPrint("USE_ITEM Triggered!", LOGGER_LEVEL_WARN); return 0; }
}