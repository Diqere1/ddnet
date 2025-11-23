// (c) MRX-Client - Command Overlay component implementation
#include "comandoverlay.h"

#include <engine/shared/config.h>
#include <engine/graphics.h>
#include <engine/textrender.h>

#include <game/client/gameclient.h>
#include <game/client/ui.h>

#include <base/color.h>

#include <algorithm>

static inline ColorRGBA GetSlotTextColor(int Slot)
{
    switch(Slot)
    {
    case 0: return color_cast<ColorRGBA>(ColorHSLA(g_Config.m_MRXTextColor, false));
    case 1: return color_cast<ColorRGBA>(ColorHSLA(g_Config.m_MRXText1Color, false));
    case 2: return color_cast<ColorRGBA>(ColorHSLA(g_Config.m_MRXText2Color, false));
    case 3: return color_cast<ColorRGBA>(ColorHSLA(g_Config.m_MRXText3Color, false));
    case 4: return color_cast<ColorRGBA>(ColorHSLA(g_Config.m_MRXText4Color, false));
    default: return color_cast<ColorRGBA>(ColorHSLA(g_Config.m_MRXOverlayTextColor, false));
    }
}

void CCommandOverlay::RenderTextSlot(const char *pText, int XPercent, int YPercent, int Slot) const
{
    if(!pText || pText[0] == '\0')
        return;

    // Clamp percents to [0,10000]
    const int ClampedX = std::clamp(XPercent, 0, 10000);
    const int ClampedY = std::clamp(YPercent, 0, 10000);

    const float ScreenW = this->Graphics()->ScreenWidth();
    const float ScreenH = this->Graphics()->ScreenHeight();

    const float X = (ClampedX / 10000.0f) * ScreenW;
    const float Y = (ClampedY / 10000.0f) * ScreenH;

    const float FontSize = (float)g_Config.m_MRXOverlayTextSize;
    const float Padding = 4.0f;
    const int Corners = IGraphics::CORNER_ALL;
    const float Rounding = 3.0f;

    // Measure text
    const float TextW = this->Ui()->TextRender()->TextWidth(FontSize, pText, -1);
    const float TextH = FontSize; // approximate line height

    const ColorRGBA SlotTxtCol = GetSlotTextColor(Slot);
    // Text color
    this->Ui()->TextRender()->TextColor(SlotTxtCol.r, SlotTxtCol.g, SlotTxtCol.b, SlotTxtCol.a);
    this->Ui()->TextRender()->Text(X, Y, FontSize, pText, -1.0f);
    this->Ui()->TextRender()->TextColor(1.0f, 1.0f, 1.0f, 1.0f);
}

void CCommandOverlay::OnRender()
{
    if(!g_Config.m_MRXOverlayText)
        return;

    if(Client()->State() != IClient::STATE_ONLINE)
        return;

    // Отрисовываем даже когда меню активно — компонент теперь рендерится ДО меню, поэтому будет под ним

    // Save screen mapping
    float Points[4];
    this->Graphics()->GetScreen(Points, Points + 1, Points + 2, Points + 3);
    this->Graphics()->MapScreen(0.0f, 0.0f, this->Graphics()->ScreenWidth(), this->Graphics()->ScreenHeight());

    // Slot 0..4
    RenderTextSlot(g_Config.m_MRXText,  g_Config.m_MRXTextX,  g_Config.m_MRXTextY,  0);
    RenderTextSlot(g_Config.m_MRXText1, g_Config.m_MRXText1X, g_Config.m_MRXText1Y, 1);
    RenderTextSlot(g_Config.m_MRXText2, g_Config.m_MRXText2X, g_Config.m_MRXText2Y, 2);
    RenderTextSlot(g_Config.m_MRXText3, g_Config.m_MRXText3X, g_Config.m_MRXText3Y, 3);
    RenderTextSlot(g_Config.m_MRXText4, g_Config.m_MRXText4X, g_Config.m_MRXText4Y, 4);

    // Доп. надпись: если локальный игрок по X ровно под/над другим игроком
    // Управляется флагом mrx_overlay_samex и позиционируется по процентам X/Y
    const float epsX = 0.5f; // допуск по X в мировых единицах
    const int LocalId = GameClient()->m_Snap.m_LocalClientId;
    if(g_Config.m_MRXOverlaySameX && LocalId >= 0 && LocalId < MAX_CLIENTS && GameClient()->m_Snap.m_aCharacters[LocalId].m_Active)
    {
        const vec2 myPos = GameClient()->m_Snap.m_aCharacters[LocalId].m_Position;
        float bestAbsDy = 1e9f;
        int bestDir = 0; // -1 сверху, +1 снизу

        for(int i = 0; i < MAX_CLIENTS; ++i)
        {
            if(i == LocalId)
                continue;
            const auto &Ch = GameClient()->m_Snap.m_aCharacters[i];
            if(!Ch.m_Active)
                continue;

            const vec2 p = Ch.m_Position;
            if(fabsf(p.x - myPos.x) <= epsX)
            {
                const float dy = p.y - myPos.y; // >0 ниже, <0 выше (экранная ось Y вниз)
                const float ady = fabsf(dy);
                if(ady < bestAbsDy)
                {
                    bestAbsDy = ady;
                    bestDir = (dy > 0.0f ? +1 : -1);
                }
            }
        }

        if(bestDir != 0)
        {
            const char *pMsg = bestDir < 0 ? "X совпадает с игроком сверху" : "X совпадает с игроком снизу";
            const float FontSize = (float)g_Config.m_MRXOverlayTextSize;
            const float ScreenW = this->Graphics()->ScreenWidth();
            const float ScreenH = this->Graphics()->ScreenHeight();
            // Позиция по процентам (по умолчанию X=50%, Y=8%)
            const int px = std::clamp((int)g_Config.m_MRXOverlaySameXX, 0, 10000);
            const int py = std::clamp((int)g_Config.m_MRXOverlaySameXY, 0, 10000);
            const float x = (px / 10000.0f) * ScreenW;
            const float y = (py / 10000.0f) * ScreenH;

            // Цвет как общий для overlay
            ColorRGBA col = color_cast<ColorRGBA>(ColorHSLA(g_Config.m_MRXOverlayTextColor, false));
            this->Ui()->TextRender()->TextColor(col.r, col.g, col.b, col.a);
            this->Ui()->TextRender()->Text(x, y, FontSize, pMsg, -1.0f);
            // Сбрасываем цвет в белый, чтобы не протёк на остальной интерфейс
            this->Ui()->TextRender()->TextColor(1.0f, 1.0f, 1.0f, 1.0f);
        }
    }

    // Restore screen mapping
    this->Graphics()->MapScreen(Points[0], Points[1], Points[2], Points[3]);
    // Safety: ensure text color is back to white for any subsequent rendering
    this->Ui()->TextRender()->TextColor(1.0f, 1.0f, 1.0f, 1.0f);
}

