#pragma once
#include <game/client/component.h>
#include <engine/console.h>
#include <vector>

// MRX: Wheel Binds

enum
{
    MRX_BINDWHEEL_MAX_NAME = 64,
    MRX_BINDWHEEL_MAX_CMD = 1024,
    MRX_BINDWHEEL_MAX_BINDS = 64
};

class CBindWheelMRX : public CComponent
{
public:
    struct CBind
    {
        char m_aName[MRX_BINDWHEEL_MAX_NAME] = "EMPTY";
        char m_aCommand[MRX_BINDWHEEL_MAX_CMD] = "";
        bool operator==(const CBind &Other) const
        {
            return str_comp(m_aName, Other.m_aName) == 0 && str_comp(m_aCommand, Other.m_aCommand) == 0;
        }
    };

    std::vector<CBind> m_vBinds;
    
    float m_AnimationTime = 0.0f;
    float m_aAnimationTimeItems[MRX_BINDWHEEL_MAX_BINDS] = {0};
    bool m_Active = false;
    bool m_WasActive = false;
    int m_SelectedBind = -1;
    vec2 m_SelectorMouse = vec2(0.0f, 0.0f);

    CBindWheelMRX();
    int Sizeof() const override { return sizeof(*this); }
    void OnConsoleInit() override;
    void AddBind(const char *Name, const char *Command);
    void RemoveBind(const char *Name, const char *Command);
    void RemoveBind(int Index);
    void RemoveAllBinds();
    void OnRender() override;
    void OnReset() override;
    void OnRelease() override;
    bool OnCursorMove(float x, float y, IInput::ECursorType CursorType) override;
    bool OnInput(const IInput::CEvent &Event) override;
    void ExecuteBind(int Bind);
    void ExecuteHoveredBind();
    void LoadBindsFromConfig(); // Load saved binds from config
    bool IsActive() const { return m_Active; }
    
    // Console command handlers
    static void ConOpenBindwheel(IConsole::IResult *pResult, void *pUserData);
    static void ConBindwheelExecuteHover(IConsole::IResult *pResult, void *pUserData);
    static void ConAddBindwheel(IConsole::IResult *pResult, void *pUserData);
    static void ConRemoveBindwheel(IConsole::IResult *pResult, void *pUserData);
    static void ConRemoveAllBindwheelBinds(IConsole::IResult *pResult, void *pUserData);
    
    // Config save callback
    static void ConfigSaveCallback(IConfigManager *pConfigManager, void *pUserData);
};
