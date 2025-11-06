/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <base/math.h>

#include <engine/client.h>
#include <engine/shared/config.h>

#include <game/client/components/camera.h>
#include <game/client/components/chat.h>
#include <game/client/components/menus.h>
#include <game/client/components/scoreboard.h>
#include <game/client/gameclient.h>
#include <game/collision.h>

#include <base/vmath.h>

#include "controls.h"
#include "cheat/mrxaimbot/f_helper.h"

CControls::CControls()
{
	mem_zero(&m_aLastData, sizeof(m_aLastData));
	mem_zero(m_aMousePos, sizeof(m_aMousePos));
	mem_zero(m_aMousePosOnAction, sizeof(m_aMousePosOnAction));
	mem_zero(m_aTargetPos, sizeof(m_aTargetPos));
	mem_zero(m_aDesiredFire, sizeof(m_aDesiredFire));
}

void CControls::OnReset()
{
	ResetInput(0);
	ResetInput(1);

	for(int &AmmoCount : m_aAmmoCount)
		AmmoCount = 0;

	m_LastSendTime = 0;
}

void CControls::ResetInput(int Dummy)
{
	m_aLastData[Dummy].m_Direction = 0;
	// simulate releasing the fire button
	if((m_aLastData[Dummy].m_Fire & 1) != 0)
		m_aLastData[Dummy].m_Fire++;
	m_aLastData[Dummy].m_Fire &= INPUT_STATE_MASK;
	m_aLastData[Dummy].m_Jump = 0;
	m_aInputData[Dummy] = m_aLastData[Dummy];

	m_aInputDirectionLeft[Dummy] = 0;
	m_aInputDirectionRight[Dummy] = 0;
	m_aDesiredFire[Dummy] = 0;
}

void CControls::OnPlayerDeath()
{
	for(int &AmmoCount : m_aAmmoCount)
		AmmoCount = 0;
}

struct CInputState
{
	CControls *m_pControls;
	int *m_apVariables[NUM_DUMMIES];
};

void CControls::ConKeyInputState(IConsole::IResult *pResult, void *pUserData)
{
	CInputState *pState = (CInputState *)pUserData;

	if(pState->m_pControls->GameClient()->m_GameInfo.m_BugDDRaceInput && pState->m_pControls->GameClient()->m_Snap.m_SpecInfo.m_Active)
		return;

	*pState->m_apVariables[g_Config.m_ClDummy] = pResult->GetInteger(0);
}

void CControls::ConKeyInputCounter(IConsole::IResult *pResult, void *pUserData)
{
	CInputState *pState = (CInputState *)pUserData;

	if((pState->m_pControls->GameClient()->m_GameInfo.m_BugDDRaceInput && pState->m_pControls->GameClient()->m_Snap.m_SpecInfo.m_Active) || pState->m_pControls->GameClient()->m_Spectator.IsActive())
		return;

	int *pVariable = pState->m_apVariables[g_Config.m_ClDummy];
	if(((*pVariable) & 1) != pResult->GetInteger(0))
		(*pVariable)++;
	*pVariable &= INPUT_STATE_MASK;
}

void CControls::ConKeyInputFireWithCheck(IConsole::IResult *pResult, void *pUserData)
{
	CInputState *pState = (CInputState *)pUserData;

	if((pState->m_pControls->GameClient()->m_GameInfo.m_BugDDRaceInput && pState->m_pControls->GameClient()->m_Snap.m_SpecInfo.m_Active) || pState->m_pControls->GameClient()->m_Spectator.IsActive())
		return;

	// Сохраняем желаемое состояние fire
	int DesiredState = pResult->GetInteger(0);
	
	// Если функция отключена через конфиг, используем стандартное поведение
	if(!g_Config.m_MRXFireCheckRadius || (pState->m_pControls->GameClient()->m_Snap.m_pLocalCharacter && pState->m_pControls->GameClient()->m_Snap.m_pLocalCharacter->m_Weapon != WEAPON_HAMMER))
	{
		int *pVariable = pState->m_apVariables[g_Config.m_ClDummy];
		if(((*pVariable) & 1) != DesiredState)
			(*pVariable)++;
		*pVariable &= INPUT_STATE_MASK;
		pState->m_pControls->m_aDesiredFire[g_Config.m_ClDummy] = DesiredState; // Keep desired state for other weapons
		return;
	}

	// Обновляем желаемое состояние
	pState->m_pControls->m_aDesiredFire[g_Config.m_ClDummy] = DesiredState;

	// Проверяем, есть ли игроки в радиусе 63 единиц
	if(pState->m_pControls->GameClient()->m_Snap.m_pLocalCharacter)
	{
		int LocalId = pState->m_pControls->GameClient()->m_aLocalIds[g_Config.m_ClDummy];
		if(LocalId < 0)
			return;

		vec2 LocalPos = pState->m_pControls->GameClient()->m_aClients[LocalId].m_Predicted.m_Pos;
		bool HasPlayerNearby = false;
		const float FireRadius = 63.0f;

		// Проверяем всех клиентов
		for(int i = 0; i < MAX_CLIENTS; i++)
		{
			if(i == LocalId)
				continue;

			if(!pState->m_pControls->GameClient()->m_aClients[i].m_Active)
				continue;

			vec2 PlayerPos = pState->m_pControls->GameClient()->m_aClients[i].m_Predicted.m_Pos;
			float Dist = distance(LocalPos, PlayerPos);

			if(Dist <= FireRadius)
			{
				HasPlayerNearby = true;
				break;
			}
		}

		// Устанавливаем m_Fire только если есть игрок поблизости
		if(HasPlayerNearby)
		{
			int *pVariable = pState->m_apVariables[g_Config.m_ClDummy];
			if(((*pVariable) & 1) != DesiredState)
				(*pVariable)++;
			*pVariable &= INPUT_STATE_MASK;
		}
	}
}

struct CInputSet
{
	CControls *m_pControls;
	int *m_apVariables[NUM_DUMMIES];
	int m_Value;
};

void CControls::ConKeyInputSet(IConsole::IResult *pResult, void *pUserData)
{
	CInputSet *pSet = (CInputSet *)pUserData;
	if(pResult->GetInteger(0))
	{
		*pSet->m_apVariables[g_Config.m_ClDummy] = pSet->m_Value;
	}
}

void CControls::ConKeyInputNextPrevWeapon(IConsole::IResult *pResult, void *pUserData)
{
	CInputSet *pSet = (CInputSet *)pUserData;
	ConKeyInputCounter(pResult, pSet);
	pSet->m_pControls->m_aInputData[g_Config.m_ClDummy].m_WantedWeapon = 0;
}

static void ConKeyAutoHitHold(IConsole::IResult *pResult, void *pUserData)
{
	CControls *pThis = static_cast<CControls *>(pUserData);
	bool held = pResult->GetInteger(0) != 0;
	if(g_pMrxAimbotHelper && g_pMrxAimbotHelper->m_pAimbot)
		g_pMrxAimbotHelper->m_pAimbot->SetAutoHitHeld(held);
}

void CControls::OnConsoleInit()
{
	// game commands
	{
		static CInputState s_State = {this, {&m_aInputDirectionLeft[0], &m_aInputDirectionLeft[1]}};
		Console()->Register("+left", "", CFGFLAG_CLIENT, ConKeyInputState, &s_State, "Move left");
	}
	{
		static CInputState s_State = {this, {&m_aInputDirectionRight[0], &m_aInputDirectionRight[1]}};
		Console()->Register("+right", "", CFGFLAG_CLIENT, ConKeyInputState, &s_State, "Move right");
	}
	{
		static CInputState s_State = {this, {&m_aInputData[0].m_Jump, &m_aInputData[1].m_Jump}};
		Console()->Register("+jump", "", CFGFLAG_CLIENT, ConKeyInputState, &s_State, "Jump");
	}
	{
		static CInputState s_State = {this, {&m_aInputData[0].m_Hook, &m_aInputData[1].m_Hook}};
		Console()->Register("+hook", "", CFGFLAG_CLIENT, ConKeyInputState, &s_State, "Hook");
	}
	{
		static CInputState s_State = {this, {&m_aInputData[0].m_Fire, &m_aInputData[1].m_Fire}};
		Console()->Register("+fire", "", CFGFLAG_CLIENT, ConKeyInputFireWithCheck, &s_State, "Fire");
	}
	{
		static CInputState s_State = {this, {&m_aShowHookColl[0], &m_aShowHookColl[1]}};
		Console()->Register("+showhookcoll", "", CFGFLAG_CLIENT, ConKeyInputState, &s_State, "Show Hook Collision");
	}

	{
		static CInputSet s_Set = {this, {&m_aInputData[0].m_WantedWeapon, &m_aInputData[1].m_WantedWeapon}, 1};
		Console()->Register("+weapon1", "", CFGFLAG_CLIENT, ConKeyInputSet, &s_Set, "Switch to hammer");
	}
	{
		static CInputSet s_Set = {this, {&m_aInputData[0].m_WantedWeapon, &m_aInputData[1].m_WantedWeapon}, 2};
		Console()->Register("+weapon2", "", CFGFLAG_CLIENT, ConKeyInputSet, &s_Set, "Switch to gun");
	}
	{
		static CInputSet s_Set = {this, {&m_aInputData[0].m_WantedWeapon, &m_aInputData[1].m_WantedWeapon}, 3};
		Console()->Register("+weapon3", "", CFGFLAG_CLIENT, ConKeyInputSet, &s_Set, "Switch to shotgun");
	}
	{
		static CInputSet s_Set = {this, {&m_aInputData[0].m_WantedWeapon, &m_aInputData[1].m_WantedWeapon}, 4};
		Console()->Register("+weapon4", "", CFGFLAG_CLIENT, ConKeyInputSet, &s_Set, "Switch to grenade");
	}
	{
		static CInputSet s_Set = {this, {&m_aInputData[0].m_WantedWeapon, &m_aInputData[1].m_WantedWeapon}, 5};
		Console()->Register("+weapon5", "", CFGFLAG_CLIENT, ConKeyInputSet, &s_Set, "Switch to laser");
	}

	{
		static CInputSet s_Set = {this, {&m_aInputData[0].m_NextWeapon, &m_aInputData[1].m_NextWeapon}, 0};
		Console()->Register("+nextweapon", "", CFGFLAG_CLIENT, ConKeyInputNextPrevWeapon, &s_Set, "Switch to next weapon");
	}
	{
		static CInputSet s_Set = {this, {&m_aInputData[0].m_PrevWeapon, &m_aInputData[1].m_PrevWeapon}, 0};
		Console()->Register("+prevweapon", "", CFGFLAG_CLIENT, ConKeyInputNextPrevWeapon, &s_Set, "Switch to previous weapon");
	}
	Console()->Register("+mrx_aimbot_autohit", "", CFGFLAG_CLIENT, ConKeyAutoHitHold, this, "Hold for auto hammer hit");
	Console()->Register("-mrx_aimbot_autohit", "", CFGFLAG_CLIENT, ConKeyAutoHitHold, this, "Release for auto hammer hit");
}

void CControls::OnMessage(int Msg, void *pRawMsg)
{
	if(Msg == NETMSGTYPE_SV_WEAPONPICKUP)
	{
		CNetMsg_Sv_WeaponPickup *pMsg = (CNetMsg_Sv_WeaponPickup *)pRawMsg;
		if(g_Config.m_ClAutoswitchWeapons)
			m_aInputData[g_Config.m_ClDummy].m_WantedWeapon = pMsg->m_Weapon + 1;
		// We don't really know ammo count, until we'll switch to that weapon, but any non-zero count will suffice here
		m_aAmmoCount[maximum(0, pMsg->m_Weapon % NUM_WEAPONS)] = 10;
	}
}

int CControls::SnapInput(int *pData)
{
	// update player state
	if(GameClient()->m_Chat.IsActive())
	{
		// Hide chat emote state from server if enabled
		if(g_Config.m_MRXHideChatEmote)
			m_aInputData[g_Config.m_ClDummy].m_PlayerFlags = PLAYERFLAG_PLAYING;
		else
			m_aInputData[g_Config.m_ClDummy].m_PlayerFlags = PLAYERFLAG_CHATTING;
	}
	else if(GameClient()->m_Menus.IsActive())
		m_aInputData[g_Config.m_ClDummy].m_PlayerFlags = PLAYERFLAG_IN_MENU;
	else
		m_aInputData[g_Config.m_ClDummy].m_PlayerFlags = PLAYERFLAG_PLAYING;

	if(GameClient()->m_Scoreboard.IsActive())
		m_aInputData[g_Config.m_ClDummy].m_PlayerFlags |= PLAYERFLAG_SCOREBOARD;

	if(Client()->ServerCapAnyPlayerFlag() && GameClient()->m_Controls.m_aShowHookColl[g_Config.m_ClDummy])
		m_aInputData[g_Config.m_ClDummy].m_PlayerFlags |= PLAYERFLAG_AIM;

	if(Client()->ServerCapAnyPlayerFlag() && GameClient()->m_Camera.CamType() == CCamera::CAMTYPE_SPEC)
		m_aInputData[g_Config.m_ClDummy].m_PlayerFlags |= PLAYERFLAG_SPEC_CAM;

	bool Send = m_aLastData[g_Config.m_ClDummy].m_PlayerFlags != m_aInputData[g_Config.m_ClDummy].m_PlayerFlags;

	m_aLastData[g_Config.m_ClDummy].m_PlayerFlags = m_aInputData[g_Config.m_ClDummy].m_PlayerFlags;

	// we freeze the input if chat or menu is activated
	if(!(m_aInputData[g_Config.m_ClDummy].m_PlayerFlags & PLAYERFLAG_PLAYING))
	{
		if(!GameClient()->m_GameInfo.m_BugDDRaceInput)
			ResetInput(g_Config.m_ClDummy);

		mem_copy(pData, &m_aInputData[g_Config.m_ClDummy], sizeof(m_aInputData[0]));

		// set the target anyway though so that we can keep seeing our surroundings,
		// even if chat or menu are activated
		m_aInputData[g_Config.m_ClDummy].m_TargetX = (int)m_aMousePos[g_Config.m_ClDummy].x;
		m_aInputData[g_Config.m_ClDummy].m_TargetY = (int)m_aMousePos[g_Config.m_ClDummy].y;

		// send once a second just to be sure
		Send = Send || time_get() > m_LastSendTime + time_freq();
	}
	else
	{
		m_aInputData[g_Config.m_ClDummy].m_TargetX = (int)m_aMousePos[g_Config.m_ClDummy].x;
		m_aInputData[g_Config.m_ClDummy].m_TargetY = (int)m_aMousePos[g_Config.m_ClDummy].y;

		if(g_Config.m_ClSubTickAiming && m_aMousePosOnAction[g_Config.m_ClDummy] != vec2(0.0f, 0.0f))
		{
			m_aInputData[g_Config.m_ClDummy].m_TargetX = (int)m_aMousePosOnAction[g_Config.m_ClDummy].x;
			m_aInputData[g_Config.m_ClDummy].m_TargetY = (int)m_aMousePosOnAction[g_Config.m_ClDummy].y;
			m_aMousePosOnAction[g_Config.m_ClDummy] = vec2(0.0f, 0.0f);
		}

		if(!m_aInputData[g_Config.m_ClDummy].m_TargetX && !m_aInputData[g_Config.m_ClDummy].m_TargetY)
		{
			m_aInputData[g_Config.m_ClDummy].m_TargetX = 1;
			m_aMousePos[g_Config.m_ClDummy].x = 1;
		}

		// set direction
		m_aInputData[g_Config.m_ClDummy].m_Direction = 0;
		if(m_aInputDirectionLeft[g_Config.m_ClDummy] && !m_aInputDirectionRight[g_Config.m_ClDummy])
			m_aInputData[g_Config.m_ClDummy].m_Direction = -1;
		if(!m_aInputDirectionLeft[g_Config.m_ClDummy] && m_aInputDirectionRight[g_Config.m_ClDummy])
			m_aInputData[g_Config.m_ClDummy].m_Direction = 1;

			if(g_Config.m_MRXThrowDummy)
			{
				GameClient()->m_MrxClient.DummyThrow();
			}
			
		if(g_Config.m_MRXNullMovement)
		{
			if(m_aInputDirectionLeft[g_Config.m_ClDummy] && m_aInputDirectionRight[g_Config.m_ClDummy])
			{
				if(m_aLastData[g_Config.m_ClDummy].m_Direction != 0 && !m_NullMovement)
				{
					m_NullMovement = true;
					m_aInputData[g_Config.m_ClDummy].m_Direction = m_aLastData[g_Config.m_ClDummy].m_Direction * -1;
				}
				else if(m_NullMovement)
				{
					m_aInputData[g_Config.m_ClDummy].m_Direction = m_aLastData[g_Config.m_ClDummy].m_Direction;
				}
			}
			else
			{
				m_NullMovement = false;
			}
		}		

		if(g_Config.m_MRXPixelWalk)
		{
			GameClient()->m_PixelWalk.HandlePixelWalk();
		}

		if(g_Config.m_MRXSlowWalk)
		{
			GameClient()->m_SlowWalk.HandleSlowWalk();
		}

		if(g_Config.m_MRXAutoHookDummy)
		{
			GameClient()->m_AimHookDummy.HandleHook();
		}

		if(g_Config.m_MRXAvoidEnable)
		{
			GameClient()->m_AvoidFreeze.HandleAvoidFreeze();
		}

		// MRX Aimbot: always sync config, run only when enabled; reset when disabled
		if(g_pMrxAimbotHelper)
		{
			// Sync config values from MRX to helper every tick
			g_pMrxAimbotHelper->m_pConfig->aimbotCfg.enabled = g_Config.m_MRXAimbotEnable;
			g_pMrxAimbotHelper->m_pConfig->aimbotCfg.fov = g_Config.m_MRXAimbotFov;
			g_pMrxAimbotHelper->m_pConfig->aimbotCfg.silent = g_Config.m_MRXAimbotSilent;
			g_pMrxAimbotHelper->m_pConfig->aimbotCfg.hookVisible = g_Config.m_MRXAimbotAutoHook;
			g_pMrxAimbotHelper->m_pConfig->aimbotCfg.autoHit = g_Config.m_MRXAimbotAutoHit;
			g_pMrxAimbotHelper->m_pConfig->aimbotCfg.edge = g_Config.m_MRXAimbotEdge;
			g_pMrxAimbotHelper->m_pConfig->aimbotCfg.accuracy = g_Config.m_MRXAimbotAccuracy;
			g_pMrxAimbotHelper->m_pConfig->espCfg.enabled = g_Config.m_MRXEspEnable;
			g_pMrxAimbotHelper->m_pConfig->espCfg.drawFov = g_Config.m_MRXEspDrawFov;
			g_pMrxAimbotHelper->m_pConfig->espCfg.drawBox = g_Config.m_MRXEspDrawBox;
			g_pMrxAimbotHelper->m_pConfig->espCfg.drawGlow = g_Config.m_MRXEspGlowPlayer;

			if(g_Config.m_MRXAimbotEnable)
			{
				// Run aimbot logic
				g_pMrxAimbotHelper->m_pBots->Run();
			}
			else
			{
				// Ensure stale target state is cleared when disabled
				if(g_pMrxAimbotHelper->m_pAimbot)
				{
					g_pMrxAimbotHelper->m_pAimbot->m_TargetVisible = false;
					g_pMrxAimbotHelper->m_pAimbot->m_TargetId = -1;
					g_pMrxAimbotHelper->m_pAimbot->m_TargetPos = vec2(0.f, 0.f);
					g_pMrxAimbotHelper->m_pAimbot->m_TargetVel = vec2(0.f, 0.f);
					g_pMrxAimbotHelper->m_pAimbot->m_CanAim = true;
				}
			}
		}

		// dummy copy moves
		if(g_Config.m_ClDummyCopyMoves)
		{
			CNetObj_PlayerInput *pDummyInput = &GameClient()->m_DummyInput;
			if(g_Config.m_MRXDummyCopyDirection)
			{
				pDummyInput->m_Direction = m_aInputData[g_Config.m_ClDummy].m_Direction;
			}
			pDummyInput->m_Hook = m_aInputData[g_Config.m_ClDummy].m_Hook;
			pDummyInput->m_Jump = m_aInputData[g_Config.m_ClDummy].m_Jump;
			pDummyInput->m_PlayerFlags = m_aInputData[g_Config.m_ClDummy].m_PlayerFlags;
			pDummyInput->m_TargetX = m_aInputData[g_Config.m_ClDummy].m_TargetX;
			pDummyInput->m_TargetY = m_aInputData[g_Config.m_ClDummy].m_TargetY;
			pDummyInput->m_WantedWeapon = m_aInputData[g_Config.m_ClDummy].m_WantedWeapon;

			// MRX: принудительно удерживаем молот для дамми, когда активны autopseudo copy-моды или cl_dummy_hammer
			if(g_Config.m_MRXDummyPseudo && (g_Config.m_MRXCopy || g_Config.m_MRXCopy_nomove || g_Config.m_ClDummyHammer))
			{
				pDummyInput->m_WantedWeapon = WEAPON_HAMMER + 1;
			}

			if(!g_Config.m_ClDummyControl && g_Config.m_MRXDummyCopyFire)
				pDummyInput->m_Fire += m_aInputData[g_Config.m_ClDummy].m_Fire - m_aLastData[g_Config.m_ClDummy].m_Fire;

			pDummyInput->m_NextWeapon += m_aInputData[g_Config.m_ClDummy].m_NextWeapon - m_aLastData[g_Config.m_ClDummy].m_NextWeapon;
			pDummyInput->m_PrevWeapon += m_aInputData[g_Config.m_ClDummy].m_PrevWeapon - m_aLastData[g_Config.m_ClDummy].m_PrevWeapon;

			m_aInputData[!g_Config.m_ClDummy] = *pDummyInput;

			// MRX
			if(g_Config.m_ClDummyCopyMoves && g_Config.m_MRXAutotriplefly && g_Config.m_MRXAutotripleflyMode == 1)
			{
				GameClient()->m_TripleHookHolder.HandleAutoTripleFlyDefault();
			}

			if(g_Config.m_MRXAdvancedDeepFlyManual)
			{
			 GameClient()->m_AdvancedDeepFly.HandleAdvancedDeepFly();
			}

		}
		


		if(g_Config.m_MRXReverseDummyCopyMoves)
		{
			CNetObj_PlayerInput *pDummyInput = &GameClient()->m_DummyInput;
			pDummyInput->m_Direction = -m_aInputData[g_Config.m_ClDummy].m_Direction;
			pDummyInput->m_Hook = m_aInputData[g_Config.m_ClDummy].m_Hook;
			pDummyInput->m_Jump = m_aInputData[g_Config.m_ClDummy].m_Jump;
			pDummyInput->m_PlayerFlags = m_aInputData[g_Config.m_ClDummy].m_PlayerFlags;
			pDummyInput->m_TargetX = -m_aInputData[g_Config.m_ClDummy].m_TargetX;
			pDummyInput->m_TargetY = m_aInputData[g_Config.m_ClDummy].m_TargetY;
			pDummyInput->m_WantedWeapon = m_aInputData[g_Config.m_ClDummy].m_WantedWeapon;
		
			if(!g_Config.m_ClDummyControl)
				pDummyInput->m_Fire += m_aInputData[g_Config.m_ClDummy].m_Fire - m_aLastData[g_Config.m_ClDummy].m_Fire;
		
			pDummyInput->m_NextWeapon += m_aInputData[g_Config.m_ClDummy].m_NextWeapon - m_aLastData[g_Config.m_ClDummy].m_NextWeapon;
			pDummyInput->m_PrevWeapon += m_aInputData[g_Config.m_ClDummy].m_PrevWeapon - m_aLastData[g_Config.m_ClDummy].m_PrevWeapon;
		
			m_aInputData[!g_Config.m_ClDummy] = *pDummyInput;
		}

		if(g_Config.m_ClDummyControl)
		{
			CNetObj_PlayerInput *pDummyInput = &GameClient()->m_DummyInput;
			pDummyInput->m_Jump = g_Config.m_ClDummyJump;

			if(g_Config.m_ClDummyFire)
				pDummyInput->m_Fire = g_Config.m_ClDummyFire;
			else if((pDummyInput->m_Fire & 1) != 0)
				pDummyInput->m_Fire++;

			pDummyInput->m_Hook = g_Config.m_ClDummyHook;
		}

		// **--- MRX ---** //

		if(g_Config.m_ClDummyHook)
	{	
		// Шаг 1: Устанавливаем направление хука (если нужно)
		if(g_Config.m_MRXDummyHookCursor)
		{
			GameClient()->m_DummyHookCursor.HandleDummyHookCursor();
		}
		
		// Шаг 2: Решаем хукать или нет (независимо от направления)
		if(g_Config.m_MRXSmartDummyHook)
		{
			// Умный хук дамми - хукать только при определённой скорости
			GameClient()->m_MrxClient.HandleSmartDummyHook();
		}
		else 
		{
			// Стандартное поведение cl_dummy_hook - всегда хукать
			CNetObj_PlayerInput *pDummyInput = &GameClient()->m_DummyInput;
			pDummyInput->m_Hook = g_Config.m_ClDummyHook;   
		}
	}
	
		
		if(g_Config.m_MRXAutoAled)
		{
			GameClient()->m_AutoAled.HandleAutoAled();
		}

		if(g_Config.m_MRXDummyAled)
		{
			GameClient()->m_DummyAled.HandleDummyAled();
		} 

		//-
		GameClient()->m_HookFly.HandleHookFly();
		//-

		if(g_Config.m_MRXBalanceBot)
		{
			GameClient()->m_BalanceBot.Balance();
		}
		
		if(g_Config.m_MRXMoveflyBot)
		{
		GameClient()->m_MoveFlyBot.MoveFly();
		}
		
		//-
		GameClient()->m_TripleHookHolder.HandleTripleHookHolder();
		//-

		if(g_Config.m_MRXAutotriplefly && g_Config.m_MRXAutotripleflyMode == 0)
		{
			GameClient()->m_TripleHookHolder.HandleAutoTripleFly();
		}

		if(g_Config.m_MRXAutotripleflyThrow)
		{
			GameClient()->m_TripleHookHolder.HandleAutoTripleFlyThrow();
		}

		if(g_Config.m_MRXAutotripleflyThrowBoth)
		{
			GameClient()->m_TripleHookHolder.HandleAutoTripleFlyThrowBoth();
		}

		GameClient()->m_Pseudofire.RunMRXDuoPseudo();

		// MRX: Проверка радиуса для fire
		if(g_Config.m_MRXFireCheckRadius)
		{
			int LocalId = GameClient()->m_aLocalIds[g_Config.m_ClDummy];
			if(LocalId >= 0 && GameClient()->m_Snap.m_pLocalCharacter && GameClient()->m_Snap.m_pLocalCharacter->m_Weapon == WEAPON_HAMMER)
			{
				vec2 LocalPos = GameClient()->m_aClients[LocalId].m_Predicted.m_Pos;
				bool HasPlayerNearby = false;
				// Серверная геометрия удара молотом: центр круга = LocalPos + AimDir * (0.75R),
				// попадание по центрам: <= 1.5R. Берём R=28 и без запаса.
				const float R = 28.0f;
				const float CenterOffset = 0.75f * R; // 21
				const float HitRadiusCenters = 1.5f * R; // 42
				vec2 AimDir = normalize(vec2(m_aInputData[g_Config.m_ClDummy].m_TargetX, m_aInputData[g_Config.m_ClDummy].m_TargetY));
				if(length(AimDir) < 1e-3f)
					AimDir = vec2(1.f, 0.f);
				vec2 ProjStartPos = LocalPos + AimDir * CenterOffset;

				// Проверяем всех клиентов
				for(int i = 0; i < MAX_CLIENTS; i++)
				{
					if(i == LocalId)
						continue;

					if(!GameClient()->m_aClients[i].m_Active)
						continue;

					vec2 PlayerPos = GameClient()->m_aClients[i].m_Predicted.m_Pos;
					float DistToHitCenter = distance(ProjStartPos, PlayerPos);
					if(DistToHitCenter <= HitRadiusCenters)
					{
						HasPlayerNearby = true;
						break;
					}
				}

				// Применяем желаемое состояние fire только если есть игрок в радиусе
				if(HasPlayerNearby)
				{
					// Если желаемое состояние активно и текущее отличается - применяем
					if((m_aDesiredFire[g_Config.m_ClDummy] & 1) != 0 && (m_aInputData[g_Config.m_ClDummy].m_Fire & 1) == 0)
					{
						m_aInputData[g_Config.m_ClDummy].m_Fire++;
						m_aInputData[g_Config.m_ClDummy].m_Fire &= INPUT_STATE_MASK;
					}
				}
				else
				{
					// Если нет игрока в радиусе, fire должен быть отключен
					if((m_aInputData[g_Config.m_ClDummy].m_Fire & 1) != 0)
					{
						m_aInputData[g_Config.m_ClDummy].m_Fire++;
						m_aInputData[g_Config.m_ClDummy].m_Fire &= INPUT_STATE_MASK;
					}
				}
			}
		}

		if(g_Config.m_MRXAutoHookDummy)
		{
			// При активации команды hook проверяем цель
			if(m_aInputData[g_Config.m_ClDummy].m_Hook)
			{
				// Если не нашли цель, отключаем хук
				if(!GameClient()->m_AimHookDummy.HandleHook())
				{
					m_aInputData[g_Config.m_ClDummy].m_Hook = 0;
				}
			}
		}



		// stress testing
#ifdef CONF_DEBUG
		if(g_Config.m_DbgStress)
		{
			float t = Client()->LocalTime();
			mem_zero(&m_aInputData[g_Config.m_ClDummy], sizeof(m_aInputData[0]));

			m_aInputData[g_Config.m_ClDummy].m_Direction = ((int)t / 2) & 1;
			m_aInputData[g_Config.m_ClDummy].m_Jump = ((int)t);
			m_aInputData[g_Config.m_ClDummy].m_Fire = ((int)(t * 10));
			m_aInputData[g_Config.m_ClDummy].m_Hook = ((int)(t * 2)) & 1;
			m_aInputData[g_Config.m_ClDummy].m_WantedWeapon = ((int)t) % NUM_WEAPONS;
			m_aInputData[g_Config.m_ClDummy].m_TargetX = (int)(std::sin(t * 3) * 100.0f);
			m_aInputData[g_Config.m_ClDummy].m_TargetY = (int)(std::cos(t * 3) * 100.0f);
		}
#endif
		// check if we need to send input
		Send = Send || m_aInputData[g_Config.m_ClDummy].m_Direction != m_aLastData[g_Config.m_ClDummy].m_Direction;
		Send = Send || m_aInputData[g_Config.m_ClDummy].m_Jump != m_aLastData[g_Config.m_ClDummy].m_Jump;
		Send = Send || m_aInputData[g_Config.m_ClDummy].m_Fire != m_aLastData[g_Config.m_ClDummy].m_Fire;
		Send = Send || m_aInputData[g_Config.m_ClDummy].m_Hook != m_aLastData[g_Config.m_ClDummy].m_Hook;
		Send = Send || m_aInputData[g_Config.m_ClDummy].m_WantedWeapon != m_aLastData[g_Config.m_ClDummy].m_WantedWeapon;
		Send = Send || m_aInputData[g_Config.m_ClDummy].m_NextWeapon != m_aLastData[g_Config.m_ClDummy].m_NextWeapon;
		Send = Send || m_aInputData[g_Config.m_ClDummy].m_PrevWeapon != m_aLastData[g_Config.m_ClDummy].m_PrevWeapon;
		Send = Send || time_get() > m_LastSendTime + time_freq() / 25; // send at least 25 Hz
		Send = Send || (GameClient()->m_Snap.m_pLocalCharacter && GameClient()->m_Snap.m_pLocalCharacter->m_Weapon == WEAPON_NINJA && (m_aInputData[g_Config.m_ClDummy].m_Direction || m_aInputData[g_Config.m_ClDummy].m_Jump || m_aInputData[g_Config.m_ClDummy].m_Hook));
	}

		GameClient()->m_AdvancedDeepFly.HandleAdvancedDeepFlyFire();
	GameClient()->m_AdvancedDeepFly.HandleAdvancedDeepFly();

	// copy and return size
	m_aLastData[g_Config.m_ClDummy] = m_aInputData[g_Config.m_ClDummy];

	if(!Send)
		return 0;

	m_LastSendTime = time_get();
	mem_copy(pData, &m_aInputData[g_Config.m_ClDummy], sizeof(m_aInputData[0]));
	return sizeof(m_aInputData[0]);
}

void CControls::OnRender()
{
	if(Client()->State() != IClient::STATE_ONLINE && Client()->State() != IClient::STATE_DEMOPLAYBACK)
		return;

	if(g_Config.m_ClAutoswitchWeaponsOutOfAmmo && !GameClient()->m_GameInfo.m_UnlimitedAmmo && GameClient()->m_Snap.m_pLocalCharacter)
	{
		// Keep track of ammo count, we know weapon ammo only when we switch to that weapon, this is tracked on server and protocol does not track that
		m_aAmmoCount[maximum(0, GameClient()->m_Snap.m_pLocalCharacter->m_Weapon % NUM_WEAPONS)] = GameClient()->m_Snap.m_pLocalCharacter->m_AmmoCount;
		// Autoswitch weapon if we're out of ammo
		if(m_aInputData[g_Config.m_ClDummy].m_Fire % 2 != 0 &&
			GameClient()->m_Snap.m_pLocalCharacter->m_AmmoCount == 0 &&
			GameClient()->m_Snap.m_pLocalCharacter->m_Weapon != WEAPON_HAMMER &&
			GameClient()->m_Snap.m_pLocalCharacter->m_Weapon != WEAPON_NINJA)
		{
			int Weapon;
			for(Weapon = WEAPON_LASER; Weapon > WEAPON_GUN; Weapon--)
			{
				if(Weapon == GameClient()->m_Snap.m_pLocalCharacter->m_Weapon)
					continue;
				if(m_aAmmoCount[Weapon] > 0)
					break;
			}
			if(Weapon != GameClient()->m_Snap.m_pLocalCharacter->m_Weapon)
				m_aInputData[g_Config.m_ClDummy].m_WantedWeapon = Weapon + 1;
		}
	}

	// update target pos
	if(GameClient()->m_Snap.m_pGameInfoObj && !GameClient()->m_Snap.m_SpecInfo.m_Active)
	{
		// make sure to compensate for smooth dyncam to ensure the cursor stays still in world space if zoomed
		vec2 DyncamOffsetDelta = GameClient()->m_Camera.m_DyncamTargetCameraOffset - GameClient()->m_Camera.m_aDyncamCurrentCameraOffset[g_Config.m_ClDummy];
		float Zoom = GameClient()->m_Camera.m_Zoom;
		m_aTargetPos[g_Config.m_ClDummy] = GameClient()->m_LocalCharacterPos + m_aMousePos[g_Config.m_ClDummy] - DyncamOffsetDelta + DyncamOffsetDelta / Zoom;
	}
	else if(GameClient()->m_Snap.m_SpecInfo.m_Active && GameClient()->m_Snap.m_SpecInfo.m_UsePosition)
	{
		m_aTargetPos[g_Config.m_ClDummy] = GameClient()->m_Snap.m_SpecInfo.m_Position + m_aMousePos[g_Config.m_ClDummy];
	}
	else
	{
		m_aTargetPos[g_Config.m_ClDummy] = m_aMousePos[g_Config.m_ClDummy];
	}
}

bool CControls::OnCursorMove(float x, float y, IInput::ECursorType CursorType)
{
	if(GameClient()->m_Snap.m_pGameInfoObj && (GameClient()->m_Snap.m_pGameInfoObj->m_GameStateFlags & GAMESTATEFLAG_PAUSED))
		return false;

	if(CursorType == IInput::CURSOR_JOYSTICK && g_Config.m_InpControllerAbsolute && GameClient()->m_Snap.m_pGameInfoObj && !GameClient()->m_Snap.m_SpecInfo.m_Active)
	{
		vec2 AbsoluteDirection;
		if(Input()->GetActiveJoystick()->Absolute(&AbsoluteDirection.x, &AbsoluteDirection.y))
			m_aMousePos[g_Config.m_ClDummy] = AbsoluteDirection * GetMaxMouseDistance();
		return true;
	}

	float Factor = 1.0f;
	if(g_Config.m_ClDyncam && g_Config.m_ClDyncamMousesens)
	{
		Factor = g_Config.m_ClDyncamMousesens / 100.0f;
	}
	else
	{
		switch(CursorType)
		{
		case IInput::CURSOR_MOUSE:
			Factor = g_Config.m_InpMousesens / 100.0f;
			break;
		case IInput::CURSOR_JOYSTICK:
			Factor = g_Config.m_InpControllerSens / 100.0f;
			break;
		default:
			dbg_msg("assert", "CControls::OnCursorMove CursorType %d", (int)CursorType);
			dbg_break();
			break;
		}
	}

	if(GameClient()->m_Snap.m_SpecInfo.m_Active && GameClient()->m_Snap.m_SpecInfo.m_SpectatorId < 0)
		Factor *= GameClient()->m_Camera.m_Zoom;

	m_aMousePos[g_Config.m_ClDummy] += vec2(x, y) * Factor;
	ClampMousePos();
	return true;
}

void CControls::ClampMousePos()
{
	if(GameClient()->m_Snap.m_SpecInfo.m_Active && GameClient()->m_Snap.m_SpecInfo.m_SpectatorId < 0)
	{
		m_aMousePos[g_Config.m_ClDummy].x = std::clamp(m_aMousePos[g_Config.m_ClDummy].x, -201.0f * 32, (Collision()->GetWidth() + 201.0f) * 32.0f);
		m_aMousePos[g_Config.m_ClDummy].y = std::clamp(m_aMousePos[g_Config.m_ClDummy].y, -201.0f * 32, (Collision()->GetHeight() + 201.0f) * 32.0f);
	}
	else
	{
		const float MouseMin = GetMinMouseDistance();
		const float MouseMax = GetMaxMouseDistance();

		float MouseDistance = length(m_aMousePos[g_Config.m_ClDummy]);
		if(MouseDistance < 0.001f)
		{
			m_aMousePos[g_Config.m_ClDummy].x = 0.001f;
			m_aMousePos[g_Config.m_ClDummy].y = 0;
			MouseDistance = 0.001f;
		}
		if(MouseDistance < MouseMin)
			m_aMousePos[g_Config.m_ClDummy] = normalize_pre_length(m_aMousePos[g_Config.m_ClDummy], MouseDistance) * MouseMin;
		MouseDistance = length(m_aMousePos[g_Config.m_ClDummy]);
		if(MouseDistance > MouseMax)
			m_aMousePos[g_Config.m_ClDummy] = normalize_pre_length(m_aMousePos[g_Config.m_ClDummy], MouseDistance) * MouseMax;
	}
}

float CControls::GetMinMouseDistance() const
{
	return g_Config.m_ClDyncam ? g_Config.m_ClDyncamMinDistance : g_Config.m_ClMouseMinDistance;
}

float CControls::GetMaxMouseDistance() const
{
	float CameraMaxDistance = 200.0f;
	float FollowFactor = (g_Config.m_ClDyncam ? g_Config.m_ClDyncamFollowFactor : g_Config.m_ClMouseFollowfactor) / 100.0f;
	float DeadZone = g_Config.m_ClDyncam ? g_Config.m_ClDyncamDeadzone : g_Config.m_ClMouseDeadzone;
	float MaxDistance = g_Config.m_ClDyncam ? g_Config.m_ClDyncamMaxDistance : g_Config.m_ClMouseMaxDistance;
	return minimum((FollowFactor != 0 ? CameraMaxDistance / FollowFactor + DeadZone : MaxDistance), MaxDistance);
}
