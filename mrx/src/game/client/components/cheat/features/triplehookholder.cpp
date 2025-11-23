#include "triplehookholder.h"
#include <engine/shared/config.h>
#include <game/client/gameclient.h>
#include <limits>

void CTripleHookHolder::Reset()
{
    m_Target.reset();
    m_Active = false;
}

void CTripleHookHolder::OnRender()
{
    // Управление состоянием для правильного сброса fire inputs
    static bool s_WasActiveThrow = false;
    static bool s_WasActiveThrowBoth = false;
    bool ActiveThrow = g_Config.m_MRXAutotripleflyThrow;
    bool ActiveThrowBoth = g_Config.m_MRXAutotripleflyThrowBoth;

    if((!ActiveThrow && s_WasActiveThrow) || (!ActiveThrowBoth && s_WasActiveThrowBoth))
    {
        // Только при переходе из активного в неактивное состояние
        GameClient()->ReleaseFireInputs();
        GameClient()->DummyResetInput();
        m_NeedReleaseFire = true;
        
        // Сбрасываем цели при отключении команд
        if(!ActiveThrow && s_WasActiveThrow)
        {
            m_AutoTripleFlyThrow = {};
            m_GlobalCooldownThrow = 0; // Сбрасываем глобальный кулдаун
        }
        if(!ActiveThrowBoth && s_WasActiveThrowBoth)
        {
            m_AutoTripleFlyThrowBoth = {};
            m_GlobalCooldownThrowBoth = 0; // Сбрасываем глобальный кулдаун
        }
    }
    else if(m_NeedReleaseFire)
    {
        // На следующий тик сбросить флаг, чтобы не сбрасывать fire постоянно
        m_NeedReleaseFire = false;
    }

    s_WasActiveThrow = ActiveThrow;
    s_WasActiveThrowBoth = ActiveThrowBoth;

    // Визуализация для TripleHookHolder
    if(!g_Config.m_MRXTripleHookHolder || !m_Target)
        return;

    int DummyId = GameClient()->m_aLocalIds[!g_Config.m_ClDummy];
    if(DummyId < 0)
        return;

    const vec2 DummyPos = GameClient()->m_aClients[DummyId].m_Predicted.m_Pos;
    float FixedY = m_Target->FixedY;

    // Визуализация линии на фиксированной Y-координате
    float LineX0 = DummyPos.x - 400.0f;
    float LineX1 = DummyPos.x + 400.0f;
    float LineY = FixedY;

    Graphics()->TextureClear();
    Graphics()->LinesBegin();
    Graphics()->SetColor(1.0f, 0.2f, 0.2f, 0.7f); // Красная полупрозрачная линия
    IGraphics::CLineItem Line(LineX0, LineY, LineX1, LineY);
    Graphics()->LinesDraw(&Line, 1);
    Graphics()->LinesEnd();
}

std::optional<CTripleHookHolder::TargetInfo> CTripleHookHolder::FindTargetBelowDummy() const
{
    int DummyId = GameClient()->m_aLocalIds[!g_Config.m_ClDummy];
    if(DummyId < 0)
        return std::nullopt;

    const vec2 DummyPos = GameClient()->m_aClients[DummyId].m_Predicted.m_Pos;
    float MinDist = std::numeric_limits<float>::max();
    int BestId = -1;
    vec2 BestPos;

    for(int i = 0; i < MAX_CLIENTS; ++i)
    {
        if(i == DummyId || i == GameClient()->m_aLocalIds[g_Config.m_ClDummy])
            continue;
        const auto& Client = GameClient()->m_aClients[i];
        if(!Client.m_Active || Client.m_Team == TEAM_SPECTATORS)
            continue;
        const vec2 Pos = Client.m_Predicted.m_Pos;
        if(Pos.y <= DummyPos.y)
            continue;
        float Dist = distance(DummyPos, Pos);
        if(Dist < MinDist)
        {
            MinDist = Dist;
            BestId = i;
            BestPos = Pos;
        }
    }
    if(BestId != -1)
        return TargetInfo{BestId, BestPos.y, BestPos};
    return std::nullopt;
}

void CTripleHookHolder::HandleTripleHookHolder()
{
    if(!g_Config.m_MRXTripleHookHolder)
    {
        Reset();
        return;
    }

    int DummyId = GameClient()->m_aLocalIds[!g_Config.m_ClDummy];
    if(DummyId < 0)
    {
        Reset();
        return;
    }

    auto& DummyInput = GameClient()->m_DummyInput;
    const vec2 DummyPos = GameClient()->m_aClients[DummyId].m_Predicted.m_Pos;

    static bool s_LastActive = false;
    if(g_Config.m_MRXTripleHookHolder && !s_LastActive)
    {
        m_Target = FindTargetBelowDummy();
        m_Active = true;
    }
    s_LastActive = g_Config.m_MRXTripleHookHolder;

    if(!m_Target || !GameClient()->m_aClients[m_Target->ClientId].m_Active)
    {
        m_Target = FindTargetBelowDummy();
        if(!m_Target)
        {
            DummyInput.m_Hook = 0;
            return;
        }
    }

    // Получаем актуальные координаты и скорость цели
    const auto& TargetClient = GameClient()->m_aClients[m_Target->ClientId];
    const vec2 TargetPos = TargetClient.m_Predicted.m_Pos;
    float TargetY = TargetPos.y;
    float FixedY = m_Target->FixedY;
    float TargetVelY = TargetClient.m_Predicted.m_Vel.y;

    // Параметры физики
    float HookLength = GameClient()->m_aTuning->m_HookLength;
    float HookDragAccel = GameClient()->m_aTuning->m_HookDragAccel;
    float HookDragSpeed = GameClient()->m_aTuning->m_HookDragSpeed;

    // Предсказание положения цели (1 тик вперёд)
    constexpr float TickTime = 1.0f / 50.0f;
    float PredictedY = TargetY + TargetVelY * TickTime;

    // Погрешность удержания
    constexpr float HoldEpsilon = 10.0f;

    // Если цель выше фиксированной Y — отпустить хук, но если скорость вниз большая, сделать микро-хук
    if(PredictedY < FixedY - HoldEpsilon)
    {
        if(TargetVelY > 2.5f) // быстро падает — микро-хук для замедления
        {
            static int s_MicroHookTick = 0;
            if(s_MicroHookTick == 0)
            {
                DummyInput.m_Hook = 1;
                vec2 Dir = normalize(TargetPos - DummyPos);
                DummyInput.m_TargetX = (int)(Dir.x * 100.0f);
                DummyInput.m_TargetY = (int)(Dir.y * 100.0f);
            }
            else
            {
                DummyInput.m_Hook = 0;
            }
            s_MicroHookTick = (s_MicroHookTick + 1) % 2;
        }
        else
        {
            DummyInput.m_Hook = 0;
        }
        return;
    }

    // Если цель ниже фиксированной Y — тянуть хуком вверх, если скорость вниз маленькая — держать чуть дольше
    if(PredictedY > FixedY + HoldEpsilon)
    {
        DummyInput.m_Hook = 1;
        vec2 Dir = normalize(TargetPos - DummyPos);
        DummyInput.m_TargetX = (int)(Dir.x * 100.0f);
        DummyInput.m_TargetY = (int)(Dir.y * 100.0f);
        return;
    }

    // Если цель примерно на нужной высоте — удерживать, но если скорость вверх большая — отпустить хук
    if(std::abs(PredictedY - FixedY) <= HoldEpsilon)
    {
        if(TargetVelY < -2.5f) // быстро летит вверх — не хукать, чтобы не перетянуть
        {
            DummyInput.m_Hook = 0;
            return;
        }
        DummyInput.m_Hook = 1;
        vec2 Dir = normalize(TargetPos - DummyPos);
        DummyInput.m_TargetX = (int)(Dir.x * 100.0f);
        DummyInput.m_TargetY = (int)(Dir.y * 100.0f);
        return;
    }
}

void CTripleHookHolder::HandleAutoTripleFlyDefault()
{
    if(!g_Config.m_MRXAutotriplefly || g_Config.m_MRXAutotripleflyMode != 1)
    {
        m_AutoTripleFly = {};
        return;
    }

    int DummyId = GameClient()->m_aLocalIds[!g_Config.m_ClDummy];
    if(DummyId < 0)
    {
        m_AutoTripleFly = {};
        return;
    }

    auto& DummyInput = GameClient()->m_DummyInput;
    const vec2 DummyPos = GameClient()->m_aClients[DummyId].m_Predicted.m_Pos;

    // Найти ближайшего к курсору и дамми игрока
    int BestId = -1;
    float BestDist = std::numeric_limits<float>::max();
    vec2 BestPos;

    // Курсор дамми
    vec2 MouseDir = normalize(vec2(GameClient()->m_Controls.m_aInputData[!g_Config.m_ClDummy].m_TargetX, GameClient()->m_Controls.m_aInputData[!g_Config.m_ClDummy].m_TargetY));
    vec2 MousePos = DummyPos + MouseDir * 300.0f; // 300 — произвольная длина "луча" курсора

    for(int i = 0; i < MAX_CLIENTS; ++i)
    {
        if(i == DummyId || i == GameClient()->m_aLocalIds[g_Config.m_ClDummy])
            continue;
        const auto& Client = GameClient()->m_aClients[i];
        if(!Client.m_Active || Client.m_Team == TEAM_SPECTATORS)
            continue;
        const vec2 Pos = Client.m_Predicted.m_Pos;
        float DistToDummy = distance(DummyPos, Pos);
        float DistToCursor = distance(MousePos, Pos);
        float Score = DistToDummy + DistToCursor * 0.7f; // Можно варьировать вес
        if(Score < BestDist)
        {
            BestDist = Score;
            BestId = i;
            BestPos = Pos;
        }
    }

    if(BestId == -1)
    {
        m_AutoTripleFly = {};
        DummyInput.m_Hook = 0;
        return;
    }

    m_AutoTripleFly.ClientId = BestId;
    m_AutoTripleFly.Active = true;
    m_AutoTripleFly.DesiredDistance = g_Config.m_MRXAutotripleflyManual ? g_Config.m_MRXAutotripleflyDistance : distance(DummyPos, BestPos);

    // Управление хуком (аналогично HandleAutoTripleFly)
    const vec2 TargetPos = GameClient()->m_aClients[m_AutoTripleFly.ClientId].m_Predicted.m_Pos;
    float CurrentDist = distance(DummyPos, TargetPos);
    float DesiredDist = m_AutoTripleFly.DesiredDistance;
    float TargetVelY = GameClient()->m_aClients[m_AutoTripleFly.ClientId].m_Predicted.m_Vel.y;



    // Если цель слишком близко ИЛИ выше дамми - отпускаем хук
    if(CurrentDist < DesiredDist || TargetPos.y < DummyPos.y)
    {
        DummyInput.m_Hook = 0;
        
    }
    
    // Направление устанавливаем только во время хука
    if(DummyInput.m_Hook == 1)
    {
        vec2 Dir = normalize(TargetPos - DummyPos);
        DummyInput.m_TargetX = (int)(Dir.x * 100.0f);
        DummyInput.m_TargetY = (int)(Dir.y * 100.0f);
    }
}

void CTripleHookHolder::HandleAutoTripleFly()
{
    if(!g_Config.m_MRXAutotriplefly || g_Config.m_MRXAutotripleflyMode != 0)
    {
        m_AutoTripleFly = {};
        return;
    }

    int DummyId = GameClient()->m_aLocalIds[!g_Config.m_ClDummy];
    if(DummyId < 0)
    {
        m_AutoTripleFly = {};
        return;
    }

    auto& DummyInput = GameClient()->m_DummyInput;
    const vec2 DummyPos = GameClient()->m_aClients[DummyId].m_Predicted.m_Pos;

    // Если нет цели, ищем её по хуку
    if(!m_AutoTripleFly.Active)
    {
        int HookedId = GameClient()->m_aClients[DummyId].m_Predicted.HookedPlayer();
        if(g_Config.m_ClDummyHook && HookedId >= 0 && HookedId != DummyId)
        {
            m_AutoTripleFly.ClientId = HookedId;
            const vec2 TargetPos = GameClient()->m_aClients[HookedId].m_Predicted.m_Pos;
            if(g_Config.m_MRXAutotripleflyManual)
                m_AutoTripleFly.DesiredDistance = g_Config.m_MRXAutotripleflyDistance;
            else
                m_AutoTripleFly.DesiredDistance = distance(DummyPos, TargetPos);
            m_AutoTripleFly.Active = true;
        }
        else
        {
            return;
        }
    }

    // Если цель невалидна или хук отпущен — сбрасываем
    if(!g_Config.m_ClDummyHook || m_AutoTripleFly.ClientId < 0 || !GameClient()->m_aClients[m_AutoTripleFly.ClientId].m_Active)
    {
        m_AutoTripleFly = {};
        DummyInput.m_Hook = 0;
        return;
    }

    // Получаем актуальные координаты цели
    const vec2 TargetPos = GameClient()->m_aClients[m_AutoTripleFly.ClientId].m_Predicted.m_Pos;
    float CurrentDist = distance(DummyPos, TargetPos);
    float DesiredDist = m_AutoTripleFly.DesiredDistance;
    float TargetVelY = GameClient()->m_aClients[m_AutoTripleFly.ClientId].m_Predicted.m_Vel.y;


    if(CurrentDist < DesiredDist || TargetPos.y < DummyPos.y)
    {
        DummyInput.m_Hook = 0;
    }
    
    if(DummyInput.m_Hook == 1)
    {
        vec2 Dir = normalize(TargetPos - DummyPos);
        DummyInput.m_TargetX = (int)(Dir.x * 100.0f);
        DummyInput.m_TargetY = (int)(Dir.y * 100.0f);
    }
}

int CTripleHookHolder::FindTargetNearCursor() const
{
    int DummyId = GameClient()->m_aLocalIds[!g_Config.m_ClDummy];
    if(DummyId < 0)
        return -1;

    const vec2 DummyPos = GameClient()->m_aClients[DummyId].m_Predicted.m_Pos;
    
    // Курсор дамми
    vec2 MouseDir = normalize(vec2(GameClient()->m_Controls.m_aInputData[!g_Config.m_ClDummy].m_TargetX, GameClient()->m_Controls.m_aInputData[!g_Config.m_ClDummy].m_TargetY));
    vec2 MousePos = DummyPos + MouseDir * 300.0f; // 300 — произвольная длина "луча" курсора

    int BestId = -1;
    float BestDist = std::numeric_limits<float>::max();

    for(int i = 0; i < MAX_CLIENTS; ++i)
    {
        if(i == DummyId || i == GameClient()->m_aLocalIds[g_Config.m_ClDummy])
            continue;
        const auto& Client = GameClient()->m_aClients[i];
        if(!Client.m_Active || Client.m_Team == TEAM_SPECTATORS)
            continue;
        const vec2 Pos = Client.m_Predicted.m_Pos;
        
        // Не хукаем игроков, которые выше дамми
        if(Pos.y <= DummyPos.y)
            continue;
            
        float DistToCursor = distance(MousePos, Pos);
        if(DistToCursor < BestDist)
        {
            BestDist = DistToCursor;
            BestId = i;
        }
    }
    return BestId;
}

int CTripleHookHolder::FindTargetNearMainCursor() const
{
    int DummyId = GameClient()->m_aLocalIds[!g_Config.m_ClDummy];
    if(DummyId < 0)
        return -1;

    const vec2 DummyPos = GameClient()->m_aClients[DummyId].m_Predicted.m_Pos;
    
    // Получаем позицию курсора основного игрока с учетом масштаба камеры (как в dummythrow.cpp)
    float CameraZoom = GameClient()->m_Camera.m_Zoom;
    vec2 MousePos = GameClient()->m_Controls.m_aMousePos[g_Config.m_ClDummy]; // Основной игрок
    vec2 WorldPos;

    if(GameClient()->m_Snap.m_SpecInfo.m_Active && GameClient()->m_Snap.m_SpecInfo.m_UsePosition)
    {
        WorldPos = GameClient()->m_Snap.m_SpecInfo.m_Position + MousePos * CameraZoom;
    }
    else if(GameClient()->m_Snap.m_pGameInfoObj)
    {
        WorldPos = GameClient()->m_LocalCharacterPos + MousePos * CameraZoom;
    }
    else
    {
        WorldPos = MousePos * CameraZoom;
    }

    int BestId = -1;
    float BestDist = std::numeric_limits<float>::max();

    for(int i = 0; i < MAX_CLIENTS; ++i)
    {
        if(i == DummyId || i == GameClient()->m_aLocalIds[g_Config.m_ClDummy])
            continue;
        const auto& Client = GameClient()->m_aClients[i];
        if(!Client.m_Active || Client.m_Team == TEAM_SPECTATORS)
            continue;
        const vec2 Pos = Client.m_Predicted.m_Pos;
        
        // Не хукаем игроков, которые выше дамми
        if(Pos.y <= DummyPos.y)
            continue;
            
        float DistToCursor = distance(WorldPos, Pos);
        if(DistToCursor < BestDist)
        {
            BestDist = DistToCursor;
            BestId = i;
        }
    }
    return BestId;
}

void CTripleHookHolder::HandleAutoTripleFlyThrow()
{
    if(!g_Config.m_MRXAutotripleflyThrow)
    {
        m_AutoTripleFlyThrow = {};
        m_GlobalCooldownThrow = 0; // Сбрасываем глобальный кулдаун
        return;
    }

    // Проверяем глобальный кулдаун
    if(m_GlobalCooldownThrow > 0)
    {
        m_GlobalCooldownThrow--;
        return;
    }

    int DummyId = GameClient()->m_aLocalIds[!g_Config.m_ClDummy];
    if(DummyId < 0)
    {
        m_AutoTripleFlyThrow = {};
        return;
    }

    auto& DummyInput = GameClient()->m_DummyInput;
    const vec2 DummyPos = GameClient()->m_aClients[DummyId].m_Predicted.m_Pos;

    // Ищем ближайшего к курсору основного игрока
    int TargetId = FindTargetNearMainCursor();
    
    // Если нет активной цели и нашли новую цель
    if(!m_AutoTripleFlyThrow.Active && TargetId != -1)
    {
        m_AutoTripleFlyThrow.ClientId = TargetId;
        m_AutoTripleFlyThrow.Active = true;
        m_AutoTripleFlyThrow.HasThrown = false;
    }
    // Если есть активная цель, но текущая цель стала невалидной
    else if(m_AutoTripleFlyThrow.Active && TargetId != -1 && TargetId != m_AutoTripleFlyThrow.ClientId)
    {
        // Проверяем, валидна ли текущая цель
        if(m_AutoTripleFlyThrow.ClientId < 0 || !GameClient()->m_aClients[m_AutoTripleFlyThrow.ClientId].m_Active)
        {
            // Текущая цель невалидна, переключаемся на новую
            m_AutoTripleFlyThrow.ClientId = TargetId;
            m_AutoTripleFlyThrow.HasThrown = false;
        }
        // Иначе оставляем текущую цель (не меняем постоянно)
    }
    
    // Если не нашли цель и нет активной цели
    if(TargetId == -1 && !m_AutoTripleFlyThrow.Active)
    {
        DummyInput.m_Hook = 0;
        return;
    }

    // Если цель невалидна — сбрасываем
    if(m_AutoTripleFlyThrow.ClientId < 0 || !GameClient()->m_aClients[m_AutoTripleFlyThrow.ClientId].m_Active)
    {
        m_AutoTripleFlyThrow = {};
        DummyInput.m_Hook = 0;
        return;
    }

    // Получаем актуальные координаты цели
    const vec2 TargetPos = GameClient()->m_aClients[m_AutoTripleFlyThrow.ClientId].m_Predicted.m_Pos;
    float CurrentDist = distance(DummyPos, TargetPos);

    // Если цель находится слишком далеко — сбрасываем
    if(CurrentDist > 400.0f)
    {
        m_AutoTripleFlyThrow = {};
        DummyInput.m_Hook = 0;
        return;
    }

    // Если цель на расстоянии 63 или меньше — делаем удар
    if(CurrentDist <= 63.0f)
    {
        DummyInput.m_Hook = 0; // Отпускаем хук от игрока
        vec2 Dir = normalize(TargetPos - DummyPos);
        DummyInput.m_TargetX = (int)(Dir.x * 100.0f);
        DummyInput.m_TargetY = (int)(Dir.y * 100.0f);
        DummyInput.m_Fire++; // Делаем удар
        m_AutoTripleFlyThrow.HasThrown = true;
        m_GlobalCooldownThrow = g_Config.m_MRXAutotripleflyCooldown; // Устанавливаем глобальный кулдаун
        
        // Сбрасываем состояние после удара
        m_AutoTripleFlyThrow = {};
        return;
    }

    // Иначе зажимаем хук на цели
    DummyInput.m_Hook = 1;
    vec2 Dir = normalize(TargetPos - DummyPos);
    DummyInput.m_TargetX = (int)(Dir.x * 100.0f);
    DummyInput.m_TargetY = (int)(Dir.y * 100.0f);
}

void CTripleHookHolder::HandleAutoTripleFlyThrowBoth()
{
    if(!g_Config.m_MRXAutotripleflyThrowBoth)
    {
        m_AutoTripleFlyThrowBoth = {};
        m_GlobalCooldownThrowBoth = 0; // Сбрасываем глобальный кулдаун
        return;
    }

    // Проверяем глобальный кулдаун
    if(m_GlobalCooldownThrowBoth > 0)
    {
        m_GlobalCooldownThrowBoth--;
        return;
    }

    int DummyId = GameClient()->m_aLocalIds[!g_Config.m_ClDummy];
    if(DummyId < 0)
    {
        m_AutoTripleFlyThrowBoth = {};
        return;
    }

    auto& DummyInput = GameClient()->m_DummyInput;
    const vec2 DummyPos = GameClient()->m_aClients[DummyId].m_Predicted.m_Pos;
    const vec2 LocalPos = GameClient()->m_LocalCharacterPos;

    // Ищем ближайшего к курсору основного игрока
    int TargetId = FindTargetNearMainCursor();
    
    // Если нет активной цели и нашли новую цель
    if(!m_AutoTripleFlyThrowBoth.Active && TargetId != -1)
    {
        m_AutoTripleFlyThrowBoth.ClientId = TargetId;
        m_AutoTripleFlyThrowBoth.Active = true;
        m_AutoTripleFlyThrowBoth.HasThrown = false;
    }
    // Если есть активная цель, но текущая цель стала невалидной
    else if(m_AutoTripleFlyThrowBoth.Active && TargetId != -1 && TargetId != m_AutoTripleFlyThrowBoth.ClientId)
    {
        // Проверяем, валидна ли текущая цель
        if(m_AutoTripleFlyThrowBoth.ClientId < 0 || !GameClient()->m_aClients[m_AutoTripleFlyThrowBoth.ClientId].m_Active)
        {
            // Текущая цель невалидна, переключаемся на новую
            m_AutoTripleFlyThrowBoth.ClientId = TargetId;
            m_AutoTripleFlyThrowBoth.HasThrown = false;
        }
        // Иначе оставляем текущую цель (не меняем постоянно)
    }
    
    // Если не нашли цель и нет активной цели
    if(TargetId == -1 && !m_AutoTripleFlyThrowBoth.Active)
    {
        DummyInput.m_Hook = 0;
        return;
    }

    // Если цель невалидна — сбрасываем
    if(m_AutoTripleFlyThrowBoth.ClientId < 0 || !GameClient()->m_aClients[m_AutoTripleFlyThrowBoth.ClientId].m_Active)
    {
        m_AutoTripleFlyThrowBoth = {};
        DummyInput.m_Hook = 0;
        return;
    }

    // Получаем актуальные координаты цели
    const vec2 TargetPos = GameClient()->m_aClients[m_AutoTripleFlyThrowBoth.ClientId].m_Predicted.m_Pos;
    float DistToDummy = distance(DummyPos, TargetPos);
    float DistToLocal = distance(LocalPos, TargetPos);

    // Если цель находится слишком далеко от дамми — сбрасываем
    if(DistToDummy > 400.0f)
    {
        m_AutoTripleFlyThrowBoth = {};
        DummyInput.m_Hook = 0;
        return;
    }

    // Если цель в зоне удара и дамми, и основного игрока — делаем удар
    if(DistToDummy <= 63.0f && DistToLocal <= 63.0f)
    {
        DummyInput.m_Hook = 0; // Отпускаем хук от игрока
        
        // Удар дамми
        vec2 DirToDummy = normalize(TargetPos - DummyPos);
        DummyInput.m_TargetX = (int)(DirToDummy.x * 100.0f);
        DummyInput.m_TargetY = (int)(DirToDummy.y * 100.0f);
        DummyInput.m_Fire++; // Делаем удар дамми
        
        // Удар основного игрока
        vec2 DirToLocal = normalize(TargetPos - LocalPos);
        GameClient()->m_Controls.m_aInputData[g_Config.m_ClDummy].m_TargetX = (int)(DirToLocal.x * 100.0f);
        GameClient()->m_Controls.m_aInputData[g_Config.m_ClDummy].m_TargetY = (int)(DirToLocal.y * 100.0f);
        GameClient()->m_Controls.m_aInputData[g_Config.m_ClDummy].m_Fire++; // Делаем удар основного игрока
        
        m_AutoTripleFlyThrowBoth.HasThrown = true;
        m_GlobalCooldownThrowBoth = g_Config.m_MRXAutotripleflyCooldown; // Устанавливаем глобальный кулдаун
        
        // Сбрасываем состояние после удара
        m_AutoTripleFlyThrowBoth = {};
        return;
    }

    // Иначе зажимаем хук на цели
    DummyInput.m_Hook = 1;
    vec2 Dir = normalize(TargetPos - DummyPos);
    DummyInput.m_TargetX = (int)(Dir.x * 100.0f);
    DummyInput.m_TargetY = (int)(Dir.y * 100.0f);
}

bool CTripleHookHolder::IsTripleThrowBothTargetInStrikeRange() const
{
    if(!g_Config.m_MRXAutotripleflyThrowBoth || !m_AutoTripleFlyThrowBoth.Active)
        return false;
    
    if(m_AutoTripleFlyThrowBoth.ClientId < 0 || !GameClient()->m_aClients[m_AutoTripleFlyThrowBoth.ClientId].m_Active)
        return false;
    
    int DummyId = GameClient()->m_aLocalIds[!g_Config.m_ClDummy];
    if(DummyId < 0)
        return false;
    
    const vec2 DummyPos = GameClient()->m_aClients[DummyId].m_Predicted.m_Pos;
    const vec2 LocalPos = GameClient()->m_LocalCharacterPos;
    const vec2 TargetPos = GameClient()->m_aClients[m_AutoTripleFlyThrowBoth.ClientId].m_Predicted.m_Pos;
    
    float DistToDummy = distance(DummyPos, TargetPos);
    float DistToLocal = distance(LocalPos, TargetPos);
    
    // Проверяем, находится ли цель в зоне удара (63.0f - радиус молота)
    return DistToDummy <= 63.0f && DistToLocal <= 63.0f;
}