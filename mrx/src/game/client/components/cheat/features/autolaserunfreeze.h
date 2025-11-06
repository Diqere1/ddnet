#pragma once

#include <game/client/component.h>

class CAutolaserUnfreeze : public CComponent
{
public:
    void OnRender() override;
    void HandleAutoLaserUnfreeze();
    virtual int Sizeof() const override { return sizeof(*this); }

private:
    int m_LastWeapon = 0;
    bool m_WasFired = false;
    bool m_WeaponSwitched = false;
};