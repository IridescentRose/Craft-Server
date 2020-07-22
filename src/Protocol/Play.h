#pragma once
#include <Network/NetworkDriver.h>
#include <Events/Events.h>
#include <Utilities/JSON.h>
#include "../Internal/Chunks/ChunkColumn.h"
#include "../Internal/Inventory/Inventory.h"
#include "../Internal/Player/Player.h"
#include "../Internal/InternalServer.h"

#include <Network/NetworkTypes.h>
using namespace Stardust::Events;
using namespace Stardust::Network;

namespace Minecraft::Server::Protocol {
	namespace Play {
		enum PlayPackets {
			TELEPORT_CONFIRM = 0x00,
			QUERY_BLOCK_NBT = 0x01,
			CHAT_MESSAGE = 0x02,
			CLIENT_STATUS = 0x03,
			CLIENT_SETTINGS = 0x04,
			TAB_COMPLETE = 0x05,
			CONFIRM_TRANSACTION = 0x06,
			ENCHANT_ITEM = 0x07,
			CLICK_WINDOW = 0x08,
			CLOSE_WINDOW = 0x09,
			PLUGIN_MESSAGE = 0x0A,
			EDIT_BOOK = 0x0B,
			QUERY_ENTITY_NBT = 0x0C,
			USE_ENTITY = 0x0D,
			KEEP_ALIVE = 0x0E,
			PLAYER = 0x0F,
			PLAYER_POSITION = 0x10,
			PLAYER_POSITION_AND_LOOK = 0x11,
			PLAYER_LOOK = 0x12,
			VEHICLE_MOVE = 0x13,
			STEER_BOAT = 0x14,
			PICK_ITEM = 0x15,
			CRAFT_RECIPE_REQUEST = 0x16,
			PLAYER_ABILITIES = 0x17,
			PLAYER_DIGGING = 0x18,
			ENTITY_ACTION = 0x19,
			STEER_VEHICLE = 0x1A,
			RECIPE_BOOK_DATA = 0x1B,
			NAME_ITEM = 0x1C,
			RESOURCE_PACK_STATUS = 0x1D,
			ADVANCEMENT_TAB = 0x1E,
			SELECT_TRADE = 0x1F,
			SET_BEACON_EFFECT = 0x20,
			HELD_ITEM_CHANGE = 0x21,
			UPDATE_COMMAND_BLOCK = 0x22,
			UPDATE_COMMAND_BLOCK_MINECART = 0x23,
			CREATIVE_INVENTORY_ACTION = 0x24,
			UPDATE_STRUCTURE_BLOCK = 0x25,
			UPDATE_SIGN = 0x26,
			ANIMATION = 0x27,
			SPECTATE = 0x28,
			PLAYER_BLOCK_PLACEMENT = 0x29,
			USE_ITEM = 0x2A,
		};

		int teleport_confirm_handler(PacketIn* p);
		int query_block_nbt_handler(PacketIn* p);
		int chat_message_handler(PacketIn* p);
		int client_status_handler(PacketIn* p);
		int client_settings_handler(PacketIn* p);
		int tab_complete_handler(PacketIn* p);
		int click_window_handler(PacketIn* p);
		int close_window_handler(PacketIn* p);
		int plugin_message_handler(PacketIn* p);
		int edit_book_handler(PacketIn* p);
		int keep_alive_handler(PacketIn* p);
		int player_handler(PacketIn* p);
		int player_position_handler(PacketIn* p);
		int player_position_and_look_handler(PacketIn* p);
		int player_look_handler(PacketIn* p);
		int vehicle_move_handler(PacketIn* p);
		int steer_boat_handler(PacketIn* p);
		int pick_item_handler(PacketIn* p);
		int craft_recipe_request_handler(PacketIn* p);
		int player_abilities_handler(PacketIn* p);
		int player_digging_handler(PacketIn* p);
		int entity_action_handler(PacketIn* p);
		int steer_vehicle_handler(PacketIn* p);
		int recipe_book_data_handler(PacketIn* p);
		int name_item_handler(PacketIn* p);
		int resource_pack_status_handler(PacketIn* p);
		int advancement_tab_handler(PacketIn* p);
		int select_trade_handler(PacketIn* p);
		int set_beacon_effect_handler(PacketIn* p);
		int held_item_change_handler(PacketIn* p);
		int update_command_block_handler(PacketIn* p);
		int update_command_block_minecart_handler(PacketIn* p);
		int creative_inventory_action_handler(PacketIn* p);
		int update_structure_block_handler(PacketIn* p);
		int update_sign_handler(PacketIn* p);
		int animation_handler(PacketIn* p);
		int spectate_handler(PacketIn* p);
		int player_block_placement_handler(PacketIn* p);
		int use_item_handler(PacketIn* p);
		int confirm_transaction_handler(PacketIn* p);
		int enchant_item_handler(PacketIn* p);
		int query_entity_nbt_handler(PacketIn* p);
		int use_entity_handler(PacketIn* p);
		
		namespace PacketsOut {
			void send_join_game(int eid);
			void send_plugin_message(std::string type);
			void send_server_difficulty();
			void send_player_abilities();
			void send_hotbar_slot(int slot);
			void send_entity_status(int eid, int action);
			void send_player_info();
			void send_player_position_look();
			void send_world_border();
			void send_time_update();
			void send_time_update2(Internal::TimeDataStruct data);
			void send_spawn_position();

			void send_keepalive(long long int ll);
			void send_chat(std::string text, std::string color = "default", std::string format = "none", bool serverChat = false);
			void send_chat_command(std::string text);

			void send_disconnect(std::string reason, std::string color = "gold");

			void send_change_gamestate(uint8_t code, float value);
			
			void send_chunk(Internal::Chunks::ChunkColumn* chnkc, bool first);
			void send_unload_chunk(int x, int z);
			void send_initial_inventory();

			void send_set_slot(uint8_t id, uint16_t num, Internal::Inventory::Slot* slt);
			void send_change_block(int x, int y, int z, BlockID id);
		}

		namespace PacketEvents {
			struct event_tab_complete : public Event {
				std::string text;
				bool assumeCommand;
				bool hasPosition;
				uint64_t position; //Special position format
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