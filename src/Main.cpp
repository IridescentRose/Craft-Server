#include <Platform/Platform.h>

PSP_MODULE_INFO("Craft Server", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_VFPU | THREAD_ATTR_USER);
PSP_HEAP_SIZE_KB(-1024);

using namespace Stardust;

int main() {
	Platform::initPlatform();

	while (true) {
		Platform::platformUpdate();
	}

	Platform::exitPlatform();
	return 0;
}