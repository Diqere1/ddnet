#ifndef GAME_CLIENT_COMPONENTS_CHEAT_FEATURES_PSEUDOFIRE_H
#define GAME_CLIENT_COMPONENTS_CHEAT_FEATURES_PSEUDOFIRE_H

#include <game/client/component.h>

class CPseudofire : public CComponent
{
public:
    void OnRender() override;
    void RunMRXDuoPseudo();

     virtual int Sizeof() const override { return sizeof(*this); }
private:
    int64_t m_LastHammerTime = 0;
    bool FindTarget(vec2 &TargetPos);
    int m_CurrentTargetId = -1;
    vec2 m_ArrowRenderPos = vec2(0, 0); // Интерполированная позиция стрелочки
    void RenderArrow(const vec2 &Pos) const;
    bool IsValidTarget(int ClientId) const;
};

#endif