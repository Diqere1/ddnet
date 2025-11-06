#include "pixelwalk.h"
#include <engine/shared/config.h>
#include <game/client/gameclient.h>
#include <base/system.h>

void CPixelWalk::OnRender()
{
    if(!GameClient()->m_Snap.m_pLocalCharacter)
        return;
}

void CPixelWalk::HandlePixelWalk()
{
    if(!g_Config.m_MRXPixelWalk || !GameClient()->m_Snap.m_pLocalCharacter)
        return;

    int Dummy = g_Config.m_ClDummy;
    int &Left = GameClient()->m_Controls.m_aInputDirectionLeft[Dummy];
    int &Right = GameClient()->m_Controls.m_aInputDirectionRight[Dummy];
    CNetObj_PlayerInput &Input = GameClient()->m_Controls.m_aInputData[Dummy];

    static bool s_LeftPrev = false;
    static bool s_RightPrev = false;
    static int s_PendingReverse = 0;

    // Если ожидается обратный тик, делаем его и сбрасываем
    if(s_PendingReverse != 0)
    {
        Input.m_Direction = -s_PendingReverse;
        s_PendingReverse = 0;
        return;
    }

    // Фронт нажатия — активируем движение и ставим обратный тик
    if(Left && !s_LeftPrev)
    {
        Input.m_Direction = -1;
        s_PendingReverse = -1;
    }
    else if(Right && !s_RightPrev)
    {
        Input.m_Direction = 1;
        s_PendingReverse = 1;
    }
    else
    {
        Input.m_Direction = 0;
    }

    s_LeftPrev = Left;
    s_RightPrev = Right;
}