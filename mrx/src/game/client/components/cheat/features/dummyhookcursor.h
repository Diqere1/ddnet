#ifndef GAME_CLIENT_COMPONENTS_CHEAT_FEATURES_DUMMYHOOKCURSOR_H
#define GAME_CLIENT_COMPONENTS_CHEAT_FEATURES_DUMMYHOOKCURSOR_H

#include <game/client/component.h>

class CDummyHookCursor : public CComponent
{
public:
    void OnRender() override;
    void HandleDummyHookCursor();
    virtual int Sizeof() const override { return sizeof(*this); }

private:
    bool m_LastHookState = false;
    vec2 m_LastHookPos{0,0};
};

#endif