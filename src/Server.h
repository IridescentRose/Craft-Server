#pragma once
#include "Utils.h"
#include "Networking/Socket2.h"
#include "Networking/NetworkManager2.h"
#include <Utilities/Thread.h>

namespace Minecraft::Server {

	class Server {
	public:

		Server();
		~Server();

		void run();

		static int update(unsigned int, void*);

		inline bool isRunning() {
			return m_IsRunning;
		}

	private:
		ServerSocket* socket;
		bool m_IsRunning;
		Thread* thr;
	};

	extern Server* g_Server;
}