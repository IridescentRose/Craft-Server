#include "Utils.h"
#include <iostream>

void utilityPrint(std::string s, int level)
{
#ifdef CRAFT_SERVER_DEBUG
	pspDebugScreenPrintf((s + "\n").c_str());
	//std::cout << s << std::endl;
#endif

	Stardust::Utilities::app_Logger->log(s, (Stardust::Utilities::LoggerLevel)level);
}
