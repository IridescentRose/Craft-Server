#include <Platform/Platform.h>
#include "Server.h"
#include <Graphics/RendererCore.h>
#include <Utilities/Logger.h>

PSP_MODULE_INFO("Craft Server", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_VFPU | THREAD_ATTR_USER);
PSP_HEAP_SIZE_KB(-1024);

using namespace Stardust;
using namespace Stardust::Utilities;
using namespace Stardust::Graphics;
using namespace Minecraft::Server;

int main() {
	Platform::initPlatform("Craft-Server");

#ifdef CRAFT_SERVER_DEBUG
	app_Logger->currentLevel = LOGGER_LEVEL_TRACE;
	detail::core_Logger->currentLevel = LOGGER_LEVEL_TRACE;

	pspDebugScreenInit();
	app_Logger->log("Debug Mode Enabled!");
#endif


	Server* server = new Server();
	try {
		server->run();
	}catch(std::runtime_error e){
		utilityPrint(e.what(), LOGGER_LEVEL_ERROR);

		sceKernelDelayThread(1000 * 1000 * 3);
		Platform::exitPlatform();
	}

	while (server->isRunning()) {
		Platform::platformUpdate();

		server->update();

		sceDisplayWaitVblankStart();
	}

	Platform::exitPlatform();
	return 0;
}