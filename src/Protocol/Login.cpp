#include "Play.h"
#include "Status.h"
#include "Handshake.h"
#include "Login.h"

#include "../Networking/NetworkManager2.h"
#include "../Utilities/Utils.h"
#include <Utilities/JSON.h>
#include "../Utilities/Config.h"

#include <Utilities/UUID.h>
#include "../Internal/InternalServer.h"
#if CURRENT_PLATFORM == PLATFORM_PSP
#include <dirent.h>
#elif CURRENT_PLATFORM == PLATFORM_WIN
#include <filesystem>
#else
#include <experimental/filesystem>
#endif

namespace Minecraft::Server::Protocol {


	Json::Value playerJSON;
	Json::Value banned;
	bool downfall;

	void Login::PacketsOut::send_disconnect(std::string reason, std::string color) {
		//Send a disconnect
		PacketOut* p2 = new PacketOut(512);
		p2->ID = 0x0;

		std::string build = "{\"text\":\"" + reason + "\",\"color\":\"" + color + "\"}";
		p2->buffer->WriteVarUTF8String(build);
		g_NetMan->AddPacket(p2);
		g_NetMan->SendPackets();
		g_NetMan->m_Socket->Close();
	}

	int Login::login_start_packet_handler(PacketIn* p){
		downfall = false;

		p->buffer->ReadVarUTF8String(Internal::Player::g_Player.username);

		utilityPrint(Internal::Player::g_Player.username + " is attempting to join", LOGGER_LEVEL_INFO);


		banned = Utilities::JSON::openJSON("banned.json");
		//BEGIN LOGIN SEQUENCE HERE!


#if CURRENT_PLATFORM == PLATFORM_PSP
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
			}
			else {
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
#else
#if CURRENT_PLATFORM == PLATFORM_WIN
		if (std::filesystem::exists("./playerdata/" + Internal::Player::g_Player.username + ".json")) {
#else
		if (std::experimental::filesystem::exists("./playerdata/" + Internal::Player::g_Player.username + ".json")) {
#endif
			utilityPrint("Found Player Profile", LOGGER_LEVEL_TRACE);
			//Load a JSON for their stats.
			playerJSON = Utilities::JSON::openJSON("./playerdata/" + std::string(Internal::Player::g_Player.username + ".json"));
			Internal::Player::g_Player.uuid = playerJSON["uuid"].asString();
			Internal::Player::g_Player.operatorLevel = playerJSON["oplevel"].asInt();
		}
		else {
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
#endif

		//Check bans
		for (int i = 0; i < banned.size(); i++) {
			if (banned[i].asString() == Internal::Player::g_Player.username || banned[i].asString() == "all") {
				//They're banned! Don't connect!.
				PacketsOut::send_disconnect("You are banned!", "dark_red");
				return -1;
			}
		}

		PacketsOut::send_compression_packet();
		PacketsOut::send_login_success(Internal::Player::g_Player.username);
		g_NetMan->m_Socket->setConnectionStatus(CONNECTION_STATE_PLAY);
		
		
		utilityPrint("Dumping Packet Load!", LOGGER_LEVEL_DEBUG);
		
		int eid = 1337;

		Play::PacketsOut::send_join_game(eid);
		
		
		
		Play::PacketsOut::send_plugin_message("minecraft:brand");
		
		Play::PacketsOut::send_server_difficulty();
		Play::PacketsOut::send_player_abilities();
		
		Play::PacketsOut::send_hotbar_slot(0); //Slot 0
		Play::PacketsOut::send_entity_status(eid, 24 + Internal::Player::g_Player.operatorLevel);
		
		Play::PacketsOut::send_player_info();
		
#if CURRENT_PLATFORM == PLATFORM_PSP
		sceKernelDelayThread(50 * 1000);
#else
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
#endif
		Play::PacketsOut::send_player_position_look();
		Play::PacketsOut::send_world_border();
		Play::PacketsOut::send_time_update();
		Play::PacketsOut::send_spawn_position();
		if (downfall) {
			Play::PacketsOut::send_change_gamestate(2, 0.0f);
		}

		return 0;
	}
	void Login::PacketsOut::send_compression_packet()
	{
		PacketOut* p = new PacketOut(4);
		p->ID = 0x03;
		p->buffer->WriteVarInt32(256); //Our default compression is 256 bytes

		g_NetMan->AddPacket(p);
		g_NetMan->SendPackets();
		g_NetMan->compression = true;
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
}