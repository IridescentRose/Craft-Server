#pragma once
#include "Utils.h"
#include "Networking/Socket2.h"

namespace Minecraft::Server {
	class Server {
	public:

		Server();
		~Server();

		void run();

		inline bool isRunning() {
			return m_IsRunning;
		}

	private:
		ServerSocket* socket;
		bool m_IsRunning;
	};
}