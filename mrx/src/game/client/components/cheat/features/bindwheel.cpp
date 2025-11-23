#include "bindwheel.h"
#include <engine/console.h>
#include <engine/shared/config.h>
#include <engine/graphics.h>
#include <engine/textrender.h>
#include <engine/input.h>
#include <game/client/gameclient.h>
#include <game/client/ui.h>
#include <game/client/render.h>
#include <algorithm>
#include <cstring>

CBindWheelMRX::CBindWheelMRX()
{
    OnReset();
}

void CBindWheelMRX::ConBindwheelExecuteHover(IConsole::IResult *pResult, void *pUserData)
{
    CBindWheelMRX *pThis = (CBindWheelMRX *)pUserData;
    pThis->ExecuteHoveredBind();
}

void CBindWheelMRX::ConOpenBindwheel(IConsole::IResult *pResult, void *pUserData)
{
    CBindWheelMRX *pThis = (CBindWheelMRX *)pUserData;
    if(pThis->Client()->State() != IClient::STATE_DEMOPLAYBACK)
    {
        bool NewActive = pResult->GetInteger(0) != 0;
        if(NewActive != pThis->m_Active)
        {
            pThis->m_Active = NewActive;
            // pThis->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "bindwheel", pThis->m_Active ? "Bind wheel opened" : "Bind wheel closed");
            if(pThis->m_Active)
            {
                // Reset selector mouse position to center when opening
                pThis->m_SelectorMouse = vec2(0.0f, 0.0f);
            }
        }
    }
}

void CBindWheelMRX::ConAddBindwheel(IConsole::IResult *pResult, void *pUserData)
{
    const char *pName = pResult->GetString(0);
    const char *pCommand = pResult->GetString(1);

    CBindWheelMRX *pThis = (CBindWheelMRX *)pUserData;
    pThis->AddBind(pName, pCommand);
}

void CBindWheelMRX::ConRemoveBindwheel(IConsole::IResult *pResult, void *pUserData)
{
    const char *pName = pResult->GetString(0);
    const char *pCommand = pResult->GetString(1);

    CBindWheelMRX *pThis = (CBindWheelMRX *)pUserData;
    pThis->RemoveBind(pName, pCommand);
}

void CBindWheelMRX::ConRemoveAllBindwheelBinds(IConsole::IResult *pResult, void *pUserData)
{
    CBindWheelMRX *pThis = (CBindWheelMRX *)pUserData;
    pThis->RemoveAllBinds();
}

void CBindWheelMRX::OnConsoleInit()
{
    Console()->Register("+bindwheel", "", CFGFLAG_CLIENT, ConOpenBindwheel, this, "Open bindwheel selector");
    Console()->Register("+bindwheel_execute_hover", "", CFGFLAG_CLIENT, ConBindwheelExecuteHover, this, "Execute hovered bindwheel bind");
    Console()->Register("add_bindwheel", "s[name] s[command]", CFGFLAG_CLIENT, ConAddBindwheel, this, "Add a bind to the bindwheel");
    Console()->Register("remove_bindwheel", "s[name] s[command]", CFGFLAG_CLIENT, ConRemoveBindwheel, this, "Remove a bind from the bindwheel");
    Console()->Register("delete_all_bindwheel_binds", "", CFGFLAG_CLIENT, ConRemoveAllBindwheelBinds, this, "Removes all bindwheel binds");
    
    // Register config save callback
    ConfigManager()->RegisterCallback(ConfigSaveCallback, this);
    
    // Load saved binds from config
    LoadBindsFromConfig();
}

void CBindWheelMRX::OnReset()
{
    m_WasActive = false;
    m_Active = false;
    m_SelectedBind = -1;
}

void CBindWheelMRX::OnRelease()
{
    m_Active = false;
}

bool CBindWheelMRX::OnCursorMove(float x, float y, IInput::ECursorType CursorType)
{
    if(!m_Active)
        return false;

    // Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "bindwheel", "OnCursorMove - blocking game cursor");
    Ui()->ConvertMouseMove(&x, &y, CursorType);
    m_SelectorMouse += vec2(x, y);
    return true; // This blocks the game cursor when active
}

bool CBindWheelMRX::OnInput(const IInput::CEvent &Event)
{
    if(IsActive() && Event.m_Flags & IInput::FLAG_PRESS && Event.m_Key == KEY_ESCAPE)
    {
        OnRelease();
        return true;
    }
    return false;
}

void CBindWheelMRX::AddBind(const char *pName, const char *pCommand)
{
    if((pName[0] == '\0' && pCommand[0] == '\0') || m_vBinds.size() >= MRX_BINDWHEEL_MAX_BINDS)
        return;

    CBind Bind;
    str_copy(Bind.m_aName, pName);
    str_copy(Bind.m_aCommand, pCommand);
    m_vBinds.push_back(Bind);
}

void CBindWheelMRX::RemoveBind(const char *pName, const char *pCommand)
{
    CBind Bind;
    str_copy(Bind.m_aName, pName);
    str_copy(Bind.m_aCommand, pCommand);
    auto it = std::find(m_vBinds.begin(), m_vBinds.end(), Bind);
    if(it != m_vBinds.end())
        m_vBinds.erase(it);
}

void CBindWheelMRX::RemoveBind(int Index)
{
    if(Index >= static_cast<int>(m_vBinds.size()) || Index < 0)
        return;
    auto Pos = m_vBinds.begin() + Index;
    m_vBinds.erase(Pos);
}

void CBindWheelMRX::RemoveAllBinds()
{
    m_vBinds.clear();
}

void CBindWheelMRX::ExecuteBind(int Bind)
{
    if(Bind >= 0 && Bind < static_cast<int>(m_vBinds.size()))
        Console()->ExecuteLine(m_vBinds[Bind].m_aCommand);
}

void CBindWheelMRX::ExecuteHoveredBind()
{
    if(m_SelectedBind >= 0)
        Console()->ExecuteLine(m_vBinds[m_SelectedBind].m_aCommand);
}

void CBindWheelMRX::OnRender()
{
    if(Client()->State() != IClient::STATE_ONLINE && Client()->State() != IClient::STATE_DEMOPLAYBACK)
        return;

    // if(m_Active)
    // {
    //     Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "bindwheel", "OnRender called - bind wheel active");
    // }


    static const auto QuadEaseInOut = [](float t) -> float {
        if(t == 0.0f)
            return 0.0f;
        if(t == 1.0f)
            return 1.0f;
        return (t < 0.5f) ? (2.0f * t * t) : (1.0f - std::pow(-2.0f * t + 2.0f, 2) / 2.0f);
    };

    static const float s_InnerOuterMouseBoundaryRadius = 110.0f;
    static const float s_OuterMouseLimitRadius = 170.0f;
    static const float s_OuterItemRadius = 140.0f;
    static const float s_OuterCircleRadius = 190.0f;
    static const float s_FontSize = 12.0f;
    static const float s_FontSizeSelected = 18.0f;

    const float AnimationTime = 0.2f; // Fixed animation time
    const float ItemAnimationTime = AnimationTime / 2.0f;

    if(AnimationTime != 0.0f)
    {
        for(float &Time : m_aAnimationTimeItems)
        {
            Time -= Client()->RenderFrameTime();
            if(Time <= 0.0f)
                Time = 0.0f;
        }
    }

    if(!m_Active)
    {
        if(m_WasActive)
        {
            // Reset mouse position when closing (if enabled in settings)
            // TODO: Check if reset mouse option is enabled
            if(m_SelectedBind != -1)
                ExecuteBind(m_SelectedBind);
        }
        m_WasActive = false;

        if(AnimationTime == 0.0f)
            return;

        m_AnimationTime -= Client()->RenderFrameTime() * 3.0f; // Close animation 3x faster
        if(m_AnimationTime <= 0.0f)
        {
            m_AnimationTime = 0.0f;
            return;
        }
    }
    else
    {
        m_AnimationTime += Client()->RenderFrameTime();
        if(m_AnimationTime > AnimationTime)
            m_AnimationTime = AnimationTime;
        m_WasActive = true;
    }

    const CUIRect Screen = *Ui()->Screen();

    // Clamp selector mouse position to wheel bounds
    if(length(m_SelectorMouse) > s_OuterMouseLimitRadius)
        m_SelectorMouse = normalize(m_SelectorMouse) * s_OuterMouseLimitRadius;

    int SegmentCount = m_vBinds.size();
    if(SegmentCount == 0)
    {
        m_SelectedBind = -1;
    }
    else
    {
        float SegmentAngle = 2.0f * pi / SegmentCount;
        float SelectedAngle = angle(m_SelectorMouse) + SegmentAngle / 2.0f;
        if(SelectedAngle < 0.0f)
            SelectedAngle += 2.0f * pi;
        if(length(m_SelectorMouse) > s_InnerOuterMouseBoundaryRadius)
            m_SelectedBind = (int)(SelectedAngle / (2.0f * pi) * SegmentCount);
        else
            m_SelectedBind = -1;
    }

    if(m_SelectedBind != -1)
    {
        m_aAnimationTimeItems[m_SelectedBind] += Client()->RenderFrameTime() * 2.0f;
        if(m_aAnimationTimeItems[m_SelectedBind] >= ItemAnimationTime)
            m_aAnimationTimeItems[m_SelectedBind] = ItemAnimationTime;
    }

    std::array<float, 2> aAnimationPhase;
    if(AnimationTime == 0.0f)
    {
        aAnimationPhase.fill(1.0f);
    }
    else
    {
        aAnimationPhase[0] = QuadEaseInOut(m_AnimationTime / AnimationTime);
        aAnimationPhase[1] = aAnimationPhase[0] * aAnimationPhase[0];
    }

    Ui()->MapScreen();

    Graphics()->BlendNormal();
    Graphics()->TextureClear();
    Graphics()->QuadsBegin();
    Graphics()->SetColor(0.0f, 0.0f, 0.0f, 0.3f * aAnimationPhase[0]);
    Graphics()->DrawCircle(Screen.w / 2.0f, Screen.h / 2.0f, s_OuterCircleRadius * aAnimationPhase[0], 64);
    Graphics()->QuadsEnd();

    Graphics()->WrapClamp();
    const float Theta = pi * 2.0f / m_vBinds.size();
    for(int i = 0; i < static_cast<int>(m_vBinds.size()); i++)
    {
        const CBind &Bind = m_vBinds[i];
        const float Angle = Theta * i;
        const vec2 Pos = direction(Angle) * s_OuterItemRadius * aAnimationPhase[1];
        const float Phase = ItemAnimationTime == 0.0f ? (i == m_SelectedBind ? 1.0f : 0.0f) : QuadEaseInOut(m_aAnimationTimeItems[i] / ItemAnimationTime);
        const float FontSize = (s_FontSize + Phase * (s_FontSizeSelected - s_FontSize)) * aAnimationPhase[1];
        const char *pName = Bind.m_aName;
        if(pName[0] == '\0')
        {
            pName = "Empty";
            TextRender()->TextColor(0.7f, 0.7f, 0.7f, aAnimationPhase[1]);
        }
        else
        {
            TextRender()->TextColor(1.0f, 1.0f, 1.0f, aAnimationPhase[1]);
        }
        float Width = TextRender()->TextWidth(FontSize, pName);
        TextRender()->Text(Screen.w / 2.0f + Pos.x - Width / 2.0f, Screen.h / 2.0f + Pos.y - FontSize / 2.0f, FontSize, pName);
    }
    TextRender()->TextColor(1.0f, 1.0f, 1.0f, 1.0f);
    Graphics()->WrapNormal();

    // Draw cursor like in TaterClient
    RenderTools()->RenderCursor(m_SelectorMouse + vec2(Screen.w, Screen.h) / 2.0f, 24.0f);
}

void CBindWheelMRX::LoadBindsFromConfig()
{
    // This will be called when the game starts to load saved binds
    // The actual loading happens through the console commands that were saved
    // in the config file, which will be executed automatically
}

void CBindWheelMRX::ConfigSaveCallback(IConfigManager *pConfigManager, void *pUserData)
{
    CBindWheelMRX *pThis = (CBindWheelMRX *)pUserData;

    for(CBindWheelMRX::CBind &Bind : pThis->m_vBinds)
    {
        char aBuf[MRX_BINDWHEEL_MAX_CMD * 2] = "";
        char *pEnd = aBuf + sizeof(aBuf);
        char *pDst;
        str_append(aBuf, "add_bindwheel \"");
        // Escape name
        pDst = aBuf + str_length(aBuf);
        str_escape(&pDst, Bind.m_aName, pEnd);
        str_append(aBuf, "\" \"");
        // Escape command
        pDst = aBuf + str_length(aBuf);
        str_escape(&pDst, Bind.m_aCommand, pEnd);
        str_append(aBuf, "\"");
        pConfigManager->WriteLine(aBuf);
    }
}