#pragma once
#include "Utils.h"
#include "Networking/Socket2.h"
#include "Networking/NetworkManager2.h"

namespace Minecraft::Server {
	class Server {
	public:

		Server();
		~Server();

		void run();

		void update();

		inline bool isRunning() {
			return m_IsRunning;
		}

	private:
		ServerSocket* socket;
		bool m_IsRunning;
		NetworkManager* netman;
	};
}