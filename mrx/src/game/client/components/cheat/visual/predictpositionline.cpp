#include "predictpositionline.h"

#include <algorithm> // std::clamp
#include <vector>

#include <base/color.h>
#include <base/vmath.h>
#include <engine/graphics.h>
#include <engine/shared/config.h>
#include <game/client/gameclient.h>
#include <game/client/render.h>
#include <game/client/prediction/gameworld.h>
#include <game/client/prediction/entities/character.h>

static inline bool IsFreezeTile(int Tile)
{
    return Tile == TILE_FREEZE || Tile == TILE_DFREEZE || Tile == TILE_LFREEZE;
}

static bool SegmentTouchesFreeze(class CGameClient *pClient, const vec2 &A, const vec2 &B)
{
    // Проверяем несколько точек на отрезке A->B
    const int Samples = 8;
    for(int i = 0; i <= Samples; ++i)
    {
        float t = (float)i / (float)Samples;
        vec2 P = mix(A, B, t);
        int MapIndex = pClient->Collision()->GetMapIndex(P);
        if(MapIndex < 0)
            continue;
        int Tile = pClient->Collision()->GetTileIndex(MapIndex);
        int Front = pClient->Collision()->GetFrontTileIndex(MapIndex);
        if(IsFreezeTile(Tile) || IsFreezeTile(Front))
            return true;
    }
    return false;
}

void CPredictPositionLine::OnRender()
{
    // Переключатель рендера
    if(!g_Config.m_MRXPredictTraj)
        return;

    // Получаем локального персонажа из предсказанного мира
    auto *pCurWorld = &GameClient()->m_PredictedWorld;
    const int LocalID = GameClient()->m_aLocalIds[g_Config.m_ClDummy];
    if(LocalID < 0)
        return;
    CCharacter *pCurChar = pCurWorld->GetCharacterById(LocalID);
    if(!pCurChar)
        return;

    // Копируем мир для безопасной симуляции
    // ВАЖНО: нельзя использовать копирующий конструктор (шаллоу-копия приведёт к двойному delete при Clear()).
    // Используем пустой мир + CopyWorld, как в CTas::StartRecording().
    CGameWorld TempWorld;
    TempWorld.CopyWorld(pCurWorld);
    CCharacter *pTempChar = TempWorld.GetCharacterById(LocalID);
    if(!pTempChar)
        return;

    // Текущий ввод
    CNetObj_PlayerInput TempInput = GameClient()->m_Controls.m_aInputData[g_Config.m_ClDummy];

    // Количество тиков предсказания из конфига (кламп по правилам проекта)
    const int MaxTicks = std::clamp(g_Config.m_MRXPredictTrajTicks, 0, 200);
    if(MaxTicks <= 0)
        return;

    std::vector<vec2> Positions;
    Positions.reserve(MaxTicks + 1);
    Positions.push_back(pTempChar->m_Pos);

    for(int t = 0; t < MaxTicks; ++t)
    {
        ++TempWorld.m_GameTick;
        // Как и в CTas::RecordTick: сначала Direct, затем Predicted
        pTempChar->OnDirectInput(&TempInput);
        pTempChar->OnPredictedInput(&TempInput);
        TempWorld.Tick();

        // После тика обновляем ссылку на персонажа (мог смениться указатель)
        pTempChar = TempWorld.GetCharacterById(LocalID);
        if(!pTempChar)
            break;
        Positions.push_back(pTempChar->m_Pos);
    }

    if(Positions.size() < 2)
        return;

    // Настройка мировых координат камеры, чтобы исключить смещение/масштаб
    Graphics()->TextureClear();
    float w = 0.0f, h = 0.0f;
    Graphics()->CalcScreenParams(Graphics()->ScreenAspect(), GameClient()->m_Camera.m_Zoom, &w, &h);
    const float cx = GameClient()->m_Camera.m_Center.x;
    const float cy = GameClient()->m_Camera.m_Center.y;
    Graphics()->MapScreen(cx - w / 2.0f, cy - h / 2.0f, cx + w / 2.0f, cy + h / 2.0f);

    // Определяем цвета (через ColorHSLA -> ColorRGBA)
    ColorRGBA baseColor = color_cast<ColorRGBA>(ColorHSLA(g_Config.m_MRXPredictTrajColor).UnclampLighting(ColorHSLA::DARKEST_LGT));
    ColorRGBA freezeColor = color_cast<ColorRGBA>(ColorHSLA(g_Config.m_MRXPredictTrajColorFreeze).UnclampLighting(ColorHSLA::DARKEST_LGT));

    // Толщина линии (в мировых единицах). Чуть толще обычной
    const float kLineWidth = 2.0f;

    // Режим отрисовки: 0 = solid, 1 = dashed
    const int mode = g_Config.m_MRXPredictTrajMode;
    const float dashLen = 16.0f; // мировые единицы
    const float gapLen = 8.0f;

    Graphics()->QuadsBegin();

    // Глобальная фаза пунктирного паттерна по всей полилинии
    float phase = 0.0f;           // пройдено внутри текущего dash/gap
    bool inDash = true;           // начинаем с прорисовываемого сегмента
    const float cycle = dashLen + gapLen;

    for(size_t i = 0; i + 1 < Positions.size(); ++i)
    {
        vec2 a = Positions[i];
        vec2 b = Positions[i + 1];
        if(a.x == b.x && a.y == b.y)
            continue;

        const bool segTouchesFreeze = SegmentTouchesFreeze(GameClient(), a, b);
        const ColorRGBA segColor = segTouchesFreeze ? freezeColor : baseColor;

        vec2 dir = normalize(b - a);
        vec2 perp(-dir.y, dir.x);
        vec2 offset = perp * (kLineWidth * 0.5f);

        const float totalLen = distance(a, b);
        float consumed = 0.0f;

        if(mode == 1)
        {
            // пунктир через единую фазу на всю траекторию
            while(consumed < totalLen)
            {
                const float remain = totalLen - consumed;
                const float targetLen = (inDash ? dashLen : gapLen);
                const float step = std::min(targetLen - phase, remain);

                vec2 p0 = a + dir * consumed;
                vec2 p1 = a + dir * (consumed + step);

                if(inDash)
                {
                    Graphics()->SetColor(segColor);
                    vec2 v0 = p0 + offset;
                    vec2 v1 = p0 - offset;
                    vec2 v2 = p1 + offset;
                    vec2 v3 = p1 - offset;
                    IGraphics::CFreeformItem item(v0.x, v0.y, v1.x, v1.y, v2.x, v2.y, v3.x, v3.y);
                    Graphics()->QuadsDrawFreeform(&item, 1);
                }

                consumed += step;
                phase += step;
                if(phase >= targetLen - 1e-4f)
                {
                    phase = 0.0f;
                    inDash = !inDash;
                }
            }
        }
        else
        {
            // сплошная: весь сегмент единой полосой
            Graphics()->SetColor(segColor);
            vec2 v0 = a + offset;
            vec2 v1 = a - offset;
            vec2 v2 = b + offset;
            vec2 v3 = b - offset;
            IGraphics::CFreeformItem item(v0.x, v0.y, v1.x, v1.y, v2.x, v2.y, v3.x, v3.y);
            Graphics()->QuadsDrawFreeform(&item, 1);
        }
    }

    Graphics()->QuadsEnd();
}