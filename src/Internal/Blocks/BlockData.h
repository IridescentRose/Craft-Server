#pragma once
#include "../Registry/ItemRegistry.h"

namespace Minecraft::Server::Internal::Blocks{
	using namespace Registry;
	typedef uint16_t BlockProtocolID;

	struct BlockData{
		BlockProtocolID protocolID;
		ItemProtocolID itemID;

		std::string namespaceID;
		std::string name;

		uint8_t lightValue;
		uint8_t spreadLightFalloff;
		bool transparent;
		bool oneHitDig;
		bool pistonBreak;
		bool blocksRain;
		bool skylightDispersant;

		bool snowable;
		bool solid;
		bool usableBySpectator;
		bool fullBlock;
		bool terraformable;
		float blockHeight;
		float hardness;
	};
}