#include "playertarget.h"
#include <game/client/gameclient.h>
#include <game/client/render.h>
#include <engine/graphics.h>

void CPlayerTarget::OnConsoleInit()
{
    // Регистрируем консольную команду для включения/выключения
    Console()->Register("player_target", "", CFGFLAG_CLIENT, ConToggle, this, 
        "Toggle player targeting (shows triangle above nearest valid player)");
    
    
}

void CPlayerTarget::ConToggle(IConsole::IResult *pResult, void *pUserData)
{
    CPlayerTarget *pPlayerTarget = (CPlayerTarget *)pUserData;
    pPlayerTarget->m_Active = !pPlayerTarget->m_Active;
    pPlayerTarget->GameClient()->Console()->Print(
        IConsole::OUTPUT_LEVEL_STANDARD,
        "player_target",
        pPlayerTarget->m_Active ? "Player targeting enabled" : "Player targeting disabled");
}



bool CPlayerTarget::IsValidTarget(int ClientId) const
{
   {
    // Получаем информацию о клиенте и локальном игроке
    const CGameClient::CClientData &Client = GameClient()->m_aClients[ClientId];
    const CGameClient::CClientData &LocalClient = GameClient()->m_aClients[GameClient()->m_aLocalIds[0]];
    
    // Проверяем что:
    // 1. Клиент активен
    // 2. Не в режиме наблюдателя
    // 3. Не является основным игроком/дамми
    // 4. В другой команде
    // 5. Не в команде наблюдателей
    return Client.m_Active && 
           !Client.m_Spec &&
           ClientId != GameClient()->m_aLocalIds[0] &&  
           ClientId != GameClient()->m_aLocalIds[1] &&  
           Client.m_Team == LocalClient.m_Team && // Исправлено условие проверки команды
           Client.m_Team != TEAM_SPECTATORS;
}
}

std::optional<CPlayerTarget::STarget> CPlayerTarget::FindNearestTarget() const 
{
    float MinDistance = std::numeric_limits<float>::max();
    std::optional<STarget> NearestTarget;
    
    // Получаем позицию локального игрока для расчета дистанции
    vec2 LocalPos = GameClient()->m_LocalCharacterPos;

    // Перебираем всех клиентов
    for(int i = 0; i < MAX_CLIENTS; i++)
    {
        // Проверяем, является ли клиент валидной целью
        if(!IsValidTarget(i))
            continue;

        // Получаем позицию клиента
        vec2 PlayerPos = GameClient()->m_aClients[i].m_Predicted.m_Pos;
        float Distance = distance(LocalPos, PlayerPos);

        // Обновляем ближайшую цель
        if(Distance < MinDistance)
        {
            MinDistance = Distance;
            NearestTarget = STarget(i, PlayerPos, Distance);
        }
    }

    return NearestTarget;
}

void CPlayerTarget::RenderTriangle(const vec2& Position) const
{
    // Сохраняем текущие точки экрана
    float Points[4];
    Graphics()->GetScreen(Points, Points+1, Points+2, Points+3);

    const float TriangleSize = 16.0f;
    const float TriangleHeight = 42.0f;

    // Создаем прямоугольник для позиционирования треугольника
    CUIRect TriangleRect = {
        Position.x - TriangleSize/2.0f,
        Position.y - TriangleHeight,
        TriangleSize,
        TriangleSize
    };

    // Очищаем текстуру и начинаем отрисовку
    Graphics()->TextureClear();
    Graphics()->QuadsBegin();
    
    // Используем цвет из конфига
    float r = (g_Config.m_ClPlayerTargetColor >> 24) / 255.0f;
    float g = ((g_Config.m_ClPlayerTargetColor >> 16) & 0xff) / 255.0f;
    float b = ((g_Config.m_ClPlayerTargetColor >> 8) & 0xff) / 255.0f;
    float a = (g_Config.m_ClPlayerTargetColor & 0xff) / 255.0f;
    
    Graphics()->SetColor(r, g, b, a);

    // Определяем вершины треугольника относительно прямоугольника
    vec2 Vertices[3] = {
        vec2(TriangleRect.x + TriangleRect.w/2, TriangleRect.y + TriangleRect.h), // Нижняя вершина
        vec2(TriangleRect.x, TriangleRect.y), // Левый верхний угол
        vec2(TriangleRect.x + TriangleRect.w, TriangleRect.y)  // Правый верхний угол
    };

    // Создаем и отрисовываем треугольник
    IGraphics::CFreeformItem Freeform(
        Vertices[0].x, Vertices[0].y,
        Vertices[1].x, Vertices[1].y,
        Vertices[2].x, Vertices[2].y,
        Vertices[2].x, Vertices[2].y
    );
    Graphics()->QuadsDrawFreeform(&Freeform, 1);
    Graphics()->QuadsEnd();

    // Восстанавливаем параметры экрана
    Graphics()->MapScreen(Points[0], Points[1], Points[2], Points[3]);
}

std::optional<CPlayerTarget::STarget> CPlayerTarget::FindTargetByMousePos() const 
{
    float MinAngle = std::numeric_limits<float>::max();
    std::optional<STarget> NearestTarget;
    
    // Получаем позицию локального игрока и направление курсора
    vec2 LocalPos = GameClient()->m_LocalCharacterPos;
    vec2 MouseDir = normalize(vec2(GameClient()->m_Controls.m_aInputData[g_Config.m_ClDummy].m_TargetX, 
                                  GameClient()->m_Controls.m_aInputData[g_Config.m_ClDummy].m_TargetY));

    // Перебираем всех клиентов
    for(int i = 0; i < MAX_CLIENTS; i++)
    {
        if(!IsValidTarget(i))
            continue;

        // Получаем позицию клиента
        vec2 PlayerPos = GameClient()->m_aClients[i].m_Predicted.m_Pos;
        vec2 Dir = normalize(PlayerPos - LocalPos);
        
        // Вычисляем угол между направлением курсора и направлением к игроку
        float Angle = acos(dot(MouseDir, Dir));
        
        // Обновляем ближайшую цель по углу
        if(Angle < MinAngle)
        {
            MinAngle = Angle;
            NearestTarget = STarget(i, PlayerPos, length(PlayerPos - LocalPos));
        }
    }

    return NearestTarget;
}

std::optional<CPlayerTarget::STarget> CPlayerTarget::GetTarget() const
{
    if(m_CurrentTargetID == -1)
        return std::nullopt;
        
    // Проверяем, что цель всё ещё валидна
    if(!IsValidTarget(m_CurrentTargetID))
        return std::nullopt;
        
    vec2 PlayerPos = GameClient()->m_aClients[m_CurrentTargetID].m_Predicted.m_Pos;
    float Distance = length(PlayerPos - GameClient()->m_LocalCharacterPos);
    
    return STarget(m_CurrentTargetID, PlayerPos, Distance);
}

void CPlayerTarget::OnRender()
{
    bool WasActive = m_Active;
    
    if(g_Config.m_NnPlayerTarget)
        m_Active = true;
    else
        m_Active = false;

    // Если таргетинг только что включился, ищем новую цель
    if(m_Active && !WasActive)
    {
        auto NewTarget = FindTargetByMousePos();
        m_CurrentTargetID = NewTarget ? NewTarget->ClientId : -1;
    }
    else if(!m_Active)
    {
        m_CurrentTargetID = -1;
    }

    if(!m_Active || !GameClient()->m_Snap.m_pLocalCharacter)
        return;

    // Получаем текущую цель
    auto Target = GetTarget();
    if(!Target)
    {
        // Если текущая цель стала невалидной, ищем новую
        auto NewTarget = FindTargetByMousePos();
        m_CurrentTargetID = NewTarget ? NewTarget->ClientId : -1;
        Target = GetTarget();
    }

    if(Target)
        RenderTriangle(Target->Position);
}
