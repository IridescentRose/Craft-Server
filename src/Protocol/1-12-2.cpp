#include "1-12-2.h"
#include "../Networking/NetworkManager2.h"
#include "../Utils.h"
#include <Utilities/JSON.h>
#include "../Config.h"

#include <Utilities/UUID.h>
#include "../Internal/InternalServer.h"
#include <dirent.h>

namespace Minecraft::Server::Protocol {

	int Handshake::handshake_packet_handler(PacketIn* p)
	{

		int getVersion = decodeVarInt(*p);
		
		decodeString(*p);
		decodeShort(*p);

		int nextStatus = decodeVarInt(*p);
		
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

	using namespace Stardust::Utilities;

	Json::Value playerJSON;
	Json::Value banned;
	bool downfall;

	int Login::login_start_packet_handler(PacketIn* p)
	{
		downfall = false;

		Internal::Player::g_Player.username = decodeStringLE(*p);

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
			if (banned[i].asString() == Internal::Player::g_Player.username) {
				//They're banned! Don't connect!.
				Play::PacketsOut::send_disconnect("You are banned!", "dark_red");
				return -1;
			}
		}

		Play::PacketsOut::send_plugin_message("MC|Brand");
		Play::PacketsOut::send_server_difficulty();
		Play::PacketsOut::send_player_abilities();
		Play::PacketsOut::send_hotbar_slot(0); //Slot 0
		Play::PacketsOut::send_entity_status(eid, 24 + Internal::Player::g_Player.operatorLevel); //Make them op level 0
		Play::PacketsOut::send_player_list_item();
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
		PacketOut* p2 = new PacketOut();
		p2->ID = 0x02;
		encodeStringLE(Internal::Player::g_Player.uuid, *p2);
		encodeStringLE(username, *p2);

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
		std::string text = decodeStringNonNullLE(*p);

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

		std::string locale = decodeStringNonNullLE(*p);
		uint8_t renderDistance = decodeByte(*p);
		uint8_t chatMode = decodeByte(*p);
		bool colors = decodeBool(*p);
		uint8_t displayed = decodeByte(*p);
		uint8_t mainhand = decodeByte(*p);

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

		std::string channel = decodeStringNonNullLE(*p);
		std::string data = decodeStringNonNullLE(*p);
		utilityPrint("Channel: " + channel, LOGGER_LEVEL_TRACE);
		utilityPrint("Data: " + data, LOGGER_LEVEL_TRACE);

		return 0; 
	}
	
	int Play::use_entity_handler(PacketIn* p) { utilityPrint("USE_ENTITY Triggered!", LOGGER_LEVEL_WARN); return 0; }
	
	int Play::keep_alive_handler(PacketIn* p) { 
		return 0; 
	}
	
	int Play::player_handler(PacketIn* p) { utilityPrint("PLAYER Triggered!", LOGGER_LEVEL_WARN); return 0; }
	int Play::player_position_handler(PacketIn* p) { utilityPrint("PLAYER_POSITION Triggered!", LOGGER_LEVEL_WARN); return 0; }
	
	int Play::player_position_and_look_handler(PacketIn* p) { 
		utilityPrint("Position & Look", LOGGER_LEVEL_TRACE);

		double x, y, z;
		float yaw, pitch;
		bool onGround;

		x = decodeDouble(*p);
		y = decodeDouble(*p);
		z = decodeDouble(*p);

		yaw = decodeFloat(*p);
		pitch = decodeFloat(*p);

		onGround = decodeBool(*p);

		utilityPrint("X: " + std::to_string(x), LOGGER_LEVEL_TRACE);
		utilityPrint("Y: " + std::to_string(y), LOGGER_LEVEL_TRACE);
		utilityPrint("Z: " + std::to_string(z), LOGGER_LEVEL_TRACE);


		utilityPrint("Yaw: " + std::to_string(yaw), LOGGER_LEVEL_TRACE);
		utilityPrint("Pitch: " + std::to_string(pitch), LOGGER_LEVEL_TRACE);

		utilityPrint("On Ground: " + std::to_string(onGround), LOGGER_LEVEL_TRACE);


		return 0; 
	}
	
	int Play::player_look_handler(PacketIn* p) { utilityPrint("PLAYER_LOOK Triggered!", LOGGER_LEVEL_WARN); return 0; }
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
	int Play::animation_handler(PacketIn* p) { utilityPrint("ANIMATION Triggered!", LOGGER_LEVEL_WARN); return 0; }
	int Play::spectate_handler(PacketIn* p) { utilityPrint("SPECTATE Triggered!", LOGGER_LEVEL_WARN); return 0; }
	int Play::player_block_placement_handler(PacketIn* p) { utilityPrint("PLAYER_BLOCK_PLACEMENT Triggered!", LOGGER_LEVEL_WARN); return 0; }
	int Play::use_item_handler(PacketIn* p) { utilityPrint("USE_ITEM Triggered!", LOGGER_LEVEL_WARN); return 0; }
}

void Minecraft::Server::Protocol::Play::PacketsOut::send_join_game(int eid)
{
	PacketOut* p = new PacketOut();

	p->ID = 0x23;
	encodeInt(eid, *p);
	encodeByte(g_Config.gamemode, *p);
	encodeInt(0, *p); //DIMENSION;
	encodeByte(g_Config.difficulty, *p);
	encodeByte(1, *p); //Max players, useless
	encodeStringLE("default", *p); //Level type
	encodeBool(false, *p); //Reduce debug info? NONONONONO!

	g_NetMan->AddPacket(p);
	g_NetMan->SendPackets();
}

void Minecraft::Server::Protocol::Play::PacketsOut::send_plugin_message(std::string type)
{
	if (type == "MC|Brand") {
		PacketOut* p = new PacketOut();
		p->ID = 0x18;
		encodeStringLE(type, *p);
		encodeStringLE("PSP-Craft", *p);

		g_NetMan->AddPacket(p);
		g_NetMan->SendPackets();
	}
}

void Minecraft::Server::Protocol::Play::PacketsOut::send_server_difficulty()
{
	PacketOut* p = new PacketOut();
	p->ID = 0x0D;
	encodeByte(g_Config.difficulty, *p);

	g_NetMan->AddPacket(p);
	g_NetMan->SendPackets();
}

void Minecraft::Server::Protocol::Play::PacketsOut::send_player_abilities()
{
	PacketOut* p = new PacketOut();
	p->ID = 0x2C;
	encodeByte(0, *p);
	encodeFloat(0.5f, *p);
	encodeFloat(0.1f, *p);

	g_NetMan->AddPacket(p);
	g_NetMan->SendPackets();
}

void Minecraft::Server::Protocol::Play::PacketsOut::send_hotbar_slot(int slot)
{
	PacketOut* p = new PacketOut();
	p->ID = 0x3A;
	encodeByte(0, *p);

	g_NetMan->AddPacket(p);
	g_NetMan->SendPackets();
}

void Minecraft::Server::Protocol::Play::PacketsOut::send_entity_status(int eid, int action)
{
	PacketOut* p = new PacketOut();
	p->ID = 0x1B;
	encodeInt(eid, *p);
	encodeByte(action, *p);

	g_NetMan->AddPacket(p);
	g_NetMan->SendPackets();
}

void Minecraft::Server::Protocol::Play::PacketsOut::send_player_list_item()
{
}

void Minecraft::Server::Protocol::Play::PacketsOut::send_player_position_look()
{

	PacketOut* p = new PacketOut();
	p->ID = 0x2F;
	encodeDouble(0, *p);
	encodeDouble(63, *p);
	encodeDouble(0, *p);
	encodeFloat(0, *p);
	encodeFloat(0, *p);

	encodeByte(0, *p);
	encodeByte(1, *p);//TP ID

	g_NetMan->AddPacket(p);
	g_NetMan->SendPackets();
}

void Minecraft::Server::Protocol::Play::PacketsOut::send_world_border()
{
	PacketOut* p = new PacketOut();
	p->ID = 0x38;
	encodeByte(3, *p);
	//INIT
	encodeDouble(0, *p);
	encodeDouble(0, *p);

	encodeDouble(60000000.0, *p);
	encodeDouble(60000000.0, *p);

	encodeByte(0, *p);
	encodeVarInt(29999984, p->bytes);

	encodeByte(5, *p);
	encodeByte(15, *p);

	g_NetMan->AddPacket(p);
	g_NetMan->SendPackets();
}

void Minecraft::Server::Protocol::Play::PacketsOut::send_time_update()
{
	PacketOut* p = new PacketOut();
	p->ID = 0x47;
	encodeLong(0, *p);
	encodeLong(0, *p);

	g_NetMan->AddPacket(p);
	g_NetMan->SendPackets();
}

void Minecraft::Server::Protocol::Play::PacketsOut::send_spawn_position()
{
	PacketOut* p = new PacketOut();
	p->ID = 0x46;
	encodeLong((((long long)0 & 0x3FFFFFFLL) << 38) | ((0 & 0x3FFFFFF) << 12) | (63 & 0xFFF), *p);

	g_NetMan->AddPacket(p);
	g_NetMan->SendPackets();
}

void Minecraft::Server::Protocol::Play::PacketsOut::send_keepalive(long long int ll)
{
	PacketOut* p = new PacketOut();
	p->ID = 0x1F;
	encodeLong(ll, *p);

	g_NetMan->AddPacket(p);
	g_NetMan->SendPackets();
}

void Minecraft::Server::Protocol::Play::PacketsOut::send_chat(std::string text, std::string color, std::string format, bool serverChat)
{
	PacketOut* p = new PacketOut();
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
	
	encodeStringNonNull(build, *p);

	g_NetMan->AddPacket(p);
	g_NetMan->SendPackets();
}

void Minecraft::Server::Protocol::Play::PacketsOut::send_chat_command(std::string text)
{
	bool err = true;
	std::string response;
	if (text == "/help") {
		response = "Currently there are no in-game commands. However, you might want to look at console commands.\nConsole:\n/stop - Stops the server.\n/say - Speaks as server.";
		err = false;
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
		if (Internal::Player::g_Player.operatorLevel < 3) {
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
		if (Internal::Player::g_Player.operatorLevel < 3) {
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
	else {
		response = "Unrecognized Command!";
	}

	PacketOut* p = new PacketOut();
	p->ID = 0x0F;
	std::string build = "{\"text\":\"" + response + "\"";
	if (!err) {
		build += ",\"color\":\"green\"";
	}
	else {
		build += ",\"color\":\"dark_red\"";
	}


	build += "} ";

	encodeStringNonNull(build, *p);

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
	PacketOut* p2 = new PacketOut();
	p2->ID = 0x1A;

	std::string build = "{\"text\":\"" + reason + "\",\"color\":\"" + color + "\"}";
	encodeString(build, *p2);
	g_NetMan->AddPacket(p2);
	g_NetMan->SendPackets();
	g_NetMan->m_Socket->Close();


}

void Minecraft::Server::Protocol::Play::PacketsOut::send_change_gamestate(uint8_t code, float value)
{
	PacketOut* p2 = new PacketOut();
	p2->ID = 0x1E;

	encodeByte(code, *p2);
	encodeFloat(value, *p2);

	g_NetMan->AddPacket(p2);
	g_NetMan->SendPackets();
}
