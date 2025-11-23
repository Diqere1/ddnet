#include "autolaserunfreeze.h"
#include <game/client/gameclient.h>
#include <engine/shared/config.h>

void CAutolaserUnfreeze::OnRender()
{
    if(g_Config.m_NnLaserUnfreeze)
        HandleAutoLaserUnfreeze();
}

void CAutolaserUnfreeze::HandleAutoLaserUnfreeze()
{
    if(!g_Config.m_NnLaserUnfreeze || !GameClient()->m_Snap.m_pLocalCharacter)
        return;

    int DummyId = GameClient()->m_aLocalIds[!g_Config.m_ClDummy];
    if(DummyId < 0)
        return;

    // Проверяем состояние разморозки у основного игрока
    const CCharacterCore* pLocalChar = &GameClient()->m_aClients[GameClient()->m_Snap.m_LocalClientId].m_Predicted;
    bool IsUnfreezing = !pLocalChar->m_IsInFreeze && pLocalChar->m_FreezeEnd > GameClient()->Client()->GameTick(g_Config.m_ClDummy);

    if(IsUnfreezing)
{
    // Сохраняем прошлое оружие дамми
    if(!m_WasFired)
    {
        m_LastWeapon = GameClient()->m_DummyInput.m_WantedWeapon;
        m_WasFired = true;
        m_WeaponSwitched = false;
    }

    // Наводим дамми на игрока
    vec2 DummyPos = GameClient()->m_aClients[DummyId].m_Predicted.m_Pos;
    vec2 LocalPos = GameClient()->m_LocalCharacterPos;
    vec2 Dir = normalize(LocalPos - DummyPos);

    GameClient()->m_DummyInput.m_TargetX = static_cast<int>(Dir.x * 100.0f);
    GameClient()->m_DummyInput.m_TargetY = static_cast<int>(Dir.y * 100.0f);

    // Сначала только переключаемся на лазер
    if(GameClient()->m_DummyInput.m_WantedWeapon != 5 && !m_WeaponSwitched)
    {
        GameClient()->m_DummyInput.m_WantedWeapon = 5; // 5 — лазер
        GameClient()->m_DummyInput.m_Fire = 0;
        m_WeaponSwitched = true;
    }
    // На следующем тике, когда лазер выбран, стреляем
    else if(GameClient()->m_DummyInput.m_WantedWeapon == 5 && m_WeaponSwitched)
    {
        GameClient()->m_DummyInput.m_Fire = 1;
    }
}
else if(m_WasFired)
{
    // Возвращаем прошлое оружие после выстрела
    GameClient()->m_DummyInput.m_WantedWeapon = m_LastWeapon;
    GameClient()->m_DummyInput.m_Fire = 0;
    m_WasFired = false;
    m_WeaponSwitched = false;
}
}