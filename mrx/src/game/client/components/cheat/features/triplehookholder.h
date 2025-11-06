#pragma once

#include <game/client/component.h>
#include <optional>

class CTripleHookHolder : public CComponent
{
public:
    void HandleTripleHookHolder();
    void HandleAutoTripleFly();
    void HandleAutoTripleFlyDefault();
    void HandleAutoTripleFlyThrow();
    void HandleAutoTripleFlyThrowBoth();
    bool IsTripleThrowBothActive() const { return m_AutoTripleFlyThrowBoth.Active; }
    bool IsTripleThrowBothTargetInStrikeRange() const;
    virtual int Sizeof() const override { return sizeof(*this); }
    void OnRender() override;

private:
    struct TargetInfo
    {
        int ClientId = -1;
        float FixedY = 0.0f;
        vec2 Position;
    };

    struct AutoTripleFlyInfo
    {
        int ClientId = -1;
        float DesiredDistance = 0.0f;
        bool Active = false;
        bool HookReleasedBySystem = false; // Флаг, что система сама отпустила хук
    };

    struct AutoTripleFlyThrowInfo
    {
        int ClientId = -1;
        bool Active = false;
        bool HasThrown = false;
    };

    struct AutoTripleFlyThrowBothInfo
    {
        int ClientId = -1;
        bool Active = false;
        bool HasThrown = false;
    };

    std::optional<TargetInfo> m_Target;
    bool m_Active = false;

    AutoTripleFlyInfo m_AutoTripleFly = {};
    AutoTripleFlyThrowInfo m_AutoTripleFlyThrow = {};
    AutoTripleFlyThrowBothInfo m_AutoTripleFlyThrowBoth = {};
    
    bool m_NeedReleaseFire = false;
    
    // Глобальные кулдауны для функций
    int m_GlobalCooldownThrow = 0;
    int m_GlobalCooldownThrowBoth = 0;

    void Reset();
    std::optional<TargetInfo> FindTargetBelowDummy() const;
    int FindTargetNearCursor() const;
    int FindTargetNearMainCursor() const;
};





