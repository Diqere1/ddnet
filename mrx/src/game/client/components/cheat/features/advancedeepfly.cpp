#include "advancedeepfly.h"
#include <algorithm>
#include <base/vmath.h>
#include <engine/shared/config.h>
#include <game/client/gameclient.h>

void CAdvancedDeepFly::OnRender()
{
	if(g_Config.m_MRXAdvancedDeepFly)
		HandleAdvancedDeepFlyFire();

	static bool s_WasActive = false;
	bool Active = g_Config.m_MRXAdvancedDeepFlyManual;

	if(!Active && s_WasActive)
	{
		GameClient()->ReleaseFireInputs();
		m_NeedReleaseFire = true;
	}
	else if(m_NeedReleaseFire)
	{
		m_NeedReleaseFire = false;
	}
	else if(Active)
	{
		HandleAdvancedDeepFly();
	}

	s_WasActive = Active;
}

void CAdvancedDeepFly::HandleAdvancedDeepFlyFire()
{
	if(!g_Config.m_MRXAdvancedDeepFly)
		return;

	static int s_TargetHoldTicks = 0;
	static vec2 s_OriginalTarget;

	bool FireHeld = false;
	{
		char aFireKey[64] = {0};
		GameClient()->m_Binds.GetKey("+fire", aFireKey, sizeof(aFireKey));
		int FireKeyId = GameClient()->Input()->FindKeyByName(aFireKey);
		if(FireKeyId != KEY_UNKNOWN && GameClient()->Input()->KeyIsPressed(FireKeyId))
			FireHeld = true;
	}

	int DummyId = GameClient()->m_aLocalIds[!g_Config.m_ClDummy];
	if(DummyId < 0)
		return;

	int LocalId = GameClient()->m_aLocalIds[g_Config.m_ClDummy];
	if(LocalId < 0)
		return;

	vec2 LocalPos = GameClient()->m_aClients[LocalId].m_RenderPos;
	vec2 DummyPos = GameClient()->m_aClients[DummyId].m_RenderPos;

	if(FireHeld)
	{
		if(!GameClient()->m_Snap.m_pLocalCharacter)
			return;

		// Серверная геометрия удара молотом: центр круга = LocalPos + AimDir * (0.75R),
		// попадание по центрам: <= 1.5R. Берём R=28, без запаса.
		const float R = 28.0f;
		const float CenterOffset = 0.75f * R; // 21
		const float HitRadiusCenters = 1.5f * R; // 42
		vec2 AimDir = normalize(vec2(GameClient()->m_Controls.m_aInputData[g_Config.m_ClDummy].m_TargetX,
			GameClient()->m_Controls.m_aInputData[g_Config.m_ClDummy].m_TargetY));
		if(length(AimDir) < 1e-3f)
			AimDir = normalize(DummyPos - LocalPos);
		vec2 ProjStartPos = LocalPos + AimDir * CenterOffset;
		if(distance(ProjStartPos, DummyPos) > HitRadiusCenters)
			return;

		GameClient()->m_Controls.m_aInputData[g_Config.m_ClDummy].m_Fire++;

		if(s_TargetHoldTicks == 0)
		{
			s_OriginalTarget.x = GameClient()->m_DummyInput.m_TargetX;
			s_OriginalTarget.y = GameClient()->m_DummyInput.m_TargetY;
		}

		// Aim for dummy
		const vec2 Dir = LocalPos - DummyPos;
		GameClient()->m_DummyInput.m_TargetX = (int)Dir.x;
		GameClient()->m_DummyInput.m_TargetY = (int)Dir.y;
		GameClient()->m_DummyInput.m_Fire = (GameClient()->m_DummyInput.m_Fire + 1) | 1;
		s_TargetHoldTicks = g_Config.m_MRXTargetHit;
	}
	else
	{
		if((GameClient()->m_DummyInput.m_Fire & 1) != 0)
			GameClient()->m_DummyInput.m_Fire++;
	}

	if(s_TargetHoldTicks > 0)
	{
		const vec2 Dir = LocalPos - DummyPos;
		GameClient()->m_DummyInput.m_TargetX = (int)Dir.x;
		GameClient()->m_DummyInput.m_TargetY = (int)Dir.y;
		--s_TargetHoldTicks;

		if(s_TargetHoldTicks == 0)
		{
			GameClient()->m_DummyInput.m_TargetX = s_OriginalTarget.x;
			GameClient()->m_DummyInput.m_TargetY = s_OriginalTarget.y;
		}
	}
}

void CAdvancedDeepFly::HandleAdvancedDeepFly()
{
	if(!g_Config.m_MRXAdvancedDeepFlyManual || !GameClient()->m_Snap.m_pLocalCharacter)
		return;

	int DummyId = GameClient()->m_aLocalIds[!g_Config.m_ClDummy];
	if(DummyId < 0)
		return;

	int LocalId = GameClient()->m_aLocalIds[g_Config.m_ClDummy];
	if(LocalId < 0)
		return;

	vec2 LocalPos = GameClient()->m_aClients[LocalId].m_RenderPos;
	vec2 DummyPos = GameClient()->m_aClients[DummyId].m_RenderPos;

	// Серверная геометрия удара молотом: центр круга = LocalPos + AimDir * (0.75R),
	// попадание по центрам: <= 1.5R. Берём R=28, без запаса.
	const float R = 28.0f;
	const float CenterOffset = 0.75f * R; // 21
	const float HitRadiusCenters = 1.5f * R; // 42
	vec2 AimDir = normalize(vec2(GameClient()->m_Controls.m_aInputData[g_Config.m_ClDummy].m_TargetX,
		GameClient()->m_Controls.m_aInputData[g_Config.m_ClDummy].m_TargetY));
	if(length(AimDir) < 1e-3f)
		AimDir = normalize(DummyPos - LocalPos);
	vec2 ProjStartPos = LocalPos + AimDir * CenterOffset;
	if(distance(ProjStartPos, DummyPos) > HitRadiusCenters)
		return;

	vec2 Direction = normalize(DummyPos - LocalPos);

	vec2 OriginalAim = vec2(GameClient()->m_Controls.m_aInputData[g_Config.m_ClDummy].m_TargetX,
		GameClient()->m_Controls.m_aInputData[g_Config.m_ClDummy].m_TargetY);

	float CurrentMag = length(OriginalAim);

	if(CurrentMag < 0.001f)
	{
		OriginalAim = Direction * 200.0f;
	}

	// Серверная геометрия удара молотом вместо углового порога:
	// если текущий Aim не даёт попадания в круг (LocalPos + AimDir * 0.75R, радиус 1.5R),
	// только тогда корректируем Aim на Direction к Dummy
	vec2 AimDirNow = normalize(OriginalAim);
	if(length(AimDirNow) < 1e-3f)
		AimDirNow = Direction;
	vec2 ProjStartPosNow = LocalPos + AimDirNow * CenterOffset;
	bool AimCorrectionNeeded = distance(ProjStartPosNow, DummyPos) > HitRadiusCenters;

	if(AimCorrectionNeeded)
	{
		float TargetMag = maximum(CurrentMag, 200.0f);

		vec2 ShiftedAim = Direction * TargetMag;

		GameClient()->m_Controls.m_aInputData[g_Config.m_ClDummy].m_TargetX = static_cast<int>(ShiftedAim.x);
		GameClient()->m_Controls.m_aInputData[g_Config.m_ClDummy].m_TargetY = static_cast<int>(ShiftedAim.y);
	}

	GameClient()->m_Controls.m_aInputData[g_Config.m_ClDummy].m_Fire++;

	GameClient()->m_DummyInput.m_TargetX = (-Direction.x * 100.0f);
	GameClient()->m_DummyInput.m_TargetY = (-Direction.y * 100.0f);
	GameClient()->m_DummyInput.m_Fire++;
}
