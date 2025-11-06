#include "aimhookdummy.h"
#include <algorithm>
#include <base/vmath.h>
#include <engine/shared/config.h>
#include <game/client/gameclient.h>

bool CAimHookDummy::FindTarget(vec2 &TargetPos)
{
	if(GameClient()->m_Snap.m_LocalClientId < 0)
		return false;

	int DummyId = GameClient()->m_aLocalIds[!g_Config.m_ClDummy];
	if(DummyId < 0)
		return false;

	vec2 LocalPos = GameClient()->m_LocalCharacterPos;
	vec2 DummyPos = GameClient()->m_aClients[DummyId].m_Predicted.m_Pos;

	const float HookLength = 380.0f;
	float Distance = distance(LocalPos, DummyPos);
	if(Distance <= 0 || Distance >= HookLength)
		return false;

	TargetPos = DummyPos;
	return true;
}

bool CAimHookDummy::HandleHook()
{
	if(!g_Config.m_MRXAutoHookDummy || !GameClient()->m_Snap.m_pLocalCharacter)
	{
		return false;
	}

	// Проверяем, активен ли TripleThrowBoth и находится ли цель в зоне удара/1
	// Если да, то даем приоритет TripleThrowBoth - он сам установит прицел на врага
	// В противном случае mrx_autohookdummy работает нормально и аимит на дамми
	if(g_Config.m_MRXAutotripleflyThrowBoth && 
	   GameClient()->m_TripleHookHolder.IsTripleThrowBothTargetInStrikeRange())
	{
		// TripleThrowBoth имеет активную цель в зоне удара, не меняем прицел основного игрока
		// чтобы позволить TripleThrowBoth установить прицел на врага для удара
		return false;
	}

	vec2 TargetPos;
	if(FindTarget(TargetPos))
	{
		vec2 LocalPos = GameClient()->m_LocalCharacterPos;
		vec2 Direction = normalize(TargetPos - LocalPos);

		vec2 OriginalAim = vec2(GameClient()->m_Controls.m_aInputData[g_Config.m_ClDummy].m_TargetX,
			GameClient()->m_Controls.m_aInputData[g_Config.m_ClDummy].m_TargetY);

		float CurrentMag = length(OriginalAim);

		if(CurrentMag < 0.001f)
		{
			CurrentMag = 200.0f;
			OriginalAim = Direction * CurrentMag;
		}

		vec2 NormalizedOriginalAim = normalize(OriginalAim);

		float DotProduct = dot(NormalizedOriginalAim, Direction);

		DotProduct = std::clamp(DotProduct, -1.0f, 1.0f);

		float AbsAngleDiff = acos(DotProduct);

		const float HOOK_TOLERANCE = 0.15f;

		bool AimCorrectionNeeded = AbsAngleDiff > HOOK_TOLERANCE;

		if(AimCorrectionNeeded)
		{
			float TargetMag = maximum(CurrentMag, 200.0f);

			vec2 ShiftedAim = Direction * TargetMag;

			GameClient()->m_Controls.m_aInputData[g_Config.m_ClDummy].m_TargetX = static_cast<int>(ShiftedAim.x);
			GameClient()->m_Controls.m_aInputData[g_Config.m_ClDummy].m_TargetY = static_cast<int>(ShiftedAim.y);
		}

		return true;
	}

	return false;
}
void CAimHookDummy::ResetInput()
{
	GameClient()->m_Controls.m_aInputData[g_Config.m_ClDummy].m_Hook = 0;
}