#ifndef GAME_CLIENT_COMPONENTS_CHEAT_VISUAL_PREDICTPOSITIONLINE_H
#define GAME_CLIENT_COMPONENTS_CHEAT_VISUAL_PREDICTPOSITIONLINE_H

#include <game/client/component.h>

class CPredictPositionLine : public CComponent
{
public:
    virtual int Sizeof() const override { return sizeof(*this); }
    virtual void OnReset() override {}
    virtual void OnRender() override;
};

#endif // GAME_CLIENT_COMPONENTS_CHEAT_VISUAL_PREDICTPOSITIONLINE_H