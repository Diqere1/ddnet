#ifndef GAME_CLIENT_COMPONENTS_CHEAT_FEATURES_AUTOALED_H
#define GAME_CLIENT_COMPONENTS_CHEAT_FEATURES_AUTOALED_H

#include <game/client/component.h>

class CAutoAled : public CComponent
{
public:
    void OnRender() override;
    void HandleAutoAled();
    virtual int Sizeof() const override { return sizeof(*this); }
    bool FindClosestTarget(vec2 &TargetPos);

private:
    bool IsValidTarget(int TargetID) const;
    void ResetInput();
    bool HasFreezeTileBetween(const vec2& from, const vec2& to) const; // Новый метод
    bool m_LastHitState = false;
};

#endif