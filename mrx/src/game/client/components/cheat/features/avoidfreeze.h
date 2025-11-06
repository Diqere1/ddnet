#ifndef GAME_CLIENT_COMPONENTS_CHEAT_FEATURES_AVOIDFREEZE_H
#define GAME_CLIENT_COMPONENTS_CHEAT_FEATURES_AVOIDFREEZE_H

#include <game/client/component.h>
#include <base/vmath.h>

class CAvoidFreeze : public CComponent
{
public:
    void HandleAvoidFreeze();
    virtual int Sizeof() const override { return sizeof(*this); }

private:
    // no internal state kept currently
};

#endif