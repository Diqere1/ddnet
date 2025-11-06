#include "f_helper.h"
#include "f_bots.h"

void FBots::Run()
{
	g_pMrxAimbotHelper->m_pAimbot->Aimbot();
	g_pMrxAimbotHelper->m_pAimbot->AutoHit();
}
