#include "dummyhookcursor.h"
#include <game/client/gameclient.h>
#include <engine/shared/config.h>

void CDummyHookCursor::OnRender()
{
    if(g_Config.m_MRXDummyHookCursor)
        HandleDummyHookCursor();
}

void CDummyHookCursor::HandleDummyHookCursor()
{
    if(!GameClient()->m_Snap.m_pLocalCharacter || !GameClient()->Client()->DummyConnected())
        return;

    if(!g_Config.m_ClDummyHook)
        return;

    if(GameClient()->m_DummyAled.IsControllingAim())
        return;

    if(g_Config.m_MRXDummyHookCursor)
    {
        float CameraZoom = GameClient()->m_Camera.m_Zoom;
        vec2 MousePos = GameClient()->m_Controls.m_aMousePos[g_Config.m_ClDummy];
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

        int DummyId = GameClient()->m_aLocalIds[!g_Config.m_ClDummy];
        if(DummyId >= 0)
        {
            vec2 DummyPos = GameClient()->m_aClients[DummyId].m_Predicted.m_Pos;

            vec2 Direction = normalize(WorldPos - DummyPos);

            GameClient()->m_Controls.m_aInputData[!g_Config.m_ClDummy].m_TargetX = (int)(Direction.x * 100.0f);
            GameClient()->m_Controls.m_aInputData[!g_Config.m_ClDummy].m_TargetY = (int)(Direction.y * 100.0f);

            GameClient()->m_DummyInput.m_TargetX = GameClient()->m_Controls.m_aInputData[!g_Config.m_ClDummy].m_TargetX;
            GameClient()->m_DummyInput.m_TargetY = GameClient()->m_Controls.m_aInputData[!g_Config.m_ClDummy].m_TargetY;
        }
    }
}