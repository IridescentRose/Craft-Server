#include "Utils.h"
#include <iostream>

void utilityPrint(std::string s, int level)
{
#ifdef CRAFT_SERVER_DEBUG

	std::stringstream str;

	switch (level) {
	case LOGGER_LEVEL_TRACE: {
		pspDebugScreenSetTextColor(0xFF777777);
		str << "[TRACE]: ";
		break;
	}
	case LOGGER_LEVEL_DEBUG: {
		pspDebugScreenSetTextColor(0xFF00FF00);
		str << "[DEBUG]: ";
		break;
	}
	case LOGGER_LEVEL_INFO: {
		pspDebugScreenSetTextColor(0xFFE7E7E7);
		str << "[INFO]: ";
		break;
	}
	case LOGGER_LEVEL_WARN: {
		pspDebugScreenSetTextColor(0xFF0077FF);
		str << "[WARN]: ";
		break;
	}
	case LOGGER_LEVEL_ERROR: {
		pspDebugScreenSetTextColor(0xFF0000FF);
		str << "[ERROR]: ";
		break;
	}

	}
	str << s << std::endl;

	if (pspDebugScreenGetY() == 33) {
		pspDebugScreenClear();
		pspDebugScreenSetXY(0, 0);
	}

	pspDebugScreenPrintf(str.str().c_str());
	std::cout << str.str();
#endif

	Stardust::Utilities::app_Logger->log(s, (Stardust::Utilities::LoggerLevel)level);
}
