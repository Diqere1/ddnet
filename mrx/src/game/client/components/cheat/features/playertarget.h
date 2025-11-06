#ifndef GAME_CLIENT_COMPONENTS_CHEAT_FEATURES_PLAYERTARGET_H
#define GAME_CLIENT_COMPONENTS_CHEAT_FEATURES_PLAYERTARGET_H

#include <game/client/component.h>
#include <engine/console.h>
#include <base/vmath.h>
#include <optional>

class CPlayerTarget : public CComponent 
{
public:
    // Размер наследуемого объекта для CComponent
    virtual int Sizeof() const override { return sizeof(*this); }
    bool IsActive() const { return m_Active; }
    // Инициализация консольных команд
    virtual void OnConsoleInit() override;
    
    // Функция отрисовки, вызывается каждый кадр
    virtual void OnRender() override;

    // Структура для хранения информации о цели
    struct STarget {
        int ClientId; // ID игрока-цели
        vec2 Position; // Позиция игрока
        float Distance; // Дистанция до игрока
        
        STarget(int id, const vec2& pos, float dist) :
            ClientId(id), Position(pos), Distance(dist) {}
    };
    std::optional<STarget> GetTarget() const; // Метод для получения текущей цели
std::optional<STarget> FindNearestTarget() const;
private:
    static void ConToggle(IConsole::IResult *pResult, void *pUserData);
   
    
    bool m_Active = false; // Флаг активности компонента

    int m_CurrentTargetID = -1; // Добавляем ID текущей цели
    std::optional<STarget> FindTargetByMousePos() const; // Новый метод для поиска цели по курсору
    
    
    void RenderTriangle(const vec2& Position) const;
    bool IsValidTarget(int ClientId) const;
};

#endif