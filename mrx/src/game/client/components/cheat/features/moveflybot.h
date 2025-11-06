#ifndef GAME_CLIENT_COMPONENTS_CHEAT_FEATURES_MOVEFLYBOT_H
#define GAME_CLIENT_COMPONENTS_CHEAT_FEATURES_MOVEFLYBOT_H

#include <game/client/component.h>
#include <engine/console.h>
#include <base/vmath.h>
#include <optional>

class CMoveFlyBot : public CComponent
{
public:
virtual int Sizeof() const override { return sizeof(*this); }
    void MoveFly();
    void ResetDummyDirection();
    bool IsActive() const; // Метод для проверки активности MoveFlyBot

private:
    bool m_Active = false; // Переменная для хранения состояния активности
};

#endif