#ifndef GAME_CLIENT_COMPONENTS_CHEAT_VISUALS_ARRAYLIST_H
#define GAME_CLIENT_COMPONENTS_CHEAT_VISUALS_ARRAYLIST_H

#include <game/client/component.h>
#include <base/color.h>
#include <vector>

class CArrayList : public CComponent
{
public:
    virtual int Sizeof() const override { return sizeof(*this); }
    virtual void OnReset() override;
    virtual void OnRender() override;

    struct ArrayListItem {
        const char* name;
        ColorRGBA color;
        
        bool operator<(const ArrayListItem& other) const {
            return str_comp(name, other.name) < 0;
        }
    };

    // Идентификаторы фич строго соответствуют порядку в FEATURES в arraylist.cpp
    enum class EFeature : int
    {
        MoveFlyBot = 0,
        BalanceBot,
        PseudoAled,
        AutoAled,
        AvoidFreeze,
        AutoPseudo,
        PixelWalk,
        AdvancedDeepFly,
        AutoHookDummy,
        SlowWalk,
        TripleHookHolder,
        AutoTripleFly,
        DummyHookCursor,
        Aimbot,
        AutoHookFly,
        DummyThrow,
        DuoPseudo,
        ReverseCopy,
        COUNT
    };

    // Управление видимостью элементов ArrayList (без конфигов)
    bool GetFeatureVisible(EFeature Id) const;
    void SetFeatureVisible(EFeature Id, bool Visible);
    int FeatureCount() const { return static_cast<int>(EFeature::COUNT); }
    bool m_IsPreview = false;

private:
    std::vector<ArrayListItem> GetEnabledFeatures() const;
    void RenderFeature(float X, float Y, const char* Name, const ColorRGBA& Color, bool AlignRight) const;

    float m_X = 0.0f;
    float m_Padding = 6.0f;
    float m_EntryHeight = 22.0f;
    float m_TextSize = 15.0f;

    // Флаги видимости для ArrayList (живут в памяти, не сохраняются)
    bool m_VisInitialized = false;
    bool m_aVisible[(int)EFeature::COUNT] = {false};
};

#endif
