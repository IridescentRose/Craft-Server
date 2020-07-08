#include "Utils.h"
#include <iostream>

void utilityPrint(std::string s, int level)
{
#ifdef CRAFT_SERVER_DEBUG

	std::stringstream str;

	switch (level) {
	case LOGGER_LEVEL_TRACE: {
#if CURRENT_PLATFORM == PLATFORM_PSP
		pspDebugScreenSetTextColor(0xFF777777);
#endif
		str << "[TRACE]: ";
		break;
	}
	case LOGGER_LEVEL_DEBUG: {
#if CURRENT_PLATFORM == PLATFORM_PSP
		pspDebugScreenSetTextColor(0xFF00FF00);
#endif
		str << "[DEBUG]: ";
		break;
	}
	case LOGGER_LEVEL_INFO: {

#if CURRENT_PLATFORM == PLATFORM_PSP
		pspDebugScreenSetTextColor(0xFFE7E7E7);
#endif
		str << "[INFO]: ";
		break;
	}
	case LOGGER_LEVEL_WARN: {

#if CURRENT_PLATFORM == PLATFORM_PSP
		pspDebugScreenSetTextColor(0xFF0077FF);
#endif
		str << "[WARN]: ";
		break;
	}
	case LOGGER_LEVEL_ERROR: {
#if CURRENT_PLATFORM == PLATFORM_PSP
		pspDebugScreenSetTextColor(0xFF0000FF);
#endif
		str << "[ERROR]: ";
		break;
	}

	}
	str << s << std::endl;

#if CURRENT_PLATFORM == PLATFORM_PSP
	if (pspDebugScreenGetY() == 33) {
		pspDebugScreenClear();
		pspDebugScreenSetXY(0, 0);
	}
	pspDebugScreenPrintf(str.str().c_str());
#endif

	std::cout << str.str();
#endif

	Stardust::Utilities::app_Logger->log(s, (Stardust::Utilities::LoggerLevel)level);
}
