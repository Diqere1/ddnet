#ifndef GAME_CLIENT_COMPONENTS_CHEAT_FEATURES_AIMHOOKDUMMY_H 
#define GAME_CLIENT_COMPONENTS_CHEAT_FEATURES_AIMHOOKDUMMY_H

#include <game/client/component.h>

class CAimHookDummy : public CComponent
{
public:
    bool HandleHook(); // Изменен возвращаемый тип на bool
    virtual int Sizeof() const override { return sizeof(*this); }

private:
    bool FindTarget(vec2 &TargetPos);
    void ResetInput();
    bool m_LastHookState = false;
};

#endif