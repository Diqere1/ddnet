#include "hookfly.h"
#include <game/client/gameclient.h>
#include <game/gamecore.h>
#include <engine/shared/config.h>
#include <base/math.h>

void CHookFly::HandleHookFly()
{
	// Обработка автохукфлая
	if(g_Config.m_MRXHookFlyAuto)
	{
		HandleAutoHookFly();
	}
	else if(!g_Config.m_MRXHookFlyAuto && m_LastAutoHookState)
	{
		// Если автохук был включен и теперь выключен - сбросить хук
		ActivateDummyHook(false);
		m_LastAutoHookState = false;
	}
	
	// Обработка ручного хукфлая
	if(g_Config.m_MRXHookFlyManual)
	{
		HandleManualHookFly();
	}
	else if(!g_Config.m_MRXHookFlyManual && m_LastManualHookState)
	{
		// Если ручной режим выключен - сбросить хук
		ActivateDummyHook(false);
		m_LastManualHookState = false;
	}
	
}

void CHookFly::HandleAutoHookFly()
{
	vec2 PlayerPos, DummyPos;
	int DummyId;
	
	if(!GetPlayerAndDummyPositions(PlayerPos, DummyPos, DummyId))
	{
		m_LastAutoHookState = false;
		return;
	}
	
	// Проверяем дистанцию между игроком и дамми
	float YDifference = PlayerPos.y - DummyPos.y; // Положительное значение = игрок ниже

	// Получаем состояние хука дамми из предсказания
	int DummyHookState = GameClient()->m_aClients[DummyId].m_Predicted.m_HookState;

	// Логика:
	// 1. Начинаем хукать, когда игрок ниже на заданную дистанцию
	// 2. Продолжаем хукать, пока игрок не сравняется с дамми или не станет выше
	// 3. Перехукиваем, когда хук закончился (после полного отпускания хука)
	bool ShouldStartHook = YDifference > g_Config.m_MRXHookFlyAutoDistance; // Условие начала хука
	bool ShouldKeepHook = YDifference > 0; // Условие продолжения хука (игрок все еще ниже)

	// Состояния хука
	bool HookRetracting = (DummyHookState >= HOOK_RETRACT_START && DummyHookState <= HOOK_RETRACT_END);
	bool HookRetracted = (DummyHookState == HOOK_RETRACTED);
	bool HookReady = (DummyHookState == HOOK_IDLE);

	if(!ShouldKeepHook)
	{
		// Игрок не ниже - полностью сбрасываем состояние
		if(m_LastAutoHookState || m_AutoRehookQueued)
		{
			ActivateDummyHook(false);
			m_LastAutoHookState = false;
			m_AutoRehookQueued = false;
		}
		return;
	}

	vec2 Direction = normalize(PlayerPos - DummyPos);

	// Если хук начал втягиваться или уже втянут — отпускаем и готовим перехук
	if(m_LastAutoHookState && (HookRetracting || HookRetracted))
	{
		ActivateDummyHook(false);
		m_LastAutoHookState = false;
		m_AutoRehookQueued = true;
	}

	bool WantToHook = ShouldStartHook || m_LastAutoHookState || m_AutoRehookQueued;

	if(WantToHook)
	{
		SetDummyHookTarget(Direction);
	}

	// Если нужно начать хук (первый раз или после перехука)
	if(WantToHook && !m_LastAutoHookState)
	{
		if(HookReady)
		{
			ActivateDummyHook(true);
			m_LastAutoHookState = true;
			m_AutoRehookQueued = false;
		}
		else
		{
			m_AutoRehookQueued = true;
		}
	}

	// Удерживаем хук активным каждый тик
	if(m_LastAutoHookState && !HookRetracting && !HookRetracted)
	{
		ActivateDummyHook(true);
	}
}

void CHookFly::HandleManualHookFly()
{
	vec2 PlayerPos, DummyPos;
	int DummyId;
	
	if(!GetPlayerAndDummyPositions(PlayerPos, DummyPos, DummyId))
	{
		m_LastManualHookState = false;
		return;
	}
	
	// Вычисляем направление к игроку
	vec2 Direction = normalize(PlayerPos - DummyPos);
	
	// Устанавливаем цель хука на игрока
	SetDummyHookTarget(Direction);
	
	// Активируем хук
	ActivateDummyHook(true);
	m_LastManualHookState = true;
}

bool CHookFly::GetPlayerAndDummyPositions(vec2 &PlayerPos, vec2 &DummyPos, int &DummyId)
{
	// Проверяем, что мы в игре
	if(!GameClient()->m_Snap.m_pLocalCharacter)
		return false;
	
	// Получаем ID дамми
	DummyId = GameClient()->m_aLocalIds[!g_Config.m_ClDummy];
	if(DummyId < 0)
		return false;
	
	// Получаем позиции
	PlayerPos = GameClient()->m_LocalCharacterPos;
	DummyPos = GameClient()->m_aClients[DummyId].m_Predicted.m_Pos;
	
	return true;
}

bool CHookFly::IsPlayerBelowDummy(const vec2 &PlayerPos, const vec2 &DummyPos)
{
	// Игрок ниже, если его Y координата больше (в Teeworlds Y растет вниз)
	return PlayerPos.y > DummyPos.y;
}

void CHookFly::SetDummyHookTarget(const vec2 &Direction)
{
	// Устанавливаем цель хука дамми
	CNetObj_PlayerInput *pDummyInput = &GameClient()->m_DummyInput;
	pDummyInput->m_TargetX = (int)(Direction.x * 100.0f);
	pDummyInput->m_TargetY = (int)(Direction.y * 100.0f);
	GameClient()->m_Controls.m_aInputData[!g_Config.m_ClDummy].m_TargetX = pDummyInput->m_TargetX;
	GameClient()->m_Controls.m_aInputData[!g_Config.m_ClDummy].m_TargetY = pDummyInput->m_TargetY;
}

void CHookFly::ActivateDummyHook(bool Active)
{
	// Активируем или деактивируем хук дамми
	CNetObj_PlayerInput *pDummyInput = &GameClient()->m_DummyInput;
	pDummyInput->m_Hook = Active ? 1 : 0;
	GameClient()->m_Controls.m_aInputData[!g_Config.m_ClDummy].m_Hook = pDummyInput->m_Hook;
}
