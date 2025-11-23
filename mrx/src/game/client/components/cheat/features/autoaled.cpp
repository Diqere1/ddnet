#include "autoaled.h"
#include <game/client/gameclient.h>

void CAutoAled::OnRender()
{
    // Не вызываем здесь логику: обработка выполняется в Controls::SnapInput(), чтобы избежать двойного вызова за кадр
}

bool CAutoAled::IsValidTarget(int TargetID) const
{
    // Проверяем состояние фриза игрока
    const CCharacterCore* pCharCore = &GameClient()->m_aClients[TargetID].m_Predicted;
    
    // Цель должна быть в состоянии разморозки:
    // - Не в фризе (!m_IsInFreeze)
    // - Но всё ещё идет время разморозки (m_FreezeEnd > GameTick)
    if(!pCharCore->m_IsInFreeze && 
       pCharCore->m_FreezeEnd > GameClient()->Client()->GameTick(g_Config.m_ClDummy))
    {
        return true;
    }
    
    return false;
}

bool CAutoAled::HasFreezeTileBetween(const vec2& from, const vec2& to) const 
{
    vec2 dir = normalize(to - from); // Направление от игрока к цели
    float dist = distance(from, to); // Расстояние между игроком и целью

    const float TileSize = 32.0f; // Размер тайла

    for(float i = TileSize / 2; i < dist - TileSize / 2; i += TileSize / 2) // Шаг проверки
    {
        vec2 checkPos = from + dir * i; // Точка на линии между игроком и целью

        // Получаем индекс тайла в этой точке
        int TileIndex = GameClient()->Collision()->GetTileIndex(
            GameClient()->Collision()->GetPureMapIndex(checkPos)
        );

        // Проверяем, является ли тайл фризом
        if(TileIndex == TILE_FREEZE || TileIndex == TILE_DFREEZE || TileIndex == TILE_LFREEZE)
        {
            return true; // Тайл фриза строго между игроком и целью
        }
    }
    return false; // Между игроком и целью нет тайлов фриза
}

bool CAutoAled::FindClosestTarget(vec2 &TargetPos)
{
    if(GameClient()->m_Snap.m_LocalClientId < 0)
        return false;

    int LocalID = GameClient()->m_Snap.m_LocalClientId;
    vec2 LocalPos = GameClient()->m_LocalCharacterPos;

    float ClosestDistance = 63; // Максимальная дистанция для поиска цели
    int ClosestId = -1;

    for(int i = 0; i < MAX_CLIENTS; i++)
    {
        if(i == LocalID || !GameClient()->m_Snap.m_aCharacters[i].m_Active)
            continue;

        if(!IsValidTarget(i))
            continue;

        vec2 PlayerPos = GameClient()->m_aClients[i].m_Predicted.m_Pos;
        float Distance = distance(LocalPos, PlayerPos);

        if(Distance > 0 && Distance < ClosestDistance)
        {
            // Проверяем наличие тайла фриза между игроком и целью
            bool hasFreezeBlock = HasFreezeTileBetween(LocalPos, PlayerPos);
            

            if(hasFreezeBlock) // Удар возможен только если между игроком и целью есть тайл фриза
            {
                ClosestDistance = Distance;
                ClosestId = i;
            }
        }
    }

    if(ClosestId >= 0)
    {
        TargetPos = GameClient()->m_aClients[ClosestId].m_Predicted.m_Pos;
        return true;
    }

    return false;
}

void CAutoAled::HandleAutoAled()
{
    // Сначала отпустим удар, если в прошлый тик мы его зажимали авто-логикой
    if(m_LastHitState)
    {
        if((GameClient()->m_Controls.m_aInputData[g_Config.m_ClDummy].m_Fire & 1) != 0)
            GameClient()->m_Controls.m_aInputData[g_Config.m_ClDummy].m_Fire++;
        m_LastHitState = false;
    }

    if(!g_Config.m_MRXAutoAled || !GameClient()->m_Snap.m_pLocalCharacter)
    {
        // Выходим после возможного отпускания удара
        return;
    }

    // Отключаем autoaled, если И игрок, и дамми во фризе
    bool LocalFrozen = GameClient()->m_PredictedChar.m_IsInFreeze;
    bool DummyFrozen = false;
    {
        int DummyId = GameClient()->m_aLocalIds[!g_Config.m_ClDummy];
        if(DummyId >= 0)
            DummyFrozen = GameClient()->m_aClients[DummyId].m_Predicted.m_IsInFreeze;
    }
    if(LocalFrozen && DummyFrozen)
    {
        // Ничего не делаем: не трогаем m_Fire, чтобы не создавать лишние фронты
        return;
    }

    vec2 TargetPos;
    if(FindClosestTarget(TargetPos)) // Удар только если между вами и целью есть тайл фриза
    {
        vec2 LocalPos = GameClient()->m_LocalCharacterPos;
        vec2 Direction = normalize(TargetPos - LocalPos);

        if(g_Config.m_MRXAutoAledSilent)
        {
            vec2 CurrentMousePos = GameClient()->m_Controls.m_aMousePos[g_Config.m_ClDummy];
            GameClient()->m_Controls.m_aInputData[g_Config.m_ClDummy].m_TargetX = static_cast<int>(Direction.x * 100.0f);
            GameClient()->m_Controls.m_aInputData[g_Config.m_ClDummy].m_TargetY = static_cast<int>(Direction.y * 100.0f);
            GameClient()->m_Controls.m_aMousePos[g_Config.m_ClDummy] = CurrentMousePos;
        }
        else
        {
            GameClient()->m_Controls.m_aMousePos[g_Config.m_ClDummy] = Direction * 100.0f;
            GameClient()->m_Controls.m_aInputData[g_Config.m_ClDummy].m_TargetX = static_cast<int>(Direction.x * 100.0f);
            GameClient()->m_Controls.m_aInputData[g_Config.m_ClDummy].m_TargetY = static_cast<int>(Direction.y * 100.0f);
        }

        if(GameClient()->m_Snap.m_pLocalCharacter && GameClient()->m_Snap.m_pLocalCharacter->m_Weapon == WEAPON_HAMMER && distance(LocalPos, TargetPos) <= 63)
        {
            // Генерируем удар только по фронту (если кнопка не считается зажатой)
            if((GameClient()->m_Controls.m_aInputData[g_Config.m_ClDummy].m_Fire & 1) == 0)
            {
                GameClient()->m_Controls.m_aInputData[g_Config.m_ClDummy].m_Fire++;
                m_LastHitState = true; // выполним отпускание в начале следующего тика
            }
        }
    }
}

void CAutoAled::ResetInput()
{
    if(m_LastHitState)
    {
        GameClient()->m_Controls.m_aInputData[g_Config.m_ClDummy].m_Fire &= ~1;
        m_LastHitState = false;
    }
    if((GameClient()->m_Controls.m_aInputData[g_Config.m_ClDummy].m_Fire & 1) != 0)

    GameClient()->m_Controls.m_aInputData[g_Config.m_ClDummy].m_Fire &= INPUT_STATE_MASK;
    GameClient()->m_Controls.m_aInputData[g_Config.m_ClDummy].m_WantedWeapon = 0;
}