#ifndef GAME_CLIENT_COMPONENTS_CHEAT_FEATURES_PIXELWALK_H
#define GAME_CLIENT_COMPONENTS_CHEAT_FEATURES_PIXELWALK_H

#include <game/client/component.h>

class CPixelWalk : public CComponent
{
public:
    void OnRender() override;
    void HandlePixelWalk();
    virtual int Sizeof() const override { return sizeof(*this); }
    
};

#endif