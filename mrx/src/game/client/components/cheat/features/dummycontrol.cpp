#include "dummycontrol.h"
#include <engine/shared/config.h>
#include <game/client/gameclient.h>

void CDummyControl::OnConsoleInit()
{
    Console()->Register("cl_dummy_left", "", CFGFLAG_CLIENT, ConDummyLeft, this, "Move dummy left");
    Console()->Register("cl_dummy_right", "", CFGFLAG_CLIENT, ConDummyRight, this, "Move dummy right");
    Console()->Register("cl_dummy_stop", "", CFGFLAG_CLIENT, ConDummyStop, this, "Stop dummy movement");
}



void CDummyControl::ConDummyLeft(IConsole::IResult *pResult, void *pUserData)
{
    CDummyControl *pDummyControl = (CDummyControl *)pUserData;
    if (pDummyControl->GameClient()->m_DummyInput.m_Direction == -1)
        pDummyControl->Stop();
    else
        pDummyControl->MoveLeft();
}

void CDummyControl::ConDummyRight(IConsole::IResult *pResult, void *pUserData)
{
    CDummyControl *pDummyControl = (CDummyControl *)pUserData;
    if (pDummyControl->GameClient()->m_DummyInput.m_Direction == 1)
        pDummyControl->Stop();
    else
        pDummyControl->MoveRight();
}

void CDummyControl::ConDummyStop(IConsole::IResult *pResult, void *pUserData)
{
    CDummyControl *pDummyControl = (CDummyControl *)pUserData;
    pDummyControl->Stop();
}

void CDummyControl::MoveLeft()
{
    // Обновляем прямой инпут дамми (используется, когда дамми не под контролем и cl_dummy_copy_moves выключен)
    GameClient()->m_DummyInput.m_Direction = -1;

    // Правильная интеграция: выставляем состояние "клавиш" для противоположного слота,
    // чтобы после переключения сторона продолжила движение.
    GameClient()->m_Controls.m_aInputDirectionLeft[!g_Config.m_ClDummy] = 1;
    GameClient()->m_Controls.m_aInputDirectionRight[!g_Config.m_ClDummy] = 0;
}

void CDummyControl::MoveRight()
{
    // Обновляем прямой инпут дамми
    GameClient()->m_DummyInput.m_Direction = 1;

    // Выставляем состояния "клавиш" для противоположного слота
    GameClient()->m_Controls.m_aInputDirectionLeft[!g_Config.m_ClDummy] = 0;
    GameClient()->m_Controls.m_aInputDirectionRight[!g_Config.m_ClDummy] = 1;

}

void CDummyControl::Stop()
{
   // Останавливаем прямой инпут дамми
   GameClient()->m_DummyInput.m_Direction = 0;

   // Сбрасываем состояния "клавиш" для противоположного слота
   GameClient()->m_Controls.m_aInputDirectionLeft[!g_Config.m_ClDummy] = 0;
   GameClient()->m_Controls.m_aInputDirectionRight[!g_Config.m_ClDummy] = 0;
}