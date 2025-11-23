#ifndef GAME_CLIENT_COMPONENTS_CHEAT_FEATURES_ADVANCEDDEEPFLY_H
#define GAME_CLIENT_COMPONENTS_CHEAT_FEATURES_ADVANCEDDEEPFLY_H

#include <game/client/component.h>


class CAdvancedDeepFly : public CComponent
{
public:
    void OnRender() override;
    void HandleAdvancedDeepFly();
    void HandleAdvancedDeepFlyFire();
    bool InFov(vec2 TargetPos, float fov);
    virtual int Sizeof() const override { return sizeof(*this); }
    
    private:
    bool m_NeedReleaseFire = false;
};

#endif