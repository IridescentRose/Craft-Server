#pragma once
#include "Utilities/Utils.h"
#include "Networking/Socket2.h"
#include "Networking/NetworkManager2.h"
#include <Utilities/Thread.h>

namespace Minecraft::Server {

	class Server {
	public:

		Server();
		~Server();

		void init();

		void update();

		inline bool isRunning() {
			return m_IsRunning;
		}

	private:
		ServerSocket* socket;
		bool m_IsRunning;
	};

	extern Server* g_Server;
}