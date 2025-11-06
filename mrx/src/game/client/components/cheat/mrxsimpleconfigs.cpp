#include "mrxsimpleconfigs.h"
#include <game/client/gameclient.h>
#include <engine/console.h>
#include <engine/shared/config.h>

CMRXSimpleConfigs::CMRXSimpleConfigs()
{
	m_pGameClient = nullptr;
	m_pConsole = nullptr;
}

void CMRXSimpleConfigs::Init(CGameClient *pGameClient)
{
	m_pGameClient = pGameClient;
	m_pConsole = pGameClient->Console();
	RegisterConsoleCommands();
}

void CMRXSimpleConfigs::RegisterConsoleCommands()
{
	m_pConsole->Register("mrx_triplefly_fun", "", CFGFLAG_CLIENT, ConTripleFlyFun, m_pGameClient, "Toggle TripleFlyFun mode");
	m_pConsole->Register("mrx_triplefly_normal", "", CFGFLAG_CLIENT, ConTripleFlyNormal, m_pGameClient, "Toggle TripleFlyNormal mode");
}

void CMRXSimpleConfigs::ConTripleFlyFun(IConsole::IResult *pResult, void *pUserData)
{
	CGameClient *pGC = (CGameClient *)pUserData;
	IConsole *pConsole = pGC->Console();

	// Toggle state
	g_Config.m_MRXTripleFlyFunState = !g_Config.m_MRXTripleFlyFunState;

	if(g_Config.m_MRXTripleFlyFunState)
	{
		// Turn off TripleFlyNormal if it was active
		if(g_Config.m_MRXTripleFlyNormalState)
		{
			g_Config.m_MRXTripleFlyNormalState = 0;
			// Execute disable commands for normal mode
			pConsole->ExecuteLine("cl_dummy_copy_moves 0");
			pConsole->ExecuteLine("cl_dummy_control 0");
			pConsole->ExecuteLine("bind mouse1 +fire");
			pConsole->ExecuteLine("bind space +jump");
			pConsole->ExecuteLine("mrx_dummy_hookcursor 0");
			pConsole->ExecuteLine("dummy_reset");
			pConsole->ExecuteLine("mrx_dummy_copy_direction 1");
		}

		// Enable TripleFlyFun
		pConsole->ExecuteLine("cl_dummy_copy_moves 1");
		pConsole->ExecuteLine("mrx_copy 1");
		pConsole->ExecuteLine("mrx_dummy_copy_direction 1");
		pConsole->ExecuteLine("cl_dummy_control 1");
		pConsole->ExecuteLine("bind mouse1 \"+toggle cl_dummy_hook 1 0\"");
		pConsole->ExecuteLine("bind space \"+jump; +toggle cl_dummy_jump 1 0\"");
		pConsole->ExecuteLine("mrx_dummy_hookcursor 1");
		pConsole->ExecuteLine("mrx_autohookdummy 1");
		pConsole->ExecuteLine("mrx_text TripleFlyFun");
		pConsole->ExecuteLine("mrx_text1 \" \"");
	}
	else
	{
		// Disable TripleFlyFun
		pConsole->ExecuteLine("cl_dummy_copy_moves 0");
		pConsole->ExecuteLine("mrx_copy 0");
		pConsole->ExecuteLine("mrx_dummy_copy_direction 1");
		pConsole->ExecuteLine("cl_dummy_control 0");
		pConsole->ExecuteLine("bind mouse1 +fire");
		pConsole->ExecuteLine("bind space +jump");
		pConsole->ExecuteLine("mrx_dummy_hookcursor 0");
		pConsole->ExecuteLine("mrx_autohookdummy 0");
		pConsole->ExecuteLine("mrx_text1 \" \"");
		pConsole->ExecuteLine("mrx_text \" \"");
		pConsole->ExecuteLine("dummy_reset");
	}
}

void CMRXSimpleConfigs::ConTripleFlyNormal(IConsole::IResult *pResult, void *pUserData)
{
	CGameClient *pGC = (CGameClient *)pUserData;
	IConsole *pConsole = pGC->Console();

	// Toggle state
	g_Config.m_MRXTripleFlyNormalState = !g_Config.m_MRXTripleFlyNormalState;

	if(g_Config.m_MRXTripleFlyNormalState)
	{
		// Turn off TripleFlyFun if it was active
		if(g_Config.m_MRXTripleFlyFunState)
		{
			g_Config.m_MRXTripleFlyFunState = 0;
			// Execute disable commands for fun mode
			pConsole->ExecuteLine("cl_dummy_copy_moves 0");
			pConsole->ExecuteLine("cl_dummy_control 0");
			pConsole->ExecuteLine("bind mouse1 +fire");
			pConsole->ExecuteLine("bind space +jump");
			pConsole->ExecuteLine("mrx_dummy_hookcursor 0");
			pConsole->ExecuteLine("dummy_reset");
		}

		// Enable TripleFlyNormal
		pConsole->ExecuteLine("cl_dummy_copy_moves 1");
		pConsole->ExecuteLine("mrx_dummy_copy_direction 0");
		pConsole->ExecuteLine("mrx_copy 0");
		pConsole->ExecuteLine("mrx_copy_nomove 1");
		pConsole->ExecuteLine("cl_dummy_control 1");
		pConsole->ExecuteLine("bind mouse1 \"+toggle cl_dummy_hook 1 0\"");
		pConsole->ExecuteLine("bind space \"+jump; +toggle cl_dummy_jump 1 0\"");
		pConsole->ExecuteLine("mrx_dummy_hookcursor 1");
		pConsole->ExecuteLine("mrx_autohookdummy 1");
		pConsole->ExecuteLine("mrx_text \" \"");
		pConsole->ExecuteLine("mrx_text1 TripleFlyNormal");
	}
	else
	{
		// Disable TripleFlyNormal
		pConsole->ExecuteLine("cl_dummy_copy_moves 0");
		pConsole->ExecuteLine("mrx_dummy_copy_direction 1");
		pConsole->ExecuteLine("cl_dummy_control 0");
		pConsole->ExecuteLine("bind mouse1 +fire");
		pConsole->ExecuteLine("bind space +jump");
		pConsole->ExecuteLine("mrx_dummy_hookcursor 0");
		pConsole->ExecuteLine("mrx_autohookdummy 0");
		pConsole->ExecuteLine("mrx_text \" \"");
		pConsole->ExecuteLine("mrx_text1 \" \"");
		pConsole->ExecuteLine("dummy_reset");
	}
}

