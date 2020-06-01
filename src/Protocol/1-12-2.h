#pragma once
#include <Network/NetworkDriver.h>
#include <Events/Events.h>
#include <Utilities/JSON.h>

using namespace Stardust::Events;
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
			void send_join_game(int eid);
			void send_plugin_message(std::string type);
			void send_server_difficulty();
			void send_player_abilities();
			void send_hotbar_slot(int slot);
			void send_entity_status(int eid, int action);
			void send_player_list_item();
			void send_player_position_look();
			void send_world_border();
			void send_time_update();
			void send_spawn_position();

			void send_keepalive(long long int ll);
			void send_chat(std::string text, std::string color = "default", std::string format = "none");
		}

		namespace PacketEvents {
			struct event_teleport_confirm : public Event {
				int ID;
			};
			struct event_tab_complete : public Event {
				std::string text;
				bool assumeCommand;
				bool hasPosition;
				uint64_t position; //Special position format
			};
			struct event_chat_message : public Event {
				std::string text;
			};
			struct event_client_status : public Event {
				uint8_t actionID;
			};
			struct event_client_settings : public Event {
				std::string locale;
				uint8_t renderDistance;
				uint8_t chatMode;
				bool colors;
				uint8_t skinParts;
				uint8_t mainHand;
			};
			struct event_confirm_transaction : public Event {
				uint8_t window;
				uint16_t action;
				bool accepted;
			};
			struct event_enchant_item : public Event {
				uint8_t window;
				uint8_t enchantment;
			};
			struct event_click_window : public Event {
				uint8_t window;
				int16_t slot;
				uint8_t button;
				int16_t action;
				uint8_t mode;
				uint8_t bytes[];
				//TODO: SLOT DATA NBT
			};
			struct event_close_window : public Event {
				uint8_t window;
			};
			struct event_plugin_message : public Event {
				std::string channel;
				std::vector<byte> data;
			};
			struct event_use_entity : public Event {
				int target;
				uint8_t type;
				float tX, tY, tZ;
				uint8_t hand;
			};
			struct event_keep_alive : public Event {
				uint32_t ID; //I must check if this is 32 bit or 64 - assuming 32
			};
			struct event_player : public Event {
				bool onGround;
			};
			struct event_player_position : public Event {
				double x, y, z;
				bool onGround;
			};
			struct event_player_position_and_look : public Event {
				double x, y, z;
				float yaw, pitch;
				bool onGround;
			};
			struct event_player_look : public Event {
				float yaw, pitch;
				bool onGround;
			};
			struct event_vehicle_move : public Event {
				double x, y, z;
				float yaw, pitch;
			};
			struct event_steer_boat : public Event {
				bool rightTurn, leftTurn;
			};
			struct event_craft_recipe_request : public Event {
				uint8_t window;
				int recipe;
				bool makeAll;
			};
			struct event_player_abilities : public Event {
				uint8_t flags;
				float flySpeed;
				float walkSpeed;
			};
			struct event_player_digging : public Event {
				int status;
				uint64_t position;
				uint8_t face;
			};
			struct event_entity_action : public Event {
				int id;
				uint8_t action;
				uint8_t jumpBoost;
			};
			struct event_steer_vehicle : public Event {
				float sideways;
				float forward;
				uint8_t flags;
			};
			struct event_crafting_book_data : public Event {
				int type;
				int id;
				bool craftBookOpen;
				bool craftFilter;
			};
			struct event_resource_pack_status : public Event {
				uint8_t result;
			};
			struct event_advancement_tab : public Event {
				uint8_t action;
				int tab;
			};
			struct event_held_item_change : public Event {
				uint16_t slot;
			};
			struct event_creative_inventory_action : public Event {
				uint16_t slot;
				uint8_t data[]; //SLOT:: TODO
			};
			struct event_update_sign : public Event {
				uint64_t position;
				std::string line1;
				std::string line2;
				std::string line3;
				std::string line4;
			};
			struct event_animation : public Event {
				uint8_t hand;
			};
			struct event_spectate : public Event {
				uint64_t uuid;
			};
			struct event_player_block_placement : public Event {
				uint64_t position;
				uint8_t face;
				uint8_t hand;
				float x, y, z;
			};
			struct event_use_item : public Event {
				uint8_t hand;
			};
		}
	}
}