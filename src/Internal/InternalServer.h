#pragma once
#include <Utilities/Thread.h>

using namespace Stardust::Utilities;

namespace Minecraft::Server::Internal {

	class InternalServer {
	public:
		InternalServer();
		~InternalServer();

		void start();
		void stop();

	private:
		static int tickUpdateThread(unsigned int argc, void* argv);
		Thread* tickUpdate;
	};

	extern InternalServer* g_InternalServer;
}