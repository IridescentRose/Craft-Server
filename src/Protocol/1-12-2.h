#pragma once
#include <Network/NetworkDriver.h>

using namespace Stardust::Network;

namespace Minecraft::Server::Protocol {
	namespace Handshake {

		enum HandShakePackets {
			HANDSHAKE = 0x00
		};

		int handshake_packet_handler(PacketIn* p);

		namespace PacketsOut {
			//There are none for Handshakes
		}
	}

	namespace Status {
		enum StatusPackets {
			REQUEST	= 0x00,
			PING	= 0x01
		};

		int request_packet_handler(PacketIn* p);
		int ping_packet_handler(PacketIn* p);

		namespace PacketsOut {
			void send_response();
			void send_pong(uint64_t l);
		}
	}

	namespace Login {
		enum LoginPackets {
			LOGIN_START = 0x00
		};

		int login_start_packet_handler(PacketIn* p);

		namespace PacketsOut {
			//TODO: ENCRYPTION!!!
			//TODO: GZ COMPRESSION!!!
			void send_login_success(std::string username);
		}
	}

	namespace Play {
		enum PlayPackets {
			TELEPORT_CONFIRM			= 0x00,
			TAB_COMPLETE				= 0x01,
			CHAT_MESSAGE				= 0x02,
			CLIENT_STATUS				= 0x03,
			CLIENT_SETTINGS				= 0x04,
			CONFIRM_TRANSACTION			= 0x05,
			ENCHANT_ITEM				= 0x06,
			CLICK_WINDOW				= 0x07,
			CLOSE_WINDOW				= 0x08,
			PLUGIN_MESSAGE				= 0x09,
			USE_ENTITY					= 0x0A,
			KEEP_ALIVE					= 0x0B,
			PLAYER						= 0x0C,
			PLAYER_POSITION				= 0x0D,
			PLAYER_POSITION_AND_LOOK	= 0x0E,
			PLAYER_LOOK					= 0x0F,
			VEHICLE_MOVE				= 0x10,
			STEER_BOAT					= 0x11,
			CRAFT_RECIPE_REQUEST		= 0x12,
			PLAYER_ABILITIES			= 0x13,
			PLAYER_DIGGING				= 0x14,
			ENTITY_ACTION				= 0x15,
			STEER_VEHICLE				= 0x16,
			CRAFTING_BOOK_DATA			= 0x17,
			RESOURCE_PACK_STATUS		= 0x18,
			ADVANCEMENT_TAB				= 0x19,
			HELD_ITEM_CHANGE			= 0x1A,
			CREATIVE_INVENTORY_ACTION	= 0x1B,
			UPDATE_SIGN					= 0x1C,
			ANIMATION					= 0x1D,
			SPECTATE					= 0x1E,
			PLAYER_BLOCK_PLACEMENT		= 0x1F,
			USE_ITEM					= 0x20,
		};

		
		int teleport_confirm_handler(PacketIn* p);
		
		int tab_complete_handler(PacketIn* p);
		
		int chat_message_handler(PacketIn* p);
		
		int client_status_handler(PacketIn* p);
		
		int client_settings_handler(PacketIn* p);
		
		int confirm_transaction_handler(PacketIn* p);
		
		int enchant_item_handler(PacketIn* p);
		
		int click_window_handler(PacketIn* p);
		
		int close_window_handler(PacketIn* p);
		
		int plugin_message_handler(PacketIn* p);
		
		int use_entity_handler(PacketIn* p);
		
		int keep_alive_handler(PacketIn* p);
		
		int player_handler(PacketIn* p);
		
		int player_position_handler(PacketIn* p);
		
		int player_position_and_look_handler(PacketIn* p);
		
		int player_look_handler(PacketIn* p);
		
		int vehicle_move_handler(PacketIn* p);
		
		int steer_boat_handler(PacketIn* p);
		
		int craft_recipe_request_handler(PacketIn* p);
		
		int player_abilities_handler(PacketIn* p);
		
		int player_digging_handler(PacketIn* p);
		
		int entity_action_handler(PacketIn* p);
		
		int steer_vehicle_handler(PacketIn* p);
		
		int crafting_book_data_handler(PacketIn* p);
		
		int resource_pack_status_handler(PacketIn* p);
		
		int advancement_tab_handler(PacketIn* p);
		
		int held_item_change_handler(PacketIn* p);
		
		int creative_inventory_action_handler(PacketIn* p);
		
		int update_sign_handler(PacketIn* p);
		
		int animation_handler(PacketIn* p);
		
		int spectate_handler(PacketIn* p);
		
		int player_block_placement_handler(PacketIn* p);
		
		int use_item_handler(PacketIn* p);

		namespace PacketsOut {

		}
	}
}