
#include "customchat.h"
#include <engine/shared/config.h>
#include <game/client/gameclient.h>
#include <engine/textrender.h>
#include <game/client/render.h>

void CCustomChat::OnRender()
{
    if(!g_Config.m_ClShowCustomChat)
        return;

    float Points[4];
    Graphics()->GetScreen(Points, Points+1, Points+2, Points+3);

    for(int i = 0; i < MAX_CLIENTS; i++)
    {
        const CFloatingMessage &Message = m_aFloatingMessages[i];
        if(!Message.m_Active || time_get() > Message.m_EndTime)
            continue;

        if(!GameClient()->m_aClients[i].m_Active) 
            continue;

            if(GameClient()->IsOtherTeam(i))
            continue;

        vec2 Position = GameClient()->m_aClients[i].m_RenderPos;
        Position.y -= 50.0f;



        // Calculate alpha for fade in/out
        float TimePassed = (time_get() - Message.m_StartTime) / (float)time_freq();
        float Alpha = 1.0f;
        if(TimePassed < 0.25f)
            Alpha = TimePassed * 4.0f;
        else if(time_get() > Message.m_EndTime - time_freq() * 0.25f)
            Alpha = (Message.m_EndTime - time_get()) / (float)(time_freq() * 0.25f);

       float TextWidth = TextRender()->TextWidth(g_Config.m_ClCustomChatTextSize, Message.m_aText, -1);
// Изменяем размер бокса относительно размера текста
float BoxWidth = TextWidth + g_Config.m_ClCustomChatTextSize * 0.5f; // Padding пропорционален размеру текста
float BoxHeight = g_Config.m_ClCustomChatTextSize * 1.25f; // Высота бокса пропорциональна размеру текста 

// Main background box using CUIRect like in ArrayList
CUIRect MainBg = {
    Position.x - BoxWidth/2.0f - 2.0f,
    Position.y - BoxHeight/2.0f,
    BoxWidth + 4.0f,
    BoxHeight
};
MainBg.Draw(ColorRGBA(0.1f, 0.1f, 0.1f, 0.85f * Alpha), IGraphics::CORNER_ALL, 3.0f);

// Accent line on the left
Graphics()->TextureClear();
Graphics()->QuadsBegin();
Graphics()->SetColor(1.0f, 1.0f, 1.0f, Alpha);
IGraphics::CQuadItem QuadLine(
    Position.x - BoxWidth/2.0f - 2.0f,
    Position.y - BoxHeight/2.0f,
    2.0f, 
    BoxHeight
);
Graphics()->QuadsDrawTL(&QuadLine, 1);
Graphics()->QuadsEnd();

// Adjust text position relative to new box size
float TextY = Position.y - (g_Config.m_ClCustomChatTextSize/2.0f); // Центрируем текст по вертикали

// Text with shadow
TextRender()->TextColor(0.0f, 0.0f, 0.0f, 0.5f * Alpha); // Shadow
TextRender()->Text(
    Position.x - TextWidth/2.0f + 1.0f,
    TextY + 1.0f,
    g_Config.m_ClCustomChatTextSize,
    Message.m_aText,
    -1.0f
);

TextRender()->TextColor(1.0f, 1.0f, 1.0f, Alpha); // Main text
TextRender()->Text(
    Position.x - TextWidth/2.0f,
    TextY,
    g_Config.m_ClCustomChatTextSize, 
    Message.m_aText,
    -1.0f
);
        TextRender()->TextColor(1.0f, 1.0f, 1.0f, 1.0f);
    }

    Graphics()->MapScreen(Points[0], Points[1], Points[2], Points[3]);
}

void CCustomChat::AddMessage(int ClientID, const char *pText)
{
    if(ClientID < 0 || ClientID >= MAX_CLIENTS)
        return;

    CFloatingMessage &Message = m_aFloatingMessages[ClientID];
    Message.m_Active = true;
    Message.m_StartTime = time_get();
    Message.m_EndTime = time_get() + time_freq() * g_Config.m_ClCustomChatDisplayTime; // Используем новую переменную
    str_copy(Message.m_aText, pText);

    if(str_length(Message.m_aText) > 256)
    {
        Message.m_aText[256] = '\0';
    }
}


void CCustomChat::OnMessage(int MsgType, void *pRawMsg)
{
    if(MsgType == NETMSGTYPE_SV_CHAT)
    {
        CNetMsg_Sv_Chat *pMsg = (CNetMsg_Sv_Chat *)pRawMsg;
        if(pMsg->m_ClientId >= 0)
        {
            const char* pPlayerName = GameClient()->m_aClients[pMsg->m_ClientId].m_aName;
            AddMessage(pMsg->m_ClientId, pMsg->m_pMessage);
        }
    }
}

void CCustomChat::OnInit()
{
    // Initialize message array
    for(int i = 0; i < MAX_CLIENTS; i++)
    {
        m_aFloatingMessages[i].m_Active = false;
        m_aFloatingMessages[i].m_aText[0] = '\0';
    }
}