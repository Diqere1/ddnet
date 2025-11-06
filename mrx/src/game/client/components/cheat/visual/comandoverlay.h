// (c) MRX-Client - Command Overlay component
#ifndef GAME_CLIENT_COMPONENTS_CHEAT_VISUAL_COMMANDOVERLAY_H
#define GAME_CLIENT_COMPONENTS_CHEAT_VISUAL_COMMANDOVERLAY_H

#include <game/client/component.h>

class CCommandOverlay : public CComponent
{
public:
    virtual int Sizeof() const override { return sizeof(*this); }
    virtual void OnReset() override {}
    virtual void OnRender() override;

private:
    void RenderTextSlot(const char *pText, int XPercent, int YPercent, int Slot) const;
};

#endif // GAME_CLIENT_COMPONENTS_CHEAT_VISUAL_COMMANDOVERLAY_H


