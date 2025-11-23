#include "pseudofire.h"
#include <game/client/gameclient.h>
#include <base/system.h>
#include <cmath>
#include <algorithm>

void CPseudofire::OnRender()
{
	if(g_Config.m_MRXDuoPseudoTarget && IsValidTarget(m_CurrentTargetId))
	{
		vec2 TargetPos = GameClient()->m_aClients[m_CurrentTargetId].m_Predicted.m_Pos;
		const float SmoothFactor = 0.3f;
		if(length(m_ArrowRenderPos) < 1.0f)
			m_ArrowRenderPos = TargetPos;
		else
			m_ArrowRenderPos += (TargetPos - m_ArrowRenderPos) * SmoothFactor;
		RenderArrow(m_ArrowRenderPos);
	}
	else
	{
		m_ArrowRenderPos = vec2(0, 0);
	}
}

void CPseudofire::RunMRXDuoPseudo()
{
    if(!g_Config.m_MRXDuoPseudo)
        return;

    if(!GameClient()->m_Snap.m_pLocalCharacter || GameClient()->m_Snap.m_pLocalCharacter->m_Weapon != WEAPON_HAMMER)
        return;

    bool FireHeld = false;
    {
        char aFireKey[64] = {0};
        GameClient()->m_Binds.GetKey("+fire", aFireKey, sizeof(aFireKey));
        int FireKeyId = GameClient()->Input()->FindKeyByName(aFireKey);
        if(FireKeyId != KEY_UNKNOWN && GameClient()->Input()->KeyIsPressed(FireKeyId))
            FireHeld = true;

  if(!FireHeld)
  {
   GameClient()->m_Binds.GetKey("+fire; +toggle cl_dummy_hammer 1 0", aFireKey, sizeof(aFireKey));
   FireKeyId = GameClient()->Input()->FindKeyByName(aFireKey);
   if(FireKeyId != KEY_UNKNOWN && GameClient()->Input()->KeyIsPressed(FireKeyId))
    FireHeld = true;
  }
    }
    if(!FireHeld)
        return;

    vec2 TargetPos;
    if(!FindTarget(TargetPos))
    {
        if(g_Config.m_MRXDuoPseudoFire)
        {
            GameClient()->m_Controls.m_aInputData[g_Config.m_ClDummy].m_Fire = (GameClient()->m_Controls.m_aLastData[g_Config.m_ClDummy].m_Fire & ~1) & INPUT_STATE_MASK;
        }
        return;
    }

    int LocalId = GameClient()->m_Snap.m_LocalClientId;
    vec2 LocalPos = GameClient()->m_aClients[LocalId].m_Predicted.m_Pos;
    float Dist = distance(LocalPos, TargetPos);
    if(!(Dist > 0.f && Dist < 63.0f))
    {
        if(g_Config.m_MRXDuoPseudoFire)
        {
            GameClient()->m_Controls.m_aInputData[g_Config.m_ClDummy].m_Fire = (GameClient()->m_Controls.m_aLastData[g_Config.m_ClDummy].m_Fire & ~1) & INPUT_STATE_MASK;
        }
        return;
    }

    vec2 DirToTarget = normalize(TargetPos - LocalPos);
    if(g_Config.m_MRXDuoPseudoAutoAim)
    {
        GameClient()->m_Controls.m_aInputData[g_Config.m_ClDummy].m_TargetX = (int)(DirToTarget.x * 100.0f);
        GameClient()->m_Controls.m_aInputData[g_Config.m_ClDummy].m_TargetY = (int)(DirToTarget.y * 100.0f);
    }
    else
    {
        // Геометрическая проверка удара молотом без автоприцела (как на сервере)
        // Центр круга удара: LocalPos + AimDir * (0.75R), радиус проверки по центрам: 0.5R + R
        const float R = 28.0f;
        const float CenterOffset = 0.75f * R; // 21
        const float HitRadiusCenters = 1.5f * R; // 42
        const float Epsilon = 0.0f; // небольшой запас под предикцию

        vec2 AimDir = normalize(vec2(GameClient()->m_Controls.m_aInputData[g_Config.m_ClDummy].m_TargetX,
                                      GameClient()->m_Controls.m_aInputData[g_Config.m_ClDummy].m_TargetY));
        if(length(AimDir) < 1e-3f)
            AimDir = vec2(1.f, 0.f);

        vec2 ProjStartPos = LocalPos + AimDir * CenterOffset;
        float HitDist = distance(ProjStartPos, TargetPos);
        if(HitDist > (HitRadiusCenters + Epsilon))
        {
            if(g_Config.m_MRXDuoPseudoFire)
            {
                GameClient()->m_Controls.m_aInputData[g_Config.m_ClDummy].m_Fire = (GameClient()->m_Controls.m_aLastData[g_Config.m_ClDummy].m_Fire & ~1) & INPUT_STATE_MASK;
            }
            return;
        }
    }

    int64_t Now = time_get();
    int64_t Cooldown = (int64_t)(time_freq() * (GameClient()->GetTuning(0)->m_HammerHitFireDelay / 1000.0f));
    if(Now - m_LastHammerTime > Cooldown)
    {
        GameClient()->m_Controls.m_aInputData[g_Config.m_ClDummy].m_Fire = (GameClient()->m_Controls.m_aInputData[g_Config.m_ClDummy].m_Fire + 1) | 1;
        if(GameClient()->m_Snap.m_pLocalCharacter->m_Weapon != WEAPON_HAMMER)
            GameClient()->m_Controls.m_aInputData[g_Config.m_ClDummy].m_WantedWeapon = WEAPON_HAMMER + 1;
        m_LastHammerTime = Now;
    }
}
void CPseudofire::RenderArrow(const vec2 &Pos) const
{
	float Points[4];
	Graphics()->GetScreen(Points, Points + 1, Points + 2, Points + 3);

	const float Size = 16.0f;
	const float Height = 42.0f;

	CUIRect ArrowRect = {
		Pos.x - Size / 2.0f,
		Pos.y - Height,
		Size,
		Size};

	Graphics()->TextureClear();
	Graphics()->QuadsBegin();
	Graphics()->SetColor(1.0f, 0.2f, 0.2f, 1.0f); // красная стрелка

	vec2 V[3] = {
		vec2(ArrowRect.x + ArrowRect.w / 2, ArrowRect.y + ArrowRect.h),
		vec2(ArrowRect.x, ArrowRect.y),
		vec2(ArrowRect.x + ArrowRect.w, ArrowRect.y)};

	IGraphics::CFreeformItem Freeform(
		V[0].x, V[0].y,
		V[1].x, V[1].y,
		V[2].x, V[2].y,
		V[2].x, V[2].y);
	Graphics()->QuadsDrawFreeform(&Freeform, 1);
	Graphics()->QuadsEnd();

	Graphics()->MapScreen(Points[0], Points[1], Points[2], Points[3]);
}
bool CPseudofire::IsValidTarget(int ClientId) const
{
	if(ClientId < 0 || ClientId >= MAX_CLIENTS)
		return false;

	if(ClientId == GameClient()->m_Snap.m_LocalClientId) // сам себя не таргетим
		return false;

	const CGameClient::CClientData &Client = GameClient()->m_aClients[ClientId];
	if(!Client.m_Active || GameClient()->IsOtherTeam(ClientId))
		return false;

	return true;
}

bool CPseudofire::FindTarget(vec2 &TargetPos)
{
	if(!g_Config.m_MRXDuoPseudoTarget)
		m_CurrentTargetId = -1;

	int LocalId = GameClient()->m_Snap.m_LocalClientId;
	vec2 LocalPos = GameClient()->m_aClients[LocalId].m_Predicted.m_Pos;

	// 1. Проверить, действительна ли текущая цель
		if(IsValidTarget(m_CurrentTargetId))
		{
		vec2 EnemyPos = GameClient()->m_aClients[m_CurrentTargetId].m_Predicted.m_Pos;
		TargetPos = EnemyPos;
			return true;
		}

	// 2. Если текущая цель недействительна, найти новую
	float MinAngle = pi;
		int BestId = -1;

	vec2 AimDir = normalize(vec2(GameClient()->m_Controls.m_aInputData[g_Config.m_ClDummy].m_TargetX, GameClient()->m_Controls.m_aInputData[g_Config.m_ClDummy].m_TargetY));

		for(int i = 0; i < MAX_CLIENTS; i++)
		{
			if(!IsValidTarget(i))
				continue;

		vec2 EnemyPos = GameClient()->m_aClients[i].m_Predicted.m_Pos;

		if(distance(LocalPos, EnemyPos) > 63.0f)
			continue;

		vec2 EnemyDir = normalize(EnemyPos - LocalPos);
		float Angle = acosf(std::clamp(dot(AimDir, EnemyDir), -1.0f, 1.0f));

		if(Angle < MinAngle)
			{
			MinAngle = Angle;
				BestId = i;
			TargetPos = EnemyPos;
			}
		}

		m_CurrentTargetId = BestId;
		return BestId != -1;
	}
