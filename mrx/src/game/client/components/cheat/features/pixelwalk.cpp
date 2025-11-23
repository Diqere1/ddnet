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

    // Проверка стен вплотную слева/справа от игрока
    int LocalId = GameClient()->m_aLocalIds[Dummy];
    if(LocalId < 0)
        return;
    const vec2 Pos = GameClient()->m_aClients[LocalId].m_Predicted.m_Pos;
    const float R = 15.0f; // радиус ти
    const float Eps = 0.01f;
    const bool WallRight = GameClient()->Collision()->CheckPoint((int)(Pos.x + R + Eps), (int)Pos.y);
    const bool WallLeft = GameClient()->Collision()->CheckPoint((int)(Pos.x - R - Eps), (int)Pos.y);

    // Если ожидается обратный тик, делаем его и сбрасываем
    if(s_PendingReverse != 0)
    {
        int Desired = -s_PendingReverse; // -1: влево, 1: вправо
        if((Desired < 0 && WallLeft) || (Desired > 0 && WallRight))
        {
            // Если обратный тик упирается в стену, гасим его
            s_PendingReverse = 0;
            Input.m_Direction = 0;
            return;
        }
        Input.m_Direction = Desired;
        s_PendingReverse = 0;
        return;
    }

    // Фронт нажатия — активируем движение и ставим обратный тик
    if(Left && !s_LeftPrev)
    {
        if(!WallLeft)
        {
            Input.m_Direction = -1;
            s_PendingReverse = -1;
        }
        else
        {
            // Влево упёрлись в стену — ничего не делаем и не ставим реверс
            Input.m_Direction = 0;
            s_PendingReverse = 0;
        }
    }
    else if(Right && !s_RightPrev)
    {
        if(!WallRight)
        {
            Input.m_Direction = 1;
            s_PendingReverse = 1;
        }
        else
        {
            // Вправо упёрлись в стену — ничего не делаем и не ставим реверс
            Input.m_Direction = 0;
            s_PendingReverse = 0;
        }
    }
    else
    {
        Input.m_Direction = 0;
    }

    s_LeftPrev = Left;
    s_RightPrev = Right;
}