// (c) MRX-Client - Core MRX visuals (center lines, Same-X hint)
#include "mrxclient.h"

#include <engine/shared/config.h>
#include <engine/graphics.h>
#include <engine/textrender.h>

#include <game/client/gameclient.h>
#include <game/client/components/cheat/features/dummycontrol.h>
#include <game/client/components/binds.h>
#include <engine/input.h>

#include <base/color.h>
#include <base/vmath.h>

#include <algorithm>
#include <cmath>

void CMrxClient::OnRender()
{
    if(Client()->State() != IClient::STATE_ONLINE)
        return;

    if(g_Config.m_MRXThrowDummy)
        DummyThrow();

    if(g_Config.m_MRXFreezeSwitchFire)
        DummyFreezeFire();

    // Save and set screen mapping to pixel coordinates
    float X0, Y0, X1, Y1;
    Graphics()->GetScreen(&X0, &Y0, &X1, &Y1);
    Graphics()->MapScreen(0.0f, 0.0f, Graphics()->ScreenWidth(), Graphics()->ScreenHeight());

    // Render features
    RenderCenterLines();
    RenderSameXHint();

    // Restore mapping
    Graphics()->MapScreen(X0, Y0, X1, Y1);
}

void CMrxClient::RenderCenterLines()
{
    // Configs are defined in config_variables.h (MRXCenter*) per project rules
    if(g_Config.m_MRXCenterEnable <= 0)
        return;

    // Do not draw over scoreboard
    if(GameClient()->m_Scoreboard.IsActive())
        return;

    Graphics()->TextureClear();

    float SX0, SY0, SX1, SY1;
    Graphics()->GetScreen(&SX0, &SY0, &SX1, &SY1);
    const float XMid = (SX0 + SX1) * 0.5f;
    const float YMid = (SY0 + SY1) * 0.5f;

    const int Wconf = g_Config.m_MRXCenterWidth;
    if(Wconf == 0)
    {
        Graphics()->LinesBegin();
        IGraphics::CLineItem aLines[2] = {
            IGraphics::CLineItem(XMid, SY0, XMid, SY1),
            IGraphics::CLineItem(SX0, YMid, SX1, YMid)};
        Graphics()->SetColor(color_cast<ColorRGBA>(ColorHSLA(g_Config.m_MRXCenterColor, false)));
        Graphics()->LinesDraw(aLines, (int)std::size(aLines));
        Graphics()->LinesEnd();
    }
    else
    {
        const float W = (float)Wconf;
        const float H = (SY1 - SY0);
        const float Quarter1Y = SY0 + 0.25f * H;
        const float Quarter3Y = SY0 + 0.75f * H;

        Graphics()->QuadsBegin();
        IGraphics::CQuadItem aQuads[3] = {
            IGraphics::CQuadItem(XMid, Quarter1Y - W * 0.25f, W, (SY1 - SY0 - W) * 0.5f),
            IGraphics::CQuadItem(XMid, Quarter3Y + W * 0.25f, W, (SY1 - SY0 - W) * 0.5f),
            IGraphics::CQuadItem(XMid, YMid, (SX1 - SX0), W)};
        Graphics()->SetColor(color_cast<ColorRGBA>(ColorHSLA(g_Config.m_MRXCenterColor, false)));
        Graphics()->QuadsDraw(aQuads, (int)std::size(aQuads));
        Graphics()->QuadsEnd();
    }
}

void CMrxClient::RenderSameXHint()
{
    if(!g_Config.m_MRXOverlaySameX)
        return;

    const int LocalId = GameClient()->m_Snap.m_LocalClientId;
    if(LocalId < 0 || LocalId >= MAX_CLIENTS || !GameClient()->m_Snap.m_aCharacters[LocalId].m_Active)
        return;

    const float epsX = 0.5f;
    const vec2 myPos = GameClient()->m_Snap.m_aCharacters[LocalId].m_Position;
    float bestAbsDy = 1e9f;
    int bestDir = 0; // -1 above, +1 below

    for(int i = 0; i < MAX_CLIENTS; ++i)
    {
        if(i == LocalId)
            continue;
        const auto &Ch = GameClient()->m_Snap.m_aCharacters[i];
        if(!Ch.m_Active)
            continue;
        const vec2 p = Ch.m_Position;
        if(std::fabs(p.x - myPos.x) <= epsX)
        {
            const float dy = p.y - myPos.y;
            const float ady = std::fabs(dy);
            if(ady < bestAbsDy)
            {
                bestAbsDy = ady;
                bestDir = (dy > 0.0f ? +1 : -1);
            }
        }
    }

    if(bestDir == 0)
        return;

    const char *pMsg = bestDir < 0 ? "X совпадает с игроком сверху" : "X совпадает с игроком снизу";
    const float FontSize = (float)g_Config.m_MRXOverlayTextSize;
    const float ScreenW = Graphics()->ScreenWidth();
    const float ScreenH = Graphics()->ScreenHeight();

    const int px = std::clamp((int)g_Config.m_MRXOverlaySameXX, 0, 10000);
    const int py = std::clamp((int)g_Config.m_MRXOverlaySameXY, 0, 10000);
    const float x = (px / 10000.0f) * ScreenW;
    const float y = (py / 10000.0f) * ScreenH;
    ColorRGBA col = color_cast<ColorRGBA>(ColorHSLA(g_Config.m_MRXOverlayTextColor, false));
    Ui()->TextRender()->TextColor(col.r, col.g, col.b, col.a);
    Ui()->TextRender()->Text(x, y, FontSize, pMsg, -1.0f);
    Ui()->TextRender()->TextColor(1.0f, 1.0f, 1.0f, 1.0f);
}

void CMrxClient::DummyThrow()
{
    // Должен быть онлайн и иметь снапшот
    if(Client()->State() != IClient::STATE_ONLINE)
        return;

    // Локальный игрок и дамми
    const int LocalId = GameClient()->m_Snap.m_LocalClientId;
    const int DummyId = GameClient()->m_aLocalIds[!g_Config.m_ClDummy];
    if(LocalId < 0 || LocalId >= MAX_CLIENTS || DummyId < 0 || DummyId >= MAX_CLIENTS)
        return;

    // Активность
    if(!GameClient()->m_Snap.m_aCharacters[LocalId].m_Active || !GameClient()->m_aClients[DummyId].m_Active)
        return;

    // Оружие локального — молот
    const int ActiveWpn = GameClient()->m_Snap.m_aCharacters[LocalId].m_Cur.m_Weapon;
    if(ActiveWpn != WEAPON_HAMMER)
        return;

    // Фронт нажатия удара (0->1)
    const auto &Cur = GameClient()->m_Controls.m_aInputData[g_Config.m_ClDummy];
    const auto &Prev = GameClient()->m_Controls.m_aLastData[g_Config.m_ClDummy];
    const bool FireNow = (Cur.m_Fire & 1) != 0;
    const bool FirePrev = (Prev.m_Fire & 1) != 0;
    if(!(FireNow && !FirePrev))
        return;

    // Дистанция до дамми в радиусе молота
    const vec2 LocalPos = GameClient()->m_aClients[LocalId].m_RenderPos;
    const vec2 DummyPos = GameClient()->m_aClients[DummyId].m_RenderPos;
    const float Dist = distance(LocalPos, DummyPos);
    if(!(Dist > 0.0f && Dist <= 63.0f))
        return;

    // Скорость X дамми и выдача движения
    const float TargetVelX = GameClient()->m_aClients[DummyId].m_Predicted.m_Vel.x;
    const float eps = 0.05f;
    if(TargetVelX > eps)
        GameClient()->m_DummyControl.MoveRight();
    else if(TargetVelX < -eps)
        GameClient()->m_DummyControl.MoveLeft();
    else
        GameClient()->m_DummyControl.Stop();
}

void CMrxClient::DummyFreezeFire()
{
    if (Client()->State() != IClient::STATE_ONLINE)
        return;

    const int DummyId = GameClient()->m_aLocalIds[!g_Config.m_ClDummy];
    if (DummyId < 0 || DummyId >= MAX_CLIENTS || !GameClient()->m_aClients[DummyId].m_Active)
        return;

    const bool isDummyFrozen = GameClient()->m_aClients[DummyId].m_Predicted.m_FreezeEnd > 0;

    char aKeyName[64] = {0};
    GameClient()->m_Binds.GetKey("+toggle cl_dummy_hook 1 0", aKeyName, sizeof(aKeyName));
    int iKey = Input()->FindKeyByName(aKeyName);

    if (iKey == 0)
        return;

    if (isDummyFrozen && Input()->KeyIsPressed(iKey))
    {
        GameClient()->m_Controls.m_aInputData[g_Config.m_ClDummy].m_Fire = (GameClient()->m_Controls.m_aInputData[g_Config.m_ClDummy].m_Fire + 1) | 1;
    }
    else
    {
        GameClient()->m_Controls.m_aInputData[g_Config.m_ClDummy].m_Fire &= ~1;
    }
}

void CMrxClient::HandleSmartDummyHook()
{
    // Умная логика хука дамми: хукать только когда летит вверх (VelY < 0)
    // Эта функция вызывается из controls.cpp когда активны cl_dummy_hook И mrx_smart_dummy_hook

    // Получаем ID дамми
    const int DummyId = GameClient()->m_aLocalIds[!g_Config.m_ClDummy];
    if(DummyId < 0 || DummyId >= MAX_CLIENTS)
        return;

    // Проверяем что дамми активен
    if(!GameClient()->m_aClients[DummyId].m_Active)
        return;

    // Получаем скорость дамми по оси Y
    const float DummyVelY = GameClient()->m_aClients[DummyId].m_Predicted.m_Vel.y;

    // Хукаем только если дамми летит ВВЕРХ (VelY < 0)
    // Если дамми падает вниз или стоит (VelY >= 0), отключаем хук
    CNetObj_PlayerInput *pDummyInput = &GameClient()->m_DummyInput;
    if(DummyVelY < 0.0f)
    {
        // Дамми летит вверх - разрешаем хук
        pDummyInput->m_Hook = 1;
    }
    else
    {
        // Дамми падает вниз или стоит - отключаем хук
        pDummyInput->m_Hook = 0;
    }
}