#ifndef GAME_CLIENT_COMPONENTS_CHEAT_FEATURES_DUMMYALED_H
#define GAME_CLIENT_COMPONENTS_CHEAT_FEATURES_DUMMYALED_H

#include <game/client/component.h>

class CDummyAled : public CComponent
{
public:
    void OnRender() override;
    void HandleDummyAled();
    virtual int Sizeof() const override { return sizeof(*this); }
    bool IsControllingAim() const { return m_IsControllingAim; }

private:
    bool FindTarget(vec2 &TargetPos);
    void ResetDummyInput(); // Новый метод для сброса ввода
     bool m_LastHitState = false; // Добавляем флаг для отслеживания предыдущего состояния удара
     bool HasFreezeTileBetween(const vec2& from, const vec2& to) const; // Новый метод
     bool m_IsControllingAim = false; // Флаг: данный компонент контролирует прицел
};

#endif