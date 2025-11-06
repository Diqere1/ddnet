#include <game/client/gameclient.h>
#include <game/client/animstate.h>
#include <generated/client_data.h>
#include <game/client/prediction/entities/character.h>
#include <algorithm>

void CVisuals::DrawLine(float x1, float y1, float x2, float y2, float w, ColorRGBA color)
{
    Graphics()->TextureClear();
    Graphics()->QuadsBegin();
    Graphics()->SetColor(color);
    
    // Create a line with width by making a quad
    vec2 LineDir = normalize(vec2(x2 - x1, y2 - y1));
    vec2 Perpendicular = vec2(LineDir.y, -LineDir.x) * w;

    IGraphics::CFreeformItem Freeform(
        x1 - Perpendicular.x, y1 - Perpendicular.y, // Top left
        x1 + Perpendicular.x, y1 + Perpendicular.y, // Top right
        x2 - Perpendicular.x, y2 - Perpendicular.y, // Bottom left
        x2 + Perpendicular.x, y2 + Perpendicular.y  // Bottom right
    );
    
    Graphics()->QuadsDrawFreeform(&Freeform, 1);
    Graphics()->QuadsEnd();
}

void CVisuals::DrawBox(float x, float y, float w, float h, bool fill, bool stroke, ColorRGBA fillColor, ColorRGBA strokeColor)
{
    Graphics()->TextureClear();

    // Draw filled box
    if(fill)
    {
        Graphics()->QuadsBegin();
        Graphics()->SetColor(fillColor);
        IGraphics::CQuadItem QuadItem(x, y, w, h);
        Graphics()->QuadsDrawTL(&QuadItem, 1);
        Graphics()->QuadsEnd();
    }

    // Draw stroke/outline
    if(stroke)
    {
        Graphics()->LinesBegin();
        Graphics()->SetColor(strokeColor);
        
        // Draw 4 lines to make the box outline
        IGraphics::CLineItem Lines[] = {
            IGraphics::CLineItem(x, y, x + w, y),         // Top
            IGraphics::CLineItem(x + w, y, x + w, y + h), // Right
            IGraphics::CLineItem(x + w, y + h, x, y + h), // Bottom
            IGraphics::CLineItem(x, y + h, x, y)          // Left
        };
        
        Graphics()->LinesDraw(Lines, 4);
        Graphics()->LinesEnd();
    }
}

void CVisuals::DrawCircle(float x, float y, float radius, bool fill, bool stroke, ColorRGBA color, ColorRGBA strokeColor)
{
    const float step = 0.1f;
    const vec2 center(x, y);

    // Filled circle (triangle fan using QuadsDrawFreeform, like duration_bar.cpp)
    if(fill)
    {
        Graphics()->TextureClear();
        Graphics()->QuadsBegin();
        Graphics()->SetColor(color);

        for(float angle = 0; angle < 2 * pi; angle += step)
        {
            vec2 p1 = center;
            vec2 p2 = center + vec2(cosf(angle), sinf(angle)) * radius;
            vec2 p3 = center + vec2(cosf(angle + step), sinf(angle + step)) * radius;

            IGraphics::CFreeformItem tri(
                p1.x, p1.y,
                p2.x, p2.y,
                p3.x, p3.y,
                p1.x, p1.y // degenerate, ignored
            );
            Graphics()->QuadsDrawFreeform(&tri, 1);
        }
        Graphics()->QuadsEnd();
    }

    // Circle outline
    if(stroke)
    {
        Graphics()->TextureClear();
        Graphics()->LinesBegin();
        Graphics()->SetColor(strokeColor);

        vec2 firstPos = center + vec2(cosf(0), sinf(0)) * radius;
        vec2 prevPos = firstPos;
        for(float i = step; i <= 2 * pi + step; i += step)
        {
            vec2 newPos = center + vec2(cosf(i), sinf(i)) * radius;
            IGraphics::CLineItem line(prevPos.x, prevPos.y, newPos.x, newPos.y);
            Graphics()->LinesDraw(&line, 1);
            prevPos = newPos;
        }

        // Close the circle
        IGraphics::CLineItem line(prevPos.x, prevPos.y, firstPos.x, firstPos.y);
        Graphics()->LinesDraw(&line, 1);

        Graphics()->LinesEnd();
    }
}

void CVisuals::DrawCharacter(const CNetObj_Character *pPrev, const CNetObj_Character *pCur, CCharacter *pChar, float IntraTick, bool ShowFreezeBar, bool UseNinjaSkin)
{
    /*
    const int LocalClientId = GameClient()->m_Snap.m_LocalClientId;
    if(LocalClientId == -1 || !pPrev || !pCur)
        return;

    CTeeRenderInfo RenderInfo = GameClient()->m_aClients[LocalClientId].m_RenderInfo;
    RenderInfo.m_Size = 64.0f;
    RenderInfo.m_ColorBody.a = 1.0f;
    RenderInfo.m_ColorFeet.a = 1.0f;

    vec2 RenderPos = mix(
        vec2(pPrev->m_X, pPrev->m_Y),
        vec2(pCur->m_X, pCur->m_Y),
        IntraTick);

    CNetObj_Character Player = *pCur;
    Player.m_X = round_to_int(RenderPos.x);
    Player.m_Y = round_to_int(RenderPos.y);

    vec2 Vel = mix(
        vec2(pPrev->m_VelX / 256.0f, pPrev->m_VelY / 256.0f),
        vec2(pCur->m_VelX / 256.0f, pCur->m_VelY / 256.0f),
        IntraTick);
    Player.m_VelX = round_to_int(Vel.x * 256.0f);
    Player.m_VelY = round_to_int(Vel.y * 256.0f);
    Player.m_Angle = mix(pPrev->m_Angle, pCur->m_Angle, IntraTick);

    Player.m_Tick = pCur->m_Tick;

    bool IsFrozen = pChar && pChar->m_FreezeTime > 0;
    bool HasNinja = pCur->m_Weapon == WEAPON_NINJA;

    if(IsFrozen)
    {
        RenderInfo.m_TeeRenderFlags |= TEE_EFFECT_FROZEN | TEE_NO_WEAPON;

        if(ShowFreezeBar && g_Config.m_ClShowFreezeBars && pChar->Core()->m_FreezeEnd > pChar->Core()->m_FreezeStart)
        {
            const float FreezeBarWidth = 64.0f;
            const float FreezeBarHeight = 16.0f;
            vec2 BarPos = RenderPos;
            BarPos.y += 32.0f;
            BarPos.x -= FreezeBarWidth / 2.0f;

            float Progress = 0.0f;
            const int Max = pChar->Core()->m_FreezeEnd - pChar->Core()->m_FreezeStart;
            const int Current = Player.m_Tick - pChar->Core()->m_FreezeStart;
            if(Max > 0)
                Progress = 1.0f - std::clamp((float)Current / Max, 0.0f, 1.0f);

            CMapItemGroup *pGroup = GameClient()->Layers()->GameGroup();
            if(pGroup)
            {
                GameClient()->m_RenderTools.MapScreenToGroup(
                    GameClient()->m_Camera.m_Center.x,
                    GameClient()->m_Camera.m_Center.y,
                    pGroup,
                    GameClient()->m_Camera.m_Zoom);

            if(Progress > 0.0f)
            {
                    GameClient()->m_FreezeBars.RenderFreezeBarPos(
                        BarPos.x, BarPos.y,
                        FreezeBarWidth, FreezeBarHeight,
                        Progress, 1.0f);
                }
            }
        }
    }

    if((HasNinja || (IsFrozen && !GameClient()->m_GameInfo.m_NoSkinChangeForFrozen)) && UseNinjaSkin && g_Config.m_ClShowNinja)
    {
        const auto *pSkin = GameClient()->m_Skins.FindOrNullptr("x_ninja");
        if(pSkin != nullptr)
        {
            bool IsTeamplay = false;
            if(GameClient()->m_Snap.m_pGameInfoObj)
                IsTeamplay = (GameClient()->m_Snap.m_pGameInfoObj->m_GameFlags & GAMEFLAG_TEAMS) != 0;

            RenderInfo.Apply(pSkin);
            RenderInfo.m_CustomColoredSkin = IsTeamplay;
            if(!IsTeamplay)
            {
                RenderInfo.m_ColorBody = ColorRGBA(1, 1, 1, 1.0f);
                RenderInfo.m_ColorFeet = ColorRGBA(1, 1, 1, 1.0f);
            }
        }
    }

    CAnimState State;
    State.Set(&g_pData->m_aAnimations[ANIM_BASE], 0);

    CMapItemGroup *pGroup = GameClient()->Layers()->GameGroup();
    if(!pGroup)
        return;

    GameClient()->m_RenderTools.MapScreenToGroup(
        GameClient()->m_Camera.m_Center.x,
        GameClient()->m_Camera.m_Center.y,
        pGroup,
        GameClient()->m_Camera.m_Zoom);

    Graphics()->SetColor(1.0f, 1.0f, 1.0f, 1.0f);

    GameClient()->m_Players.RenderHook(&Player, &Player, &RenderInfo, -1, 0);
    GameClient()->m_Players.RenderHookCollLine(&Player, &Player, -1, 0);
    GameClient()->m_Players.RenderPlayer(&Player, &Player, &RenderInfo, -1, 0);

    Graphics()->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
    */
}