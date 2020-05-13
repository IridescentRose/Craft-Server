#include "Config.h"
#include <Utilities/JSON.h>

using namespace Stardust;
void parseServerConfig(std::string path)
{
	Json::Value v = Utilities::JSON::openJSON(path);
	g_Config.difficulty = v["difficulty"].asInt();
	g_Config.gamemode = v["gamemode"].asInt();
	g_Config.port = v["server-port"].asInt();
	g_Config.hardcore = v["hardcore"].asBool();
	g_Config.max_players = v["max-players"].asInt();
	g_Config.motd = v["motd"].asString();
}

void exportServerConfig(std::string path)
{
	Json::Value v;
	v["difficulty"] = (int)g_Config.difficulty;
	v["gamemode"] = (int)g_Config.gamemode;
	v["hardcore"] = (bool)g_Config.hardcore;
	v["max-players"] = (int)g_Config.max_players;
	v["motd"] = g_Config.motd;
	v["server-port"] = (int)g_Config.port;

	std::ofstream f(path);
	f << v;
	f.close();
}

ServerConfig g_Config = { 25565, 0, "Definitely not a PSP", false, 2, 4 };