#include "dummyaled.h"
#include <game/client/gameclient.h>
void CDummyAled::OnRender()
{
    if(g_Config.m_MRXDummyAled)
        HandleDummyAled();
}
bool CDummyAled::HasFreezeTileBetween(const vec2 &from, const vec2 &to) const
{
    vec2 dir = normalize(to - from);
    float dist = distance(from, to);
    const float TileSize = 32.0f;
    for(float i = 0; i < dist; i += TileSize / 8)
    {
        vec2 checkPos = from + dir * i;
        int TileX = static_cast<int>(checkPos.x / TileSize);
        int TileY = static_cast<int>(checkPos.y / TileSize);
        int TileIndex = GameClient()->Collision()->GetTileIndex(
            GameClient()->Collision()->GetPureMapIndex(checkPos));
        if(TileIndex == TILE_FREEZE ||
            TileIndex == TILE_DFREEZE ||
            TileIndex == TILE_LFREEZE)
        {
            return true;
        }
    }
    return false;
}
bool CDummyAled::FindTarget(vec2 &TargetPos)
{
    if(GameClient()->m_Snap.m_LocalClientId < 0)
        return false;
    int LocalID = GameClient()->m_Snap.m_LocalClientId;
    int DummyId = GameClient()->m_aLocalIds[!g_Config.m_ClDummy];
    if(DummyId < 0)
        return false;
    vec2 LocalPos = GameClient()->m_aClients[LocalID].m_RenderPos;
    vec2 DummyPos = GameClient()->m_aClients[DummyId].m_RenderPos;
    float Distance = distance(LocalPos, DummyPos);
    if(Distance <= 0 || Distance >= 63)
        return false;
    const CCharacterCore *pCharCore = &GameClient()->m_aClients[LocalID].m_Predicted;
    if(!pCharCore->m_IsInFreeze &&
        pCharCore->m_FreezeEnd > GameClient()->Client()->GameTick(g_Config.m_ClDummy) &&
        HasFreezeTileBetween(DummyPos, LocalPos))
    {
        TargetPos = LocalPos;
        return true;
    }
    return false;
}
void CDummyAled::HandleDummyAled()
{
    m_IsControllingAim = false;
    if(!g_Config.m_MRXDummyAled || !GameClient()->m_Snap.m_pLocalCharacter)
    {
        return;
    }
    bool LocalFrozen = GameClient()->m_PredictedChar.m_IsInFreeze;
    bool DummyFrozen = false;
    {
        int DummyIdCheck = GameClient()->m_aLocalIds[!g_Config.m_ClDummy];
        if(DummyIdCheck >= 0)
            DummyFrozen = GameClient()->m_aClients[DummyIdCheck].m_Predicted.m_IsInFreeze;
    }
    if(DummyFrozen)
    {
        ResetDummyInput();
        return;
    }
    vec2 TargetPos;
    static bool WasFound = false;
    if(FindTarget(TargetPos))
    {
        m_IsControllingAim = true;
        WasFound = true;
        int DummyId = GameClient()->m_aLocalIds[!g_Config.m_ClDummy];
        if(DummyId >= 0)
        {
            vec2 DummyPos = GameClient()->m_aClients[DummyId].m_RenderPos;
            vec2 Direction = normalize(TargetPos - DummyPos);
            GameClient()->m_DummyInput.m_TargetX = (int)(Direction.x * 100.0f);
            GameClient()->m_DummyInput.m_TargetY = (int)(Direction.y * 100.0f);
            if((GameClient()->m_DummyInput.m_Fire & 1) == 0)
                GameClient()->m_DummyInput.m_Fire++;
            GameClient()->m_DummyInput.m_WantedWeapon = WEAPON_HAMMER;
            GameClient()->m_Controls.m_aInputData[!g_Config.m_ClDummy].m_TargetX = GameClient()->m_DummyInput.m_TargetX;
            GameClient()->m_Controls.m_aInputData[!g_Config.m_ClDummy].m_TargetY = GameClient()->m_DummyInput.m_TargetY;
        }
    }
    else if(WasFound)
    {
        WasFound = false;
    }
}
void CDummyAled::ResetDummyInput()
{
    if((GameClient()->m_DummyInput.m_Fire & 1) != 0)
        GameClient()->m_DummyInput.m_Fire++;
    GameClient()->m_DummyInput.m_Fire &= INPUT_STATE_MASK;
    GameClient()->m_DummyInput.m_WantedWeapon = 0;
    GameClient()->m_Controls.m_aInputData[!g_Config.m_ClDummy].m_TargetX = 0;
    GameClient()->m_Controls.m_aInputData[!g_Config.m_ClDummy].m_TargetY = 0;
}