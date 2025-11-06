#include "slowwalk.h"

#include <engine/shared/config.h>
#include <game/client/gameclient.h>
#include <base/system.h>



void CSlowWalk::OnRender()
{
	if(!GameClient()->m_Snap.m_pLocalCharacter)
		return;
}
void CSlowWalk::HandleSlowWalk()
{
	if(g_Config.m_MRXSlowWalk)
	{
		int Dummy = g_Config.m_ClDummy;
		int &Left = GameClient()->m_Controls.m_aInputDirectionLeft[Dummy];
		int &Right = GameClient()->m_Controls.m_aInputDirectionRight[Dummy];
		CNetObj_PlayerInput &Input = GameClient()->m_Controls.m_aInputData[Dummy];

		if(Right && !Left)
		{
			if(FunStepCounter == 0)
				GameClient()->m_Controls.m_aInputData[g_Config.m_ClDummy].m_Direction = 1;
			else if(FunStepCounter == 1)
				GameClient()->m_Controls.m_aInputData[g_Config.m_ClDummy].m_Direction = -1;
			else
				GameClient()->m_Controls.m_aInputData[g_Config.m_ClDummy].m_Direction = 0;

			FunStepCounter++;
			if(FunStepCounter > 1)
				FunStepCounter = 0;
		}
		else if(Left && !Right)
		{
			if(FunStepCounter == 0)
				GameClient()->m_Controls.m_aInputData[g_Config.m_ClDummy].m_Direction = -1;
			else if(FunStepCounter == 1)
				GameClient()->m_Controls.m_aInputData[g_Config.m_ClDummy].m_Direction = 1;
			else
				GameClient()->m_Controls.m_aInputData[g_Config.m_ClDummy].m_Direction = 0;

			FunStepCounter++;
			if(FunStepCounter > 1)
				FunStepCounter = 0;
		}
		else
		{
			GameClient()->m_Controls.m_aInputData[g_Config.m_ClDummy].m_Direction = 0;
			FunStepCounter = 0;
		}
	}
}