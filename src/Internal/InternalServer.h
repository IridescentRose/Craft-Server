#pragma once
#include <Utilities/Thread.h>
#include "Player/Player.h"
#include <glm/glm.hpp>
#include <map>
#include "Chunks/ChunkColumn.h"
#include <mclib/common/Vector.h>

using namespace Stardust::Utilities;
using namespace Minecraft::Server::Internal::Chunks;

namespace Minecraft::Server::Internal {

	class InternalServer {
	public:
		InternalServer();
		~InternalServer();

		void start();
		void stop();

		void chunkgenUpdate();

		bool isOpen();

		glm::ivec2 lastPos;
		std::map<mc::Vector3i, ChunkColumn*> chunkMap;
	private:
		static int tickUpdateThread(unsigned int argc, void* argv);
		Thread* tickUpdate;
		bool bopen;
	};

	extern InternalServer* g_InternalServer;
}