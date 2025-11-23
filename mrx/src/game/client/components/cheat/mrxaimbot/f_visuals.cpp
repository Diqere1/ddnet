#include "f_helper.h"
#include "f_visuals.h"


void FVisuals::Run(int ClientID, float Angle, vec2 Position)
{
	if(!g_pMrxAimbotHelper->m_pConfig->espCfg.enabled)
		return;
	Graphics()->TextureClear();
	
	// Draw ESP Box around player
	if(g_pMrxAimbotHelper->m_pConfig->espCfg.drawBox && ClientID >= 0 && ClientID != m_pClient->m_Snap.m_LocalClientId)
	{
		ColorRGBA BoxColor(1.0f, 0.0f, 0.0f, 1.0f); // Default: red

		if(FAimbot *pAimbot = g_pMrxAimbotHelper->m_pAimbot.get())
		{
			if(pAimbot->m_TargetVisible && pAimbot->m_TargetId == ClientID)
			{
				BoxColor = ColorRGBA(0.0f, 1.0f, 0.0f, 1.0f); // Green when aimbot can hook this player
			}
		}

		DrawBox(Position, 32.0f, BoxColor); // 32 is standard tee size
	}

	// Draw Glow only for hookable player and from player center
	if(g_pMrxAimbotHelper->m_pConfig->espCfg.drawGlow &&
	   g_pMrxAimbotHelper->m_pConfig->aimbotCfg.enabled &&
	   ClientID >= 0 && ClientID != m_pClient->m_Snap.m_LocalClientId)
	{
		if(FAimbot *pAimbot = g_pMrxAimbotHelper->m_pAimbot.get())
		{
			if(pAimbot->m_TargetVisible && pAimbot->m_TargetId == ClientID)
			{
				DrawGlowPlayer(Position, ColorRGBA(0.0f, 1.0f, 0.0f, 1.0f));
			}
		}
	}
	
	DrawFov();
}


void FVisuals::DrawFov()
{
	if(!(g_pMrxAimbotHelper->m_pConfig->espCfg.drawFov &&
	     g_pMrxAimbotHelper->m_pConfig->aimbotCfg.enabled))
		return;

    DrawFovLine(g_pMrxAimbotHelper->m_pConfig->aimbotCfg.fov * 0.01f);
    DrawFovLine(g_pMrxAimbotHelper->m_pConfig->aimbotCfg.fov * -0.01f);
}

void FVisuals::DrawFovLine(float offset)
{
	const float Angle = angle(Controls()->m_aMousePos[g_Config.m_ClDummy]) + offset;
	const vec2 ExDirection = normalize(direction(Angle));

	const vec2 InitPos = m_pClient->m_LocalCharacterPos;
	vec2 FinishPos = InitPos + ExDirection * (Tuning()->m_HookLength);

	int teleNr = 0;
	Collision()->IntersectLineTeleHook(InitPos, FinishPos, &FinishPos, nullptr, &teleNr);

	const ColorRGBA HookCollColor(0.0f, 0.0f, 0.0f, 0.5f);
	DrawLine(InitPos, FinishPos, HookCollColor);
}
