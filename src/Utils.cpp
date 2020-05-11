#include "Utils.h"

void utilityPrint(std::string s, int level)
{
#ifdef CRAFT_SERVER_DEBUG
	pspDebugScreenPrintf(s.c_str());
#endif

	Stardust::Utilities::app_Logger->log(s, (Stardust::Utilities::LoggerLevel)level);
}
