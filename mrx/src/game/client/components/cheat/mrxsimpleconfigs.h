#ifndef GAME_CLIENT_COMPONENTS_CHEAT_MRXSIMPLECONFIGS_H
#define GAME_CLIENT_COMPONENTS_CHEAT_MRXSIMPLECONFIGS_H

#include <engine/console.h>

class CGameClient;

// Класс для управления простыми конфигурациями MRX
class CMRXSimpleConfigs
{
	CGameClient *m_pGameClient;
	IConsole *m_pConsole;

public:
	CMRXSimpleConfigs();
	
	void Init(CGameClient *pGameClient);
	void RegisterConsoleCommands();

	// Console command callbacks
	static void ConTripleFlyFun(IConsole::IResult *pResult, void *pUserData);
	static void ConTripleFlyNormal(IConsole::IResult *pResult, void *pUserData);
};

#endif
