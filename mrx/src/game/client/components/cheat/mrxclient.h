// (c) MRX-Client - Core MRX visuals (center lines, Same-X hint)
#ifndef GAME_CLIENT_COMPONENTS_CHEAT_MRXCLIENT_H
#define GAME_CLIENT_COMPONENTS_CHEAT_MRXCLIENT_H

#include <game/client/component.h>

class CMrxClient : public CComponent
{
public:
    virtual int Sizeof() const override { return sizeof(*this); }
    virtual void OnReset() override {}
    virtual void OnRender() override;

    void DummyThrow();
    void DummyFreezeFire();
    void HandleSmartDummyHook();

private:
    void RenderCenterLines();
    void RenderSameXHint();
};

#endif // GAME_CLIENT_COMPONENTS_CHEAT_MRXCLIENT_H