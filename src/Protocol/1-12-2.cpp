#include "1-12-2.h"
#include "../Networking/NetworkManager2.h"
#include "../Utilities/Utils.h"
#include <Utilities/JSON.h>
#include "../Utilities/Config.h"

#include <Utilities/UUID.h>
#include "../Internal/InternalServer.h"
#include <dirent.h>

namespace Minecraft::Server::Protocol {

	int Handshake::handshake_packet_handler(PacketIn* p)
	{

		uint32_t getVersion;
		p->buffer->ReadVarInt32(getVersion);
		
		std::string ip;
		p->buffer->ReadVarUTF8String(ip);

		uint16_t port;
		p->buffer->ReadBEUInt16(port);

		uint8_t state;
		p->buffer->ReadBEUInt8(state);
		
		g_NetMan->m_Socket->setConnectionStatus(state);


		utilityPrint(std::to_string(getVersion), LOGGER_LEVEL_DEBUG);
		utilityPrint(ip, LOGGER_LEVEL_DEBUG);
		utilityPrint(std::to_string(port), LOGGER_LEVEL_DEBUG);
		utilityPrint(std::to_string((int)state), LOGGER_LEVEL_DEBUG);

		return 0;
	}

	int Status::request_packet_handler(PacketIn* p)
	{
		PacketsOut::send_response();
		return 0;
	}

	int Status::ping_packet_handler(PacketIn* p)
	{
		uint64_t l;
		p->buffer->ReadBEUInt64(l);

		PacketsOut::send_pong(l);
		return 0;
	}

	void Status::PacketsOut::send_response()
	{
		PacketOut* p2 = new PacketOut(512);
		p2->ID = 0x00;

		std::string st = "{\"description\":{\"text\":\"" + g_Config.motd + "\"},\"players\":{\"max\":" + std::to_string((int)g_Config.max_players) + ",\"online\":0},\"version\":{\"name\":\"1.12.2\",\"protocol\":340}}";
		p2->buffer->WriteVarUTF8String(st);


		g_NetMan->AddPacket(p2);
		g_NetMan->SendPackets();
	}

	void Status::PacketsOut::send_pong(uint64_t l)
	{
		PacketOut* p2 = new PacketOut(9);
		p2->ID = 0x01;

		p2->buffer->WriteBEUInt64(l);

		g_NetMan->AddPacket(p2);
		g_NetMan->SendPackets();
		g_NetMan->m_Socket->Close();
	}

	using namespace Stardust::Utilities;

	Json::Value playerJSON;
	Json::Value banned;
	bool downfall;

	int Login::login_start_packet_handler(PacketIn* p)
	{
		downfall = false;

		p->buffer->ReadVarUTF8String(Internal::Player::g_Player.username);

		utilityPrint(Internal::Player::g_Player.username + " is attempting to join", LOGGER_LEVEL_INFO);


		banned = Utilities::JSON::openJSON("banned.json");
		//BEGIN LOGIN SEQUENCE HERE!

		

		//Check if they're new.
		DIR* dir;
		struct dirent* ent;
		bool found = false;
		if ((dir = opendir("./playerdata")) != NULL) {
			while ((ent = readdir(dir)) != NULL) {
				if (Internal::Player::g_Player.username + ".json" == std::string(ent->d_name)) {
					found = true;
					break;
				}
			}
			closedir(dir);


			if (found) {
				utilityPrint("Found Player Profile", LOGGER_LEVEL_TRACE);
				//Load a JSON for their stats.
				playerJSON = Utilities::JSON::openJSON("./playerdata/" + std::string(ent->d_name));
				Internal::Player::g_Player.uuid = playerJSON["uuid"].asString();
				Internal::Player::g_Player.operatorLevel = playerJSON["oplevel"].asInt();
			} else {
				utilityPrint("Player profile not found. Generating new.", LOGGER_LEVEL_TRACE);
				//Create a default one.
				Internal::Player::g_Player.uuid = generateUUID();
				Internal::Player::g_Player.operatorLevel = 0;
				playerJSON["uuid"] = Internal::Player::g_Player.uuid;
				playerJSON["oplevel"] = (int)Internal::Player::g_Player.operatorLevel;

				std::ofstream fs("./playerdata/" + Internal::Player::g_Player.username + ".json");
				fs << playerJSON;
				fs.close();
			}

		}
		

		PacketsOut::send_login_success(Internal::Player::g_Player.username);
		g_NetMan->m_Socket->setConnectionStatus(CONNECTION_STATE_PLAY);
		utilityPrint("Dumping Packet Load!", LOGGER_LEVEL_DEBUG);

		int eid = 1337;

		Play::PacketsOut::send_join_game(eid);

		//Check bans
		for (int i = 0; i < banned.size(); i++) {
			if (banned[i].asString() == Internal::Player::g_Player.username || banned[i].asString() == "all") {
				//They're banned! Don't connect!.
				Play::PacketsOut::send_disconnect("You are banned!", "dark_red");
				return -1;
			}
		}

		Play::PacketsOut::send_plugin_message("MC|Brand");
		Play::PacketsOut::send_server_difficulty();
		Play::PacketsOut::send_player_abilities();
		Play::PacketsOut::send_hotbar_slot(0); //Slot 0
		Play::PacketsOut::send_entity_status(eid, 24 + Internal::Player::g_Player.operatorLevel);
		Play::PacketsOut::send_player_list_item();


		sceKernelDelayThread(100 * 1000);

		Play::PacketsOut::send_player_position_look();
		Play::PacketsOut::send_world_border();
		Play::PacketsOut::send_time_update();
		Play::PacketsOut::send_spawn_position();

		if (downfall) {
			Play::PacketsOut::send_change_gamestate(2, 0.0f);
		}

		return 0;
	}
	void Login::PacketsOut::send_login_success(std::string username)
	{
		PacketOut* p2 = new PacketOut(80);
		p2->ID = 0x02;
		p2->buffer->WriteVarUTF8String(Internal::Player::g_Player.uuid);
		p2->buffer->WriteVarUTF8String(username);

		g_NetMan->AddPacket(p2);
		g_NetMan->SendPackets();
	}

	int Play::teleport_confirm_handler(PacketIn* p) { 
		//This can only be triggered once anyways.
		utilityPrint("TP Confirmed", LOGGER_LEVEL_DEBUG); 
		return 0; 
	}

	int Play::tab_complete_handler(PacketIn* p) { utilityPrint("TAB_COMPLETE Triggered!", LOGGER_LEVEL_WARN); return 0; }
	
	int Play::chat_message_handler(PacketIn* p) { 
		std::string text;
		p->buffer->ReadVarUTF8String(text);

		if(text.at(0) != '/'){
			utilityPrint(Internal::Player::g_Player.username + ": " + text, LOGGER_LEVEL_INFO);
			PacketsOut::send_chat(text);
		}
		else {
			PacketsOut::send_chat_command(text);
		}

		return 0; 
	}
	
	int Play::client_status_handler(PacketIn* p) { utilityPrint("CLIENT_STATUS Triggered!", LOGGER_LEVEL_WARN); return 0; }
	
	int Play::client_settings_handler(PacketIn* p) { 
		utilityPrint("Client Settings", LOGGER_LEVEL_TRACE);

		std::string locale;
		p->buffer->ReadVarUTF8String(locale);

		uint8_t renderDistance;
		p->buffer->ReadBEUInt8(renderDistance);
		uint8_t chatMode;
		p->buffer->ReadBEUInt8(chatMode);
		bool colors;
		p->buffer->ReadBool(colors);
		uint8_t displayed;
		p->buffer->ReadBEUInt8(displayed);
		uint8_t mainhand;
		p->buffer->ReadBEUInt8(mainhand);

		utilityPrint("Locale: " + locale, LOGGER_LEVEL_TRACE);
		utilityPrint("Render: " + std::to_string((int)renderDistance), LOGGER_LEVEL_TRACE);
		utilityPrint("Chat: " + std::to_string((int)chatMode), LOGGER_LEVEL_TRACE);
		utilityPrint("Colors: " + std::to_string(colors), LOGGER_LEVEL_TRACE);
		utilityPrint("Main Hand: " + std::to_string((int)mainhand), LOGGER_LEVEL_TRACE);

		return 0; 
	}
	
	int Play::confirm_transaction_handler(PacketIn* p) { utilityPrint("CONFIRM_TRANSACTION Triggered!", LOGGER_LEVEL_WARN); return 0; }
	int Play::enchant_item_handler(PacketIn* p) { utilityPrint("ENCHANT_ITEM Triggered!", LOGGER_LEVEL_WARN); return 0; }
	int Play::click_window_handler(PacketIn* p) { utilityPrint("CLICK_WINDOW Triggered!", LOGGER_LEVEL_WARN); return 0; }
	int Play::close_window_handler(PacketIn* p) { utilityPrint("CLOSE_WINDOW Triggered!", LOGGER_LEVEL_WARN); return 0; }
	
	int Play::plugin_message_handler(PacketIn* p) { 
		utilityPrint("Plugin Message", LOGGER_LEVEL_TRACE); 

		std::string channel;
		p->buffer->ReadVarUTF8String(channel);
		std::string data;
		p->buffer->ReadVarUTF8String(data);
		utilityPrint("Channel: " + channel, LOGGER_LEVEL_TRACE);
		utilityPrint("Data: " + data, LOGGER_LEVEL_TRACE);

		return 0; 
	}
	
	int Play::use_entity_handler(PacketIn* p) { utilityPrint("USE_ENTITY Triggered!", LOGGER_LEVEL_WARN); return 0; }
	
	int Play::keep_alive_handler(PacketIn* p) { 
		return 0; 
	}
	
	int Play::player_handler(PacketIn* p) {
		p->buffer->ReadBool(Internal::Player::g_Player.onGround);

		return 0; 
	}
	
	int Play::player_position_handler(PacketIn* p) { 
		p->buffer->ReadBEDouble(Internal::Player::g_Player.x);
		p->buffer->ReadBEDouble(Internal::Player::g_Player.y);
		p->buffer->ReadBEDouble(Internal::Player::g_Player.z);
		p->buffer->ReadBool(Internal::Player::g_Player.onGround);

		return 0; 
	}
	
	int Play::player_position_and_look_handler(PacketIn* p) { 

		p->buffer->ReadBEDouble(Internal::Player::g_Player.x);
		p->buffer->ReadBEDouble(Internal::Player::g_Player.y);
		p->buffer->ReadBEDouble(Internal::Player::g_Player.z);
		p->buffer->ReadBEFloat(Internal::Player::g_Player.yaw);
		p->buffer->ReadBEFloat(Internal::Player::g_Player.pitch);
		p->buffer->ReadBool(Internal::Player::g_Player.onGround);


		return 0; 
	}
	
	int Play::player_look_handler(PacketIn* p) { 

		p->buffer->ReadBEFloat(Internal::Player::g_Player.yaw);
		p->buffer->ReadBEFloat(Internal::Player::g_Player.pitch);
		p->buffer->ReadBool(Internal::Player::g_Player.onGround);

		return 0; 
	}

	int Play::vehicle_move_handler(PacketIn* p) { utilityPrint("VEHICLE_MOVE Triggered!", LOGGER_LEVEL_WARN); return 0; }
	int Play::steer_boat_handler(PacketIn* p) { utilityPrint("STEER_BOAT Triggered!", LOGGER_LEVEL_WARN); return 0; }
	int Play::craft_recipe_request_handler(PacketIn* p) { utilityPrint("CRAFT_RECIPE_REQUEST Triggered!", LOGGER_LEVEL_WARN); return 0; }
	int Play::player_abilities_handler(PacketIn* p) { utilityPrint("PLAYER_ABILITIES Triggered!", LOGGER_LEVEL_WARN); return 0; }
	int Play::player_digging_handler(PacketIn* p) { utilityPrint("PLAYER_DIGGING Triggered!", LOGGER_LEVEL_WARN); return 0; }
	int Play::entity_action_handler(PacketIn* p) { utilityPrint("ENTITY_ACTION Triggered!", LOGGER_LEVEL_WARN); return 0; }
	int Play::steer_vehicle_handler(PacketIn* p) { utilityPrint("STEER_VEHICLE Triggered!", LOGGER_LEVEL_WARN); return 0; }
	
	int Play::crafting_book_data_handler(PacketIn* p) { 
		utilityPrint("CRAFTING_BOOK_DATA Triggered!", LOGGER_LEVEL_TRACE);
		//TODO: IMPL
		PacketsOut::send_chat("Crafting book is not yet implemented...", "gold", "none", true);
		return 0;
	}

	int Play::resource_pack_status_handler(PacketIn* p) { utilityPrint("RESOURCE_PACK_STATUS Triggered!", LOGGER_LEVEL_WARN); return 0; }
	int Play::advancement_tab_handler(PacketIn* p) { utilityPrint("ADVANCEMENT_TAB Triggered!", LOGGER_LEVEL_WARN); return 0; }
	int Play::held_item_change_handler(PacketIn* p) { utilityPrint("HELD_ITEM_CHANGE Triggered!", LOGGER_LEVEL_WARN); return 0; }
	int Play::creative_inventory_action_handler(PacketIn* p) { utilityPrint("CREATIVE_INVENTORY_ACTION Triggered!", LOGGER_LEVEL_WARN); return 0; }
	int Play::update_sign_handler(PacketIn* p) { utilityPrint("UPDATE_SIGN Triggered!", LOGGER_LEVEL_WARN); return 0; }
	
	int Play::animation_handler(PacketIn* p) { 
		//TODO: Do something about this for multiplayer. Otherwise I don't care.
		//utilityPrint("ANIMATION Triggered!", LOGGER_LEVEL_WARN); 
		return 0; 
	}
	
	int Play::spectate_handler(PacketIn* p) { utilityPrint("SPECTATE Triggered!", LOGGER_LEVEL_WARN); return 0; }
	int Play::player_block_placement_handler(PacketIn* p) { utilityPrint("PLAYER_BLOCK_PLACEMENT Triggered!", LOGGER_LEVEL_WARN); return 0; }
	int Play::use_item_handler(PacketIn* p) { utilityPrint("USE_ITEM Triggered!", LOGGER_LEVEL_WARN); return 0; }
}

void Minecraft::Server::Protocol::Play::PacketsOut::send_join_game(int eid)
{
	PacketOut* p = new PacketOut(90);

	p->ID = 0x23;
	p->buffer->WriteBEInt32(eid);
	p->buffer->WriteBEUInt8(g_Config.gamemode);
	p->buffer->WriteBEInt32(0); //DIMENSION;
	p->buffer->WriteBEUInt8(g_Config.difficulty);
	p->buffer->WriteBEUInt8(1); //Max players, useless
	std::string lvl = "default";
	p->buffer->WriteVarUTF8String(lvl); //Level type
	p->buffer->WriteBool(false); //Reduce debug info? NONONONONO!

	g_NetMan->AddPacket(p);
	g_NetMan->SendPackets();
}

void Minecraft::Server::Protocol::Play::PacketsOut::send_plugin_message(std::string type)
{
	if (type == "MC|Brand") {
		PacketOut* p = new PacketOut(512);
		p->ID = 0x18;
		p->buffer->WriteVarUTF8String(type);
		std::string serverID = "PSP-Craft";
		p->buffer->WriteVarUTF8String(serverID);

		g_NetMan->AddPacket(p);
		g_NetMan->SendPackets();
	}
}

void Minecraft::Server::Protocol::Play::PacketsOut::send_server_difficulty()
{
	PacketOut* p = new PacketOut(12);
	p->ID = 0x0D;
	p->buffer->WriteBEUInt8(g_Config.difficulty);

	g_NetMan->AddPacket(p);
	g_NetMan->SendPackets();
}

void Minecraft::Server::Protocol::Play::PacketsOut::send_player_abilities()
{
	PacketOut* p = new PacketOut(12);
	p->ID = 0x2C;
	p->buffer->WriteBEUInt8(0);
	p->buffer->WriteBEFloat(0.5f);
	p->buffer->WriteBEFloat(0.1f);

	g_NetMan->AddPacket(p);
	g_NetMan->SendPackets();
}

void Minecraft::Server::Protocol::Play::PacketsOut::send_hotbar_slot(int slot)
{
	PacketOut* p = new PacketOut(12);
	p->ID = 0x3A;
	p->buffer->WriteBEUInt8(0);

	g_NetMan->AddPacket(p);
	g_NetMan->SendPackets();
}

void Minecraft::Server::Protocol::Play::PacketsOut::send_entity_status(int eid, int action)
{
	PacketOut* p = new PacketOut(12);
	p->ID = 0x1B;
	p->buffer->WriteBEInt32(eid);
	p->buffer->WriteBEUInt8(action);

	g_NetMan->AddPacket(p);
	g_NetMan->SendPackets();
}

void Minecraft::Server::Protocol::Play::PacketsOut::send_player_list_item()
{
}

void Minecraft::Server::Protocol::Play::PacketsOut::send_player_position_look()
{

	PacketOut* p = new PacketOut(64);
	p->ID = 0x2F;
	p->buffer->WriteBEDouble(0);
	p->buffer->WriteBEDouble(16);
	p->buffer->WriteBEDouble(0);
	p->buffer->WriteBEFloat(0);
	p->buffer->WriteBEFloat(0);

	p->buffer->WriteBEUInt8(0);
	p->buffer->WriteBEUInt8(1);//TP ID

	g_NetMan->AddPacket(p);
	g_NetMan->SendPackets();
}

void Minecraft::Server::Protocol::Play::PacketsOut::send_world_border()
{
	PacketOut* p = new PacketOut(80);
	p->ID = 0x38;
	p->buffer->WriteBEUInt8(3);
	//INIT
	p->buffer->WriteBEDouble(0);
	p->buffer->WriteBEDouble(0);

	p->buffer->WriteBEDouble(60000000.0);
	p->buffer->WriteBEDouble(60000000.0);

	p->buffer->WriteBEUInt8(0);
	p->buffer->WriteVarInt32(29999984);

	p->buffer->WriteBEUInt8(5);
	p->buffer->WriteBEUInt8(15);

	g_NetMan->AddPacket(p);
	g_NetMan->SendPackets();
}

void Minecraft::Server::Protocol::Play::PacketsOut::send_time_update()
{
	PacketOut* p = new PacketOut(17);
	p->ID = 0x47;
	p->buffer->WriteBEInt64(0);
	p->buffer->WriteBEInt64(0);

	g_NetMan->AddPacket(p);
	g_NetMan->SendPackets();
}

void Minecraft::Server::Protocol::Play::PacketsOut::send_spawn_position()
{
	PacketOut* p = new PacketOut(10);
	p->ID = 0x46;
	p->buffer->WriteBEInt64((((long long)0 & 0x3FFFFFFLL) << 38) | ((0 & 0x3FFFFFF) << 12) | (63 & 0xFFF));

	g_NetMan->AddPacket(p);
	g_NetMan->SendPackets();
}

void Minecraft::Server::Protocol::Play::PacketsOut::send_keepalive(long long int ll)
{
	PacketOut* p = new PacketOut(9);
	p->ID = 0x1F;
	p->buffer->WriteBEInt64(ll);

	g_NetMan->AddPacket(p);
	g_NetMan->SendPackets();
}

void Minecraft::Server::Protocol::Play::PacketsOut::send_chat(std::string text, std::string color, std::string format, bool serverChat)
{
	PacketOut* p = new PacketOut(1024);
	p->ID = 0x0F;
	std::string build;
	if (serverChat) {
		build = "{\"text\":\"" + text + "\"";
	}
	else {
		build = "{\"translate\":\"chat.type.text\",\"with\":[{\"text\":\"" + Internal::Player::g_Player.username + "\",\"clickEvent\":{\"action\":\"suggest_command\",\"value\":\"/msg " + Internal::Player::g_Player.username + " \"},\"hoverEvent\":{\"action\":\"show_entity\",\"value\":\"{id:" + Internal::Player::g_Player.uuid + ",name:" + Internal::Player::g_Player.username + "}\"},\"insertion\":\"" + Internal::Player::g_Player.username + "\"},{\"text\":\"" + text + "\"";
	}

	if (color != "default") {
		build += ",\"color\":\"" + color + "\"";
	}

	if (format != "none") {
		build += ",\"" + format + "\":\"true\"";
	}

	build += "}]} ";
	
	p->buffer->WriteVarUTF8String(build);

	g_NetMan->AddPacket(p);
	g_NetMan->SendPackets();
}

void Minecraft::Server::Protocol::Play::PacketsOut::send_chat_command(std::string text)
{
	bool err = true;
	std::string response;
	if (text == "/help") {
		if (Internal::Player::g_Player.operatorLevel < 1) {
			response = "Currently there are no in-game player-level commands available to you. Become an operator to get more.";
			err = false;
		}
		else if(Internal::Player::g_Player.operatorLevel < 1){
			response = "Operator Commands:\n";
			response += "/gamemode <mode> - Sets your gamemode. (Currently bugged).\n/toggledownfall - Set weather to rain";

			if (Internal::Player::g_Player.operatorLevel == 3) {
				response += "\n/ban <player> - Ban & Kick a player\n/unban <player> - Pardon them\n/kick <player> - Kick a player";
				if (Internal::Player::g_Player.operatorLevel == 4) {
					response += "\n/stop - Stop the server.\n/op <player> - Make the player an admin\n/deop <player> - Demote a player";
				}
			}
			err = false;
		}
	}
	else if (text == "/stop") {
		if (Internal::Player::g_Player.operatorLevel != 4) {
			response = "You do not have adequate permissions.";
		}
		else {
			response = "Shutting Down Server!";
		}
	}
	else if (text.substr(0, 3) == "/op") {
		if (Internal::Player::g_Player.operatorLevel == 4) {
			response = "You do not have adequate permissions.";
		}
		else {
			std::string opUser = text.substr(4, text.length());

			//Check if they're new.
			DIR* dir;
			struct dirent* ent;
			bool found = false;
			if ((dir = opendir("./playerdata")) != NULL) {
				while ((ent = readdir(dir)) != NULL) {
					if (opUser + ".json" == std::string(ent->d_name)) {
						found = true;
						break;
					}
				}
				closedir(dir);

				Json::Value pj;

				if (found) {
					//Load a JSON for their stats.
					pj = Utilities::JSON::openJSON("./playerdata/" + std::string(ent->d_name));
					pj["oplevel"] = 3;

					std::ofstream fs("./playerdata/" + opUser + ".json");
					fs << pj;
					fs.close();
				}
				else {
					//Create a default one.
					pj["uuid"] = generateUUID();
					pj["oplevel"] = 3;

					std::ofstream fs("./playerdata/" + opUser + ".json");
					fs << pj;
					fs.close();
				}
				response = "Player " + opUser + " has been oped.";
			}

		}
	}
	else if (text.substr(0, 5) == "/deop") {
		if (Internal::Player::g_Player.operatorLevel == 4) {
			response = "You do not have adequate permissions.";
		}
		else {
			std::string opUser = text.substr(6, text.length());

			//Check if they're new.
			DIR* dir;
			struct dirent* ent;
			bool found = false;
			if ((dir = opendir("./playerdata")) != NULL) {
				while ((ent = readdir(dir)) != NULL) {
					if (opUser + ".json" == std::string(ent->d_name)) {
						found = true;
						break;
					}
				}
				closedir(dir);


				Json::Value pj;

				if (found) {
					//Load a JSON for their stats.
					pj = Utilities::JSON::openJSON("./playerdata/" + std::string(ent->d_name));
					pj["oplevel"] = 0;

					std::ofstream fs("./playerdata/" + opUser + ".json");
					fs << pj;
					fs.close();
				}
				else {
					//Create a default one.
					pj["uuid"] = generateUUID();
					pj["oplevel"] = 0;

					std::ofstream fs("./playerdata/" + opUser + ".json");
					fs << pj;
					fs.close();
				}
				response = "Player " + opUser + " has been deoped.";
			}

		}
	}
	else if (text.substr(0, 4) == "/ban") {
		if (Internal::Player::g_Player.operatorLevel < 3) {
			response = "You do not have adequate permissions.";
		}
		else {
			banned.append(text.substr(5, text.length()));
			std::ofstream fs("./banned.json");
			fs << banned;
			fs.close();
			response = "Banned " + text.substr(5, text.length());

			if (Internal::Player::g_Player.username == text.substr(5, text.length())) {
				PacketsOut::send_disconnect("You were banned!", "dark_red");
			}
		}
	}
	else if (text.substr(0, 6) == "/unban") {
		if (Internal::Player::g_Player.operatorLevel < 3) {
			response = "You do not have adequate permissions.";
		}
		else {
			int idx = -1;
			for (int i = 0; i < banned.size(); i++) {
				if (banned[i].asString() == text.substr(7, text.length())) {
					idx = i;
				}
			}

			if (idx != -1) {
				Json::Value r;
				banned.removeIndex(idx, &r);

				std::ofstream fs("./banned.json");
				fs << banned;
				fs.close();

				response = "Unbanned " + text.substr(7, text.length());
			}
			else {
				response = "Could not find player " + text.substr(7, text.length());
			}
		}
	}
	else if (text.substr(0, 5) == "/kick") {
		if (Internal::Player::g_Player.operatorLevel < 1) {
			response = "You do not have adequate permissions.";
		}else{
			if (Internal::Player::g_Player.username == text.substr(6, text.length())) {
				PacketsOut::send_disconnect("You were kicked!", "green");
			}
			response = "Kicked player.";
		}
	}

	else if (text.substr(0, 4) == "/say") {
		if (Internal::Player::g_Player.operatorLevel < 1) {
			response = "You do not have adequate permissions.";
		}
		else {
			response = "[Server]: " + text.substr(4, text.length());
		}
	}
	else if (text == "/toggledownfall") {
		if (Internal::Player::g_Player.operatorLevel < 1) {
			response = "You do not have adequate permissions.";
		}
		else {
			response = "Toggling downfall.";
			downfall = !downfall;

			if (downfall) {
				PacketsOut::send_change_gamestate(2, 0.0f);
			}
			else {
				PacketsOut::send_change_gamestate(1, 0.0f);
			}
		}
	}
	else if (text.substr(0, 9) == "/gamemode") {
		if (Internal::Player::g_Player.operatorLevel < 1) {
			response = "You do not have adequate permissions.";
		}
		else {
			int gmChange = g_Config.gamemode;
			if (text.substr(10, text.length()) == "creative") {
				gmChange = 1;
			}else if (text.substr(10, text.length()) == "survival") {
				gmChange = 0;
			}else if (text.substr(10, text.length()) == "spectator") {
				gmChange = 3;
			}else if (text.substr(10, text.length()) == "adventure") {
				gmChange = 2;
			}
			else {
				//Do nothing.
				response = "No such gamemode";
				return;
			}

			if (gmChange != g_Config.gamemode) {
				PacketsOut::send_change_gamestate(3, gmChange);
				response = "Gamemode changed!";
			}
		}
	}
	else if (text == "/fuck") {
		if (Internal::Player::g_Player.operatorLevel < 4) {
			response = "You do not have adequate permissions.";
		}
		else {
			banned.append("all");
			std::ofstream fs("./banned.json");
			fs << banned;
			fs.close();
			response = "Banned " + Internal::Player::g_Player.username;
			PacketsOut::send_disconnect("You were banned!", "dark_red");
			Internal::g_InternalServer->stop();
			sceKernelExitGame();
		}
	}
	else {
		response = "Unrecognized Command!";
	}

	PacketOut* p = new PacketOut(1024);
	p->ID = 0x0F;
	std::string build = "{\"text\":\"" + response + "\"";
	if (!err) {
		build += ",\"color\":\"green\"";
	}
	else {
		build += ",\"color\":\"dark_red\"";
	}


	build += "} ";

	p->buffer->WriteVarUTF8String(build);

	g_NetMan->AddPacket(p);
	g_NetMan->SendPackets();

	if (text == "/stop" && Internal::Player::g_Player.operatorLevel == 4) {
		PacketsOut::send_disconnect("Server is stopping.");
		Internal::g_InternalServer->stop();
		sceKernelExitGame();
	}
}

void Minecraft::Server::Protocol::Play::PacketsOut::send_disconnect(std::string reason, std::string color)
{

	//Send a disconnect
	PacketOut* p2 = new PacketOut(80);
	p2->ID = 0x1A;

	std::string build = "{\"text\":\"" + reason + "\",\"color\":\"" + color + "\"}";
	p2->buffer->WriteVarUTF8String(build);
	g_NetMan->AddPacket(p2);
	g_NetMan->SendPackets();
	g_NetMan->m_Socket->Close();


}

void Minecraft::Server::Protocol::Play::PacketsOut::send_change_gamestate(uint8_t code, float value)
{
	PacketOut* p = new PacketOut(80);
	p->ID = 0x1E;

	p->buffer->WriteBEUInt8(code);
	p->buffer->WriteBEFloat(value);

	g_NetMan->AddPacket(p);
	g_NetMan->SendPackets();
}

void Minecraft::Server::Protocol::Play::PacketsOut::send_demo_chunk(int xx, int zz)
{
	Internal::Chunks::ChunkColumn* chnkc = new Internal::Chunks::ChunkColumn(xx, zz);
	Internal::Chunks::ChunkSection* chnks = new Internal::Chunks::ChunkSection(0);
	chnks->generateTestData();
	chnkc->addSection(chnks);

	PacketOut* p = new PacketOut(512 KiB);
	p->ID = 0x20;

	p->buffer->WriteBEInt32(chnkc->getX());
	p->buffer->WriteBEInt32(chnkc->getZ());

	p->buffer->WriteBool(true);

	int mask = 0;
	int numSections = 0;
	std::vector<Internal::Chunks::ChunkSection*> sections;
	sections.clear();

	for (int i = 0; i < 16; i++) {
		Internal::Chunks::ChunkSection* cs = chnkc->getSection(i);
		if (cs != NULL && !cs->isEmpty()) {
			mask |= (1 << i);
			numSections++;
			sections.push_back(cs);
		}
	}
	
	p->buffer->WriteVarInt32(mask);

	std::vector<Network::byte> byteBuffer;
	byteBuffer.clear();

	std::vector<Network::byte> chunkSecBuffer;
	chunkSecBuffer.clear();

	byteBuffer.push_back(4);
	byteBuffer.push_back(1);
	byteBuffer.push_back(0b10000);

	//FILL OUT CHUNK SECTION BUFFER
	for (int x = 0; x < 16; x += 2) {
		for (int y = 0; y < 16; y++) {
			for (int z = 0; z < 16; z++) {
				chunkSecBuffer.push_back(0);
			}
		}
	}

	int value = chunkSecBuffer.size();
	while (value > 127) {
		byteBuffer.push_back(((byte)(value & 127)) | 128);

		value >>= 7;
	}
	byteBuffer.push_back((byte)value & 127);

	for (auto& b : chunkSecBuffer) {
		byteBuffer.push_back(b);
	}

	for (int x = 0; x < 16; x += 2) {
		for (int y = 0; y < 16; y++) {
			for (int z = 0; z < 16; z++) {
				byteBuffer.push_back(0x0);
			}
		}
	}

	for (int x = 0; x < 16; x += 2) {
		for (int y = 0; y < 16; y++) {
			for (int z = 0; z < 16; z++) {
				byteBuffer.push_back(0xFF);
			}
		}
	}

	for (int x = 0; x < 16; x++) {
		for (int z = 0; z < 16; z++) {
			byteBuffer.push_back(chnkc->getBiomeAt(x, z));
		}
	}
	int dataBufferSize = byteBuffer.size();

	p->buffer->WriteVarInt32(dataBufferSize);
	for (auto& b : byteBuffer) {
		p->buffer->WriteBEUInt8(b);
	}

	p->buffer->WriteBEUInt8(0);

	g_NetMan->AddPacket(p);
	g_NetMan->SendPackets();

	delete chnkc;
	delete chnks;
}


void Minecraft::Server::Protocol::Play::PacketsOut::send_chunk(Internal::Chunks::ChunkColumn* chnkc, bool first)
{
	PacketOut* p = new PacketOut(512 KiB);
	p->ID = 0x20; //CHUNK

	p->buffer->WriteBEInt32(chnkc->getX()); //Coord
	p->buffer->WriteBEInt32(chnkc->getZ()); //Coord

	p->buffer->WriteBool(first); //Ground up continuous
	
	int mask = 0;
	int numSections = 0;
	std::vector<Internal::Chunks::ChunkSection*> sections;
	sections.clear();

	for (int i = 0; i < 16; i++) {
		Internal::Chunks::ChunkSection* cs = chnkc->getSection(i);
		if (cs != NULL && !cs->isEmpty()) {
			mask |= (1 << i);
			numSections++;
			sections.push_back(cs);
		}
	}

	p->buffer->WriteVarInt32(mask);

	const size_t BitsPerEntry = 13;
	const size_t bitmask = (1 << BitsPerEntry) - 1;

	const size_t ChunkSectionDataArraySize = (16 * 16 * 16 * BitsPerEntry) / 8 / 8;  // Convert from bit count to long count
	size_t ChunkSectionSize = (
		1 +                                // Bits per block - set to 13, so the global palette is used and the palette has a length of 0
		1 +                                // Palette length
		2 +                                // Data array length VarInt - 2 bytes for the current value
		ChunkSectionDataArraySize * 8 +    // Actual block data - multiplied by 8 because first number is longs
		16 * 16 * 16  // Block light
		) ;  //Data

	size_t ChunkSize = (
		ChunkSectionSize * numSections +
		256 
		);
	p->buffer->WriteVarInt32(ChunkSize);

	for (auto& cs : sections) {
		p->buffer->WriteBEUInt8(static_cast<uint8_t>(BitsPerEntry));
		p->buffer->WriteVarInt32(0);  // Palette length is 0
		p->buffer->WriteVarInt32(static_cast<uint32_t>(ChunkSectionDataArraySize));

		uint64_t TempLong = 0;  // Temporary value that will be stored into
		uint64_t CurrentlyWrittenIndex = 0;  // "Index" of the long that would be written to

		for (size_t Index = 0; Index < 16 * 16 * 16; Index++)
		{
			uint64_t Value = cs->blocks[Index];
			Value &= bitmask;  // It shouldn't go out of bounds, but it's still worth being careful

			// Painful part where we write data into the long array.  Based off of the normal code.
			size_t BitPosition = Index * BitsPerEntry;
			size_t FirstIndex = BitPosition / 64;
			size_t SecondIndex = ((Index + 1) * BitsPerEntry - 1) / 64;
			size_t BitOffset = BitPosition % 64;

			if (FirstIndex != CurrentlyWrittenIndex)
			{
				// Write the current data before modifiying it.
				p->buffer->WriteBEUInt64(TempLong);
				TempLong = 0;
				CurrentlyWrittenIndex = FirstIndex;
			}

			TempLong |= (Value << BitOffset);

			if (FirstIndex != SecondIndex)
			{
				// Part of the data is now in the second long; write the first one first
				p->buffer->WriteBEUInt64(TempLong);
				CurrentlyWrittenIndex = SecondIndex;

				TempLong = (Value >> (64 - BitOffset));
			}
		}
		// The last long will generally not be written
		p->buffer->WriteBEUInt64(TempLong);

		for (int y = 0; y < 16; y++) {
			for (int z = 0; z < 16; z++) {
				for (int x = 0; x < 16; x += 2) {
					p->buffer->WriteBEUInt8(cs->getLightingAt(x, y, z) | cs->getLightingAt(x + 1, y, z) << 4);
				}
			}
		}

		for (int y = 0; y < 16; y++) {
			for (int z = 0; z < 16; z++) {
				for (int x = 0; x < 16; x += 2) {
					p->buffer->WriteBEUInt8(cs->getSkyLightAt(x, y, z) | cs->getSkyLightAt(x + 1, y, z) << 4);
				}
			}
		}
	}

	for (int x = 0; x < 16; x++) {
		for (int z = 0; z < 16; z++) {
			p->buffer->WriteBEUInt8(chnkc->getBiomeAt(x, z));
		}
	}

	p->buffer->WriteBEUInt8(0);
	
	g_NetMan->AddPacket(p);
	g_NetMan->SendPackets();
}

void Minecraft::Server::Protocol::Play::PacketsOut::send_test_update(int x, int z)
{
	PacketOut* p = new PacketOut(64);
	p->ID = 0x0B;
	p->buffer->WriteBEUInt64((static_cast<uint64_t>(x * 16 & 0x3FFFFFF) << 38) |
		(static_cast<uint64_t>(15 & 0xFFF) << 26) |
		(static_cast<uint64_t>(z * 16 & 0x3FFFFFF))); //POS
	p->buffer->WriteVarInt32(0);

	g_NetMan->AddPacket(p);
	g_NetMan->SendPackets();
}

void Minecraft::Server::Protocol::Play::PacketsOut::send_unload_chunk(int x, int z)
{
	PacketOut* p = new PacketOut(64);
	p->ID = 0x1D;
	p->buffer->WriteBEInt32(x);
	p->buffer->WriteBEInt32(z);

	g_NetMan->AddPacket(p);
	g_NetMan->SendPackets();
}
