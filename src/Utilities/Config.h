#pragma once
#include <string>
#include <cstdint>

struct ServerConfig {
	uint16_t	port;
	uint8_t		gamemode;
	std::string	motd;
	bool		hardcore;
	uint8_t		difficulty;
	uint16_t	max_players;
};

extern ServerConfig g_Config;

void parseServerConfig(std::string path = "./properties.json");
void exportServerConfig(std::string path = "./properties.json");