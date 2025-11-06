// MRX UI helpers implementations moved out of menus_settings_mrx.cpp
// Keep structure consistent with project

#include "menus.h"
#include <engine/textrender.h>
#include <game/client/components/cheat/visual/arraylist.h>
#include <engine/input.h>
#include <game/localization.h>
#include <game/client/gameclient.h>
#include <game/client/components/binds.h>
#include <algorithm>
#include <cctype>

// ===== MRX UI helpers implementation =====

// ===== Helper macros/functions (key binds and flags) =====
#define FIND_KEY_BINDING(key_info, command_name) \
    key_info = CKeyInfo{command_name, command_name, 0, 0}; \
    for(int Mod = 0; Mod < CBinds::MODIFIER_COMBINATION_COUNT; Mod++) \
    { \
        for(int KeyId = 0; KeyId < KEY_LAST; KeyId++) \
        { \
            const char *pBind = GameClient()->m_Binds.Get(KeyId, Mod); \
            if(!pBind[0]) \
                continue; \
            if(str_comp(pBind, key_info.m_pCommand) == 0) \
            { \
                key_info.m_KeyId = KeyId; \
                key_info.m_ModifierCombination = Mod; \
                break; \
            } \
        } \
    }

    void CMenus::DoWeaponPreview(const CUIRect *pRect, int WeaponID)
{
    if(!pRect) return;

    const float Width = pRect->w;
    const float Height = pRect->h;
    const float x = pRect->x + Width * 0.5f;
    const float y = pRect->y + Height * 0.5f;

    if(WeaponID == -1)
    {
        // Цепь: 3 сегмента по X
        Graphics()->TextureSet(GameClient()->m_GameSkin.m_SpriteHookChain);
        Graphics()->QuadsBegin();
        Graphics()->QuadsSetSubset(0, 0, 1, 1);
        Graphics()->SetColor(1,1,1,1);

        const float ChainWidth = Height * 0.35f;
        const float ChainLength = Width * 0.65f;
        const int NumSegments = 3;
        const float SegLen = ChainLength / NumSegments;
        const float StartX = x - ChainLength * 0.5f;
        const float ChainY = y - ChainWidth * 0.5f;

        for(int i = 0; i < NumSegments; i++)
        {
            float SegX = StartX + i * SegLen;
            IGraphics::CQuadItem QuadSeg(SegX, ChainY, SegLen, ChainWidth);
            Graphics()->QuadsDrawTL(&QuadSeg, 1);
        }
        Graphics()->QuadsEnd();

        // Голова
        Graphics()->TextureSet(GameClient()->m_GameSkin.m_SpriteHookHead);
        Graphics()->QuadsBegin();
        Graphics()->QuadsSetSubset(0, 0, 1, 1);
        Graphics()->SetColor(1,1,1,1);

        const float HeadSize = Height * 0.8f;
        IGraphics::CQuadItem QuadHead(x + ChainLength * 0.5f - HeadSize * 0.5f, y - HeadSize * 0.5f, HeadSize, HeadSize);
        Graphics()->QuadsDrawTL(&QuadHead, 1);
        Graphics()->QuadsEnd();
        return;
    }

    // Оружие: спрайт просто заполняет прямоугольник
    IGraphics::CTextureHandle WeaponTexture = {};
    switch(WeaponID)
    {
    case WEAPON_HAMMER:   WeaponTexture = GameClient()->m_GameSkin.m_SpriteWeaponHammer; break;
    case WEAPON_GUN:      WeaponTexture = GameClient()->m_GameSkin.m_SpriteWeaponGun; break;
    case WEAPON_SHOTGUN:  WeaponTexture = GameClient()->m_GameSkin.m_SpriteWeaponShotgun; break;
    case WEAPON_GRENADE:  WeaponTexture = GameClient()->m_GameSkin.m_SpriteWeaponGrenade; break;
    case WEAPON_LASER:    WeaponTexture = GameClient()->m_GameSkin.m_SpriteWeaponLaser; break;
    case WEAPON_NINJA:    WeaponTexture = GameClient()->m_GameSkin.m_SpriteWeaponNinja; break;
    default: return;
    }

    Graphics()->TextureSet(WeaponTexture);
    Graphics()->QuadsBegin();
    Graphics()->QuadsSetSubset(0, 0, 1, 1);
    Graphics()->SetColor(1,1,1,1);
    IGraphics::CQuadItem QuadItem(pRect->x, pRect->y, pRect->w, pRect->h);
    Graphics()->QuadsDrawTL(&QuadItem, 1);
    Graphics()->QuadsEnd();
}

void CMenus::DoCircleToggleInRect(CUIRect &Rect, const char *pLabel, int &Value)
{
    // Стабильные контейнеры и анимация по адресу Value
    static std::vector<const void*> s_Keys;
    static std::vector<CButtonContainer> s_Btns;
    static std::vector<float> s_Anim; // 0.0(off) -> 1.0(on)

    auto GetIndexForKey = [&](const void *pKey) -> int {
        for(size_t i = 0; i < s_Keys.size(); ++i)
            if(s_Keys[i] == pKey)
                return (int)i;
        s_Keys.push_back(pKey);
        s_Btns.emplace_back();
        s_Anim.push_back(Value ? 1.0f : 0.0f);
        return (int)s_Keys.size() - 1;
    };

    const int Idx = GetIndexForKey(&Value);

    // Клик по всей области Rect
    if(DoButton_Menu(&s_Btns[Idx], "", 0, &Rect, BUTTONFLAG_LEFT, nullptr, 0, 0.0f, 0.0f, ColorRGBA(0,0,0,0.0f)))
        Value ^= 1;

    // Обновление анимации к целевому состоянию
    const float Target = Value ? 1.0f : 0.0f;
    s_Anim[Idx] = s_Anim[Idx] + (Target - s_Anim[Idx]) * 0.2f;

    // Разделим прямоугольник под текст и иконку
    CUIRect LabelRect, IconRect;
    float IconBox = std::min(Rect.h, 18.0f);
    Rect.VSplitRight(IconBox, &LabelRect, &IconRect);

    // Текст метки
    Ui()->DoLabel(&LabelRect, Localize(pLabel), 12.0f, TEXTALIGN_MR);

    // Иконка
    ITextRender *pTR = Ui()->TextRender();
    unsigned PrevFlags = pTR->GetRenderFlags();
    pTR->SetFontPreset(EFontPreset::ICON_FONT);

    const float IconSize = minimum(IconRect.h * 0.9f, 14.0f);
    const float IconX = IconRect.x + (IconRect.w - IconSize) * 0.5f;
    const float IconY = IconRect.y + (IconRect.h - IconSize) * 0.5f;

    // OFF слой — пустой круг
    pTR->TextColor(0.7f, 0.7f, 0.7f, 0.5f - s_Anim[Idx]);
    pTR->Text(IconX, IconY, IconSize, FontIcons::FONT_ICON_CIRCLE);

    // ON слой — круг с точкой
    pTR->TextColor(1.0f, 1.0f, 1.0f, s_Anim[Idx]);
    pTR->Text(IconX, IconY, IconSize, FontIcons::FONT_ICON_CIRCLE_DOT);

    pTR->SetFontPreset(EFontPreset::DEFAULT_FONT);
    pTR->SetRenderFlags(PrevFlags);
    pTR->TextColor(1,1,1,1);
}

void CMenus::DoCircleToggleRow(CUIRect &Column, float LineSize, const char *pLabel, int &Value)
{
    CUIRect Row;
    Column.HSplitTop(LineSize, &Row, &Column);

    // Левая метка и правая область под иконку (адаптивно)
    CUIRect LabelRect, IconRect;
    float RightAreaHi = std::max(LineSize, Row.w * 0.30f);
    const float RightArea = std::clamp(LineSize * 1.5f, LineSize, RightAreaHi);
    Row.VSplitRight(RightArea, &LabelRect, &IconRect);

    // Текст метки
    Ui()->DoLabel(&LabelRect, Localize(pLabel), 12.0f, TEXTALIGN_ML);

    // Стабильные контейнеры и анимация по адресу Value
    static std::vector<const void*> s_Keys;
    static std::vector<CButtonContainer> s_Btns;
    static std::vector<float> s_Anim; // 0.0(off) -> 1.0(on)

    auto GetIndexForKey = [&](const void *pKey) -> int {
        for(size_t i = 0; i < s_Keys.size(); ++i)
            if(s_Keys[i] == pKey)
                return (int)i;
        s_Keys.push_back(pKey);
        s_Btns.emplace_back();
        s_Anim.push_back(Value ? 1.0f : 0.0f);
        return (int)s_Keys.size() - 1;
    };

    const int Idx = GetIndexForKey(&Value);

    // Клик по всей строке
    if(DoButton_Menu(&s_Btns[Idx], "", 0, &Row, BUTTONFLAG_LEFT, nullptr, 0, 0.0f, 0.0f, ColorRGBA(0,0,0,0.0f)))
        Value ^= 1;

    // Обновление анимации к целевому состоянию
    const float Target = Value ? 1.0f : 0.0f;
    s_Anim[Idx] = s_Anim[Idx] + (Target - s_Anim[Idx]) * 0.2f;

    // Рендер правой иконки: кросс-фейд между пустым кругом и кругом с точкой
    ITextRender *pTR = Ui()->TextRender();
    unsigned PrevFlags = pTR->GetRenderFlags();
    pTR->SetFontPreset(EFontPreset::ICON_FONT);

    const float IconSize = minimum(IconRect.h * 0.9f, 14.0f);
    const float IconX = IconRect.x + (IconRect.w - IconSize) * 0.5f;
    const float IconY = IconRect.y + (IconRect.h - IconSize) * 0.5f;

    // OFF слой — пустой круг
    pTR->TextColor(0.7f, 0.7f, 0.7f, 0.5f - s_Anim[Idx]);
    pTR->Text(IconX, IconY, IconSize, FontIcons::FONT_ICON_CIRCLE);

    // ON слой — круг с точкой
    pTR->TextColor(1.0f, 1.0f, 1.0f, s_Anim[Idx]);
    pTR->Text(IconX, IconY, IconSize, FontIcons::FONT_ICON_CIRCLE_DOT);

    pTR->SetFontPreset(EFontPreset::DEFAULT_FONT);
    pTR->SetRenderFlags(PrevFlags);
    pTR->TextColor(1,1,1,1);
}

// Капсульный дропдаун: рендер как у DoKeyBindRow (капсула), логика как у Ui()->DoDropDown
int CMenus::DoDropDownPill(CUIRect *pRect, int CurSelection, const char **pStrs, int Num, CUi::SDropDownState &State)
{
    if(!State.m_Init)
    {
        State.m_UiElement.Init(Ui(), -1);
        State.m_Init = true;
    }

    // Текст текущего значения
    const char *pLabel = (CurSelection > -1 && CurSelection < Num) ? pStrs[CurSelection] : "";

    // Кнопочная логика без стандартного рендера
    const int Clicked = Ui()->DoButtonLogic(&State.m_ButtonContainer, 0, pRect, BUTTONFLAG_LEFT);

    // Рисуем капсулу-фон
    CUIRect Pill = *pRect;
    Pill.Margin(2.0f, &Pill);
    const float Rounding = Pill.h * 0.5f;
    Pill.Draw(ColorRGBA(1.0f, 1.0f, 1.0f, 0.22f), IGraphics::CORNER_ALL, Rounding);

    // Текст по центру и стрелка справа
    CUIRect TextRect = Pill;
    Ui()->DoLabel(&TextRect, pLabel, 12.0f, TEXTALIGN_MC);

    // Стрелка вниз справа (иконка)
    {
        ITextRender *pTR = Ui()->TextRender();
        unsigned PrevFlags = pTR->GetRenderFlags();
        pTR->SetFontPreset(EFontPreset::ICON_FONT);
        pTR->SetRenderFlags(ETextRenderFlags::TEXT_RENDER_FLAG_NO_PIXEL_ALIGNMENT | ETextRenderFlags::TEXT_RENDER_FLAG_NO_OVERSIZE);
        pTR->TextColor(1.0f, 1.0f, 1.0f, 0.5f);
        const float IconSize = minimum(Pill.h * 0.9f, 14.0f);
        const float IconX = Pill.x + Pill.w - IconSize - 4.0f;
        const float IconY = Pill.y + (Pill.h - IconSize) * 0.5f;
        pTR->Text(IconX, IconY, IconSize, FontIcons::FONT_ICON_CHEVRON_DOWN);
        pTR->SetFontPreset(EFontPreset::DEFAULT_FONT);
        pTR->SetRenderFlags(PrevFlags);
        pTR->TextColor(1,1,1,1);
    }

    if(Clicked)
    {
        State.m_SelectionPopupContext.Reset();
        State.m_SelectionPopupContext.m_Props.m_BorderColor = ColorRGBA(0.7f, 0.7f, 0.7f, 0.9f);
        State.m_SelectionPopupContext.m_Props.m_BackgroundColor = ColorRGBA(0.0f, 0.0f, 0.0f, 0.25f);
        for(int i = 0; i < Num; ++i)
            State.m_SelectionPopupContext.m_vEntries.emplace_back(pStrs[i]);
        State.m_SelectionPopupContext.m_EntryHeight = pRect->h;
        State.m_SelectionPopupContext.m_EntryPadding = pRect->h >= 20.0f ? 2.0f : 1.0f;
        State.m_SelectionPopupContext.m_FontSize = (State.m_SelectionPopupContext.m_EntryHeight - 2 * State.m_SelectionPopupContext.m_EntryPadding) * CUi::ms_FontmodHeight;
        State.m_SelectionPopupContext.m_Width = pRect->w;
        State.m_SelectionPopupContext.m_AlignmentHeight = pRect->h;
        State.m_SelectionPopupContext.m_TransparentButtons = true;
        Ui()->ShowPopupSelection(pRect->x, pRect->y, &State.m_SelectionPopupContext);
    }

    if(State.m_SelectionPopupContext.m_SelectionIndex >= 0)
    {
        const int NewSelection = State.m_SelectionPopupContext.m_SelectionIndex;
        State.m_SelectionPopupContext.Reset();
        return NewSelection;
    }

    return CurSelection;
}

// Адаптивный ряд: Лейбл + выпадающий список (капсула)
// Выравнивает ширину лейбла по измеренной ширине текста с безопасными пределами,
// затем рисует DoDropDownPill на оставшейся области. Возвращает новый выбор.
int CMenus::DoLabeledDropDownRow(CUIRect &Column, float LineSize, const char *pLabel,
    int CurSelection, const char **pStrs, int Num, CUi::SDropDownState &State)
{
    CUIRect Row, LabelRect, DropRect;
    Column.HSplitTop(LineSize, &Row, &Column);

    // Измеряем текст для более точного лейбла
    ITextRender *pTR = Ui()->TextRender();
    const float FontSize = 12.0f;
    float TextW = pTR->TextWidth(FontSize, Localize(pLabel), -1);
    // Целевой размер лейбла с отступом
    float Target = TextW + 24.0f;
    // Безопасные пределы: минимум/максимум части строки
    float MinLabel = 80.0f;
    float MaxLabel = std::max(MinLabel, Row.w * 0.50f);
    float LabelW = std::clamp(Target, MinLabel, MaxLabel);

    // Гарантируем, что под дропдаун остаётся место
    const float MinDrop = 120.0f;
    if(Row.w - LabelW < MinDrop)
        LabelW = std::max(MinLabel, Row.w - MinDrop);

    Row.VSplitLeft(LabelW, &LabelRect, &DropRect);
    Ui()->DoLabel(&LabelRect, Localize(pLabel), FontSize, TEXTALIGN_ML);

    return DoDropDownPill(&DropRect, CurSelection, pStrs, Num, State);
}
void CMenus::DoCheckBoxRow(CUIRect &Column, float LineSize, const char *pLabel, int &Value)
{
    CUIRect Row;
    Column.HSplitTop(LineSize, &Row, &Column);

    // Разделим строку на текст и область под иконку тоггла
    CUIRect LabelRect, ToggleRect;
    const float ToggleArea = LineSize * 1.5f;
    Row.VSplitRight(ToggleArea, &LabelRect, &ToggleRect);

    // Текст метки
    Ui()->DoLabel(&LabelRect, Localize(pLabel), 12.0f, TEXTALIGN_ML);

    // Стабильные контейнеры и анимация по адресу Value
    static std::vector<const void*> s_Keys;
    static std::vector<CButtonContainer> s_Btns;
    static std::vector<float> s_Anim; // 0.0(off) -> 1.0(on)

    auto GetIndexForKey = [&](const void *pKey) -> int {
        for(size_t i = 0; i < s_Keys.size(); ++i)
            if(s_Keys[i] == pKey)
                return (int)i;
        s_Keys.push_back(pKey);
        s_Btns.emplace_back();
        s_Anim.push_back(Value ? 1.0f : 0.0f);
        return (int)s_Keys.size() - 1;
    };

    const int Idx = GetIndexForKey(&Value);

    // Обработка клика по всей строке
    if(DoButton_Menu(&s_Btns[Idx], "", 0, &Row, BUTTONFLAG_LEFT, nullptr, 0, 0.0f, 0.0f, ColorRGBA(0,0,0,0.0f)))
        Value ^= 1;

    // Обновление анимации к целевому состоянию
    const float Target = Value ? 1.0f : 0.0f;
    // Простое сглаживание (без дельта-тайма), достаточно плавно
    s_Anim[Idx] = s_Anim[Idx] + (Target - s_Anim[Idx]) * 0.2f;

    // Рендер иконок тоггла (кросс-фейд)
    ITextRender *pTR = Ui()->TextRender();
    unsigned PrevFlags = pTR->GetRenderFlags();
    pTR->SetFontPreset(EFontPreset::ICON_FONT);

    const float IconSize = minimum(ToggleRect.h * 2.9f, 20.0f);
    const float IconX = ToggleRect.x + (ToggleRect.w - IconSize) * 0.5f;
    const float IconY = ToggleRect.y + (ToggleRect.h - IconSize) * 0.5f;

    // OFF слой
    pTR->TextColor(0.7f, 0.7f, 0.7f, 0.5f - s_Anim[Idx]);
    pTR->Text(IconX, IconY, IconSize, FontIcons::FONT_ICON_TOGGLE_OFF);

    // ON слой
    pTR->TextColor(1.0f, 1.0f, 1.0f, s_Anim[Idx]);
    pTR->Text(IconX, IconY, IconSize, FontIcons::FONT_ICON_TOGGLE_ON);

    // Восстановление состояния текст-рендера
    pTR->SetFontPreset(EFontPreset::DEFAULT_FONT);
    pTR->SetRenderFlags(PrevFlags);
    pTR->TextColor(1,1,1,1);
}

void CMenus::DoGearToggleRow(CUIRect &Column, float LineSize, const char *pLabel, int &Value)
{
    CUIRect Row;
    Column.HSplitTop(LineSize, &Row, &Column);

    // Левая метка и правая область под иконку
    CUIRect LabelRect, IconRect;
    const float RightArea = LineSize * 1.5f;
    Row.VSplitRight(RightArea, &LabelRect, &IconRect);

    // Текст метки
    Ui()->DoLabel(&LabelRect, Localize(pLabel), 12.0f, TEXTALIGN_ML);

    // Стабильные контейнеры и анимация по адресу Value
    static std::vector<const void*> s_Keys;
    static std::vector<CButtonContainer> s_Btns;
    static std::vector<float> s_Anim; // 0.0(off) -> 1.0(on)

    auto GetIndexForKey = [&](const void *pKey) -> int {
        for(size_t i = 0; i < s_Keys.size(); ++i)
            if(s_Keys[i] == pKey)
                return (int)i;
        s_Keys.push_back(pKey);
        s_Btns.emplace_back();
        s_Anim.push_back(Value ? 1.0f : 0.0f);
        return (int)s_Keys.size() - 1;
    };

    const int Idx = GetIndexForKey(&Value);

    // Клик по всей строке
    if(DoButton_Menu(&s_Btns[Idx], "", 0, &Row, BUTTONFLAG_LEFT, nullptr, 0, 0.0f, 0.0f, ColorRGBA(0,0,0,0.0f)))
        Value ^= 1;

    // Обновление анимации к целевому состоянию
    const float Target = Value ? 1.0f : 0.0f;
    s_Anim[Idx] = s_Anim[Idx] + (Target - s_Anim[Idx]) * 0.2f;

    // Рендер правой иконки-шестерёнки
    ITextRender *pTR = Ui()->TextRender();
    unsigned PrevFlags = pTR->GetRenderFlags();
    pTR->SetFontPreset(EFontPreset::ICON_FONT);

    const float IconSize = minimum(IconRect.h * 0.9f, 14.0f);
    const float IconX = IconRect.x + (IconRect.w - IconSize) * 0.5f;
    const float IconY = IconRect.y + (IconRect.h - IconSize) * 0.5f;

    // Альфа зависит от анимации (состояния)
    const float Alpha = 0.22f + 0.78f * s_Anim[Idx];
    pTR->TextColor(1.0f, 1.0f, 1.0f, Alpha);
    pTR->Text(IconX, IconY, IconSize, FontIcons::FONT_ICON_GEAR);

    pTR->SetFontPreset(EFontPreset::DEFAULT_FONT);
    pTR->SetRenderFlags(PrevFlags);
    pTR->TextColor(1,1,1,1);
}

void CMenus::DoKeyBindRow(CUIRect &Column, float LineSize, const char *pLabel, const char *pCommand,
    CButtonContainer &ButtonContainer, int &Key, int &ModifierCombination)
{
    int CurKey = 0;
    int CurMod = 0;
    for(int Mod = 0; Mod < CBinds::MODIFIER_COMBINATION_COUNT; Mod++)
    {
        for(int KeyId = 0; KeyId < KEY_LAST; KeyId++)
        {
            const char *pBind = GameClient()->m_Binds.Get(KeyId, Mod);
            if(pBind[0] && str_comp(pBind, pCommand) == 0)
            {
                CurKey = KeyId;
                CurMod = Mod;
                break;
            }
        }
    }

    Key = CurKey;
    ModifierCombination = CurMod;

    CUIRect KeyButton, KeyLabel;
    Column.HSplitTop(LineSize, &KeyButton, &Column);
    KeyButton.VSplitMid(&KeyLabel, &KeyButton);

    char aBuf[128];
    str_format(aBuf, sizeof(aBuf), "%s:", Localize(pLabel));
    Ui()->DoLabel(&KeyLabel, aBuf, 12.0f, TEXTALIGN_ML);

    // --- Логика чтения клавиши (без фонового рендера) ---
    int OldKey = Key;
    int OldMod = ModifierCombination;
    int NewMod = OldMod;

    const int ButtonResult = Ui()->DoButtonLogic(&ButtonContainer, 0, &KeyButton, BUTTONFLAG_LEFT | BUTTONFLAG_RIGHT);
    if(ButtonResult == 1)
    {
        m_Binder.m_pKeyReaderId = &ButtonContainer;
        m_Binder.m_TakeKey = true;
        m_Binder.m_GotKey = false;
    }
    else if(ButtonResult == 2)
    {
        if(OldKey != 0)
            GameClient()->m_Binds.Bind(OldKey, "", false, OldMod);
        Key = 0;
        ModifierCombination = CBinds::MODIFIER_NONE;
    }

    if(m_Binder.m_pKeyReaderId == &ButtonContainer && m_Binder.m_GotKey)
    {
        if(m_Binder.m_Key.m_Key != KEY_ESCAPE)
        {
            Key = m_Binder.m_Key.m_Key;
            ModifierCombination = m_Binder.m_ModifierCombination;
            if(OldKey != 0)
                GameClient()->m_Binds.Bind(OldKey, "", false, OldMod);
            if(Key != 0)
                GameClient()->m_Binds.Bind(Key, pCommand, false, ModifierCombination);
        }
        m_Binder.m_pKeyReaderId = nullptr;
        m_Binder.m_GotKey = false;
        Ui()->SetActiveItem(nullptr);
    }

    // --- Текст внутри кнопки ---
    char aKeyText[64];
    if(m_Binder.m_pKeyReaderId == &ButtonContainer && m_Binder.m_TakeKey)
        str_copy(aKeyText, Localize("Press a key…"));
    else if(Key == 0)
        aKeyText[0] = '\0';
    else
    {
        char aModifiers[128];
        CBinds::GetKeyBindModifiersName(ModifierCombination, aModifiers, sizeof(aModifiers));
        str_format(aKeyText, sizeof(aKeyText), "%s%s", aModifiers, Input()->KeyName(Key));
    }

    // --- Кастомный рендер: фон-«капсула» (очень скруглённые углы), без реакции на hover ---
    const bool Capturing = (m_Binder.m_pKeyReaderId == &ButtonContainer && m_Binder.m_TakeKey);
    const float Alpha = Capturing ? 0.35f : 0.22f; // постоянная альфа; при захвате немного ярче

    // Немного уменьшим фон относительно всей области, чтобы текст не упирался в края
    CUIRect Pill = KeyButton;
    Pill.Margin(2.0f, &Pill);
    // Очень скруглённые углы: половина высоты
    const float Rounding = Pill.h * 0.5f;
    Pill.Draw(ColorRGBA(1.0f, 1.0f, 1.0f, Alpha), IGraphics::CORNER_ALL, Rounding);

    // Текст по центру строки (с тем же внутренним отступом)
    CUIRect TextRect = Pill;
    Ui()->DoLabel(&TextRect, aKeyText, 12.0f, TEXTALIGN_MC);
}

void CMenus::DoSliderRow(CUIRect &Column, float LineSize, const char *pLabel, int &Value, int Min, int Max, const char *pUnit, const char *pAdjust)
{
	CUIRect Row;
	Column.HSplitTop(LineSize, &Row, &Column);

	// Адаптивное разделение: левая метка и правая область слайдера
	CUIRect LabelRect, SliderArea;
	// Используем адаптивное разделение вместо фиксированного VSplitMid
	float MinLabelWidth = 80.0f;
	float LabelTarget = Row.w * 0.4f;
	float LabelHi = std::max(MinLabelWidth, Row.w * 0.6f);
	float LabelWidth = std::clamp(LabelTarget, MinLabelWidth, LabelHi);
	Row.VSplitLeft(LabelWidth, &LabelRect, &SliderArea);

	char aBuf[128];
	if(pUnit && pUnit[0])
		str_format(aBuf, sizeof(aBuf), "%s", Localize(pLabel));
	else
		str_format(aBuf, sizeof(aBuf), "%s", Localize(pLabel));
	Ui()->DoLabel(&LabelRect, aBuf, 10.0f, TEXTALIGN_ML);

	// Адаптивная область для значения слайдера
	CUIRect SliderRect, ValueRect;
	float ValueHi = std::max(40.0f, std::min(90.0f, SliderArea.w * 0.22f));
	float ValueWidth = std::clamp(64.0f, 40.0f, ValueHi);
	SliderArea.VSplitLeft(ValueWidth, &ValueRect, &SliderRect);
    // Сдвиг/удлинение слайдера по спецификации pAdjust
    float shiftOffset = 0.0f; // >0 вправо, <0 влево
    float extendLeft = 0.0f;
    float extendRight = 0.0f;
    if(pAdjust && pAdjust[0])
    {
        // Поддерживаем несколько директив через '+', регистр не важен.
        // Формат каждого сегмента: <N><code>, где code in {l, r, ll, lr}
        const char *s = pAdjust;
        while(*s)
        {
            // Пропускаем разделители
            while(*s == ' ' || *s == '+')
                ++s;
            if(!*s)
                break;

            // Читаем число
            int n = 0;
            while(*s >= '0' && *s <= '9')
            {
                n = n * 10 + (*s - '0');
                ++s;
            }

            // Читаем код (1-2 символа)
            char c1 = *s ? (char)tolower(*s) : 0; if(*s) ++s;
            char c2 = *s ? (char)tolower(*s) : 0;
            if(c1)
            {
                // Двухбуквенные сначала
                if(c1 == 'l' && c2 == 'l')
                {
                    extendLeft += (float)n;
                    ++s; // потребляем c2
                }
                else if(c1 == 'l' && c2 == 'r')
                {
                    extendRight += (float)n;
                    ++s; // потребляем c2
                }
                else if(c1 == 'l')
                {
                    shiftOffset -= (float)n;
                }
                else if(c1 == 'r')
                {
                    shiftOffset += (float)n;
                }
            }

            // двигаемся до следующего сегмента ('+' или конец)
            while(*s && *s != '+')
                ++s;
        }
    }
    if(shiftOffset != 0.0f)
    {
        ValueRect.x += shiftOffset;
        SliderRect.x += shiftOffset;
    }

    // Область под тонкий слайдер (рисуется) и широкая hit-область (для удобного перетаскивания)
    CUIRect Thin, Hit;
    const float TrackH = std::max(2.0f, LineSize * 0.25f); // тонкая линия
    // Hit-область: повыше, чтобы было удобно попадать мышью
    Hit = SliderRect;
    Hit.h = std::max(LineSize * 0.8f, 12.0f);
    Hit.y = Row.y + (Row.h - Hit.h) * 0.5f;
    // Тонкая линия по центру hit-области
    Thin.x = SliderRect.x;
    Thin.w = SliderRect.w;
    Thin.h = TrackH;
    Thin.y = Hit.y + (Hit.h - TrackH) * 0.5f;

    // Применяем удлинение линии (и hit-области), не двигая подпись и текст значения
    if(extendLeft > 0.0f)
    {
        Thin.x -= extendLeft;
        Thin.w += extendLeft;
        Hit.x -= extendLeft;
        Hit.w += extendLeft;
    }
    if(extendRight > 0.0f)
    {
        Thin.w += extendRight;
        Hit.w += extendRight;
    }

    // Логика: чистая обработка перетаскивания без рисования штатного слайдера
    float Rel = 0.0f;
    if(Max != Min)
        Rel = (float)(Value - Min) / (float)(Max - Min);
    Rel = std::clamp(Rel, 0.0f, 1.0f);

    float LocalX = Rel * Hit.w; // позиция ручки по текущему значению
    if(Ui()->DoPickerLogic(&Value, &Hit, &LocalX, nullptr) != EEditState::NONE)
    {
        Rel = std::clamp(LocalX / Hit.w, 0.0f, 1.0f);
        int NewVal = Min + round_to_int(Rel * (Max - Min));
        NewVal = std::clamp(NewVal, Min, Max);
        Value = NewVal;
    }

    // Текст текущего значения и единиц (справа, по правому краю)
    {
        char aVal[64];
        if(pUnit && pUnit[0])
            str_format(aVal, sizeof(aVal), "%d %s  ", Value, pUnit);
        else
            str_format(aVal, sizeof(aVal), "%d  ", Value);
        Ui()->DoLabel(&ValueRect, aVal, 8.0f, TEXTALIGN_MR);
    }

    // Рендер дорожки (тонкая линия)
    Graphics()->TextureClear();
    Graphics()->QuadsBegin();
    Graphics()->SetColor(1.0f, 1.0f, 1.0f, 0.15f);
    const float TrackY = Thin.y + (Thin.h * 0.5f - 1.0f);
    IGraphics::CQuadItem TrackQuad(Thin.x, TrackY, Thin.w, 2.0f);
    Graphics()->QuadsDrawTL(&TrackQuad, 1);
    Graphics()->QuadsEnd();

    // Прогресс-бар не рисуем по требованию — оставляем только тонкую линию и круглый ползунок

    // Рендер круглого ползунка иконкой (Font Awesome circle)
    const float KnobRadius = std::max(4.0f, TrackH * 0.9f);
    const float KnobX = Thin.x + Rel * Thin.w;
    const float KnobY = Thin.y + Thin.h * 0.5f;
    ITextRender *pTR2 = Ui()->TextRender();
    unsigned PrevFlags2 = pTR2->GetRenderFlags();
    pTR2->SetFontPreset(EFontPreset::ICON_FONT);
    pTR2->TextColor(1.0f, 1.0f, 1.0f, 1.0f);
    const float IconSize2 = KnobRadius * 2.0f;
    const float IconX2 = KnobX - IconSize2 * 0.5f;
    const float IconY2 = KnobY - IconSize2 * 0.5f;
    pTR2->Text(IconX2, IconY2, IconSize2, FontIcons::FONT_ICON_CIRCLE);
    pTR2->SetFontPreset(EFontPreset::DEFAULT_FONT);
    pTR2->SetRenderFlags(PrevFlags2);
    pTR2->TextColor(1,1,1,1);
}

void CMenus::RenderBackgroundLines(const CUIRect &Area, int SectionId)
{
	if(!g_Config.m_MRXBgLinesEnable)
		return;

	constexpr int MAX_SECTIONS = 64;
	// Map user speed to a visible range; 1..200 -> 0.02..4.0 (scale of relative units per second)
	const float SpeedScale = std::max(0.01f, g_Config.m_MRXBgLinesSpeed * 0.02f);
	const float BaseSpeedRel = 0.5f; // relative units per second baseline (higher for visible motion)
	const float Radius = std::clamp(g_Config.m_MRXBgLinesRadius * 1.0f, 1.0f, 20.0f);
	const float LineDist = std::clamp(g_Config.m_MRXBgLinesDistance * 1.0f, 1.0f, 500.0f);
	// Inverted opacity: 0% => fully opaque, 100% => fully transparent
	const float LinesAlphaBase = std::clamp(1.0f - (g_Config.m_MRXBgLinesAlpha / 100.0f), 0.0f, 1.0f);
	const int Points = std::clamp(g_Config.m_MRXBgLinesPoints, 3, 256);

	struct SPoint { vec2 rel; vec2 vel; };
	static SPoint s_aPoints[MAX_SECTIONS][256];
	static bool s_aInitialized[MAX_SECTIONS] = {};
	static int s_aInitedCount[MAX_SECTIONS] = {};
	static float s_aBaseFactor[MAX_SECTIONS][256] = {};

	const float relRadiusX = Radius / std::max(Area.w, 1.0f);
	const float relRadiusY = Radius / std::max(Area.h, 1.0f);
	// Size-based factor (used only for line density perception if needed)
	float sizeFactor = sqrtf(Area.w * Area.h) / 1900.0f;
	sizeFactor = std::clamp(sizeFactor, 0.2f, 1.5f);

	if(SectionId < 0 || SectionId >= MAX_SECTIONS)
		return;

	if(!s_aInitialized[SectionId])
	{
		// seed initial positions and velocities
		for(int i = 0; i < Points; i++)
		{
			// simple rng based on rand(); acceptable for UI animation
			float rx = relRadiusX + (float)rand() / (float)RAND_MAX * (1.0f - 2 * relRadiusX);
			float ry = relRadiusY + (float)rand() / (float)RAND_MAX * (1.0f - 2 * relRadiusY);
			const float TWO_PI = 6.28318530718f;
			float angle = (float)rand() / (float)RAND_MAX * TWO_PI;
			float base = 0.7f + ((float)rand() / (float)RAND_MAX) * 0.6f; // persistent per-point factor
			s_aBaseFactor[SectionId][i] = base;
			s_aPoints[SectionId][i].rel = vec2(rx, ry);
			vec2 dir = vec2(cosf(angle), sinf(angle));
			s_aPoints[SectionId][i].vel = dir * (BaseSpeedRel * SpeedScale * base);
		}
		s_aInitialized[SectionId] = true;
		s_aInitedCount[SectionId] = Points;
	}
	else if(s_aInitedCount[SectionId] < Points)
	{
		// lazily initialize additional points when user increases the count
		for(int i = s_aInitedCount[SectionId]; i < Points; ++i)
		{
			float rx = relRadiusX + (float)rand() / (float)RAND_MAX * (1.0f - 2 * relRadiusX);
			float ry = relRadiusY + (float)rand() / (float)RAND_MAX * (1.0f - 2 * relRadiusY);
			const float TWO_PI = 6.28318530718f;
			float angle = (float)rand() / (float)RAND_MAX * TWO_PI;
			float base = 0.7f + ((float)rand() / (float)RAND_MAX) * 0.6f;
			s_aBaseFactor[SectionId][i] = base;
			s_aPoints[SectionId][i].rel = vec2(rx, ry);
			vec2 dir = vec2(cosf(angle), sinf(angle));
			s_aPoints[SectionId][i].vel = dir * (BaseSpeedRel * SpeedScale * base);
		}
		s_aInitedCount[SectionId] = Points;
	}

	// Adaptive delta: engine may return ms or seconds depending on context
	float Delta = Client()->RenderFrameTime();
	if(Delta > 5.0f) // assume milliseconds if greater than ~5
		Delta /= 1000.0f;
	for(int i = 0; i < Points; i++)
	{
		// ensure base factor initialized for old points from previous versions
		if(s_aBaseFactor[SectionId][i] <= 0.0f)
		{
			s_aBaseFactor[SectionId][i] = 0.7f + ((float)rand() / (float)RAND_MAX) * 0.6f;
			// if velocity is zero, give random direction
			if(length(s_aPoints[SectionId][i].vel) < 1e-6f)
			{
				const float TWO_PI = 6.28318530718f;
				float angle = (float)rand() / (float)RAND_MAX * TWO_PI;
				s_aPoints[SectionId][i].vel = vec2(cosf(angle), sinf(angle));
			}
		}
		// normalize direction and apply current speed every frame
		float base = s_aBaseFactor[SectionId][i];
		vec2 dir = s_aPoints[SectionId][i].vel;
		float len = length(dir);
		if(len > 1e-6f)
			dir /= len;
		else
		{
			// fallback random direction
			const float TWO_PI = 6.28318530718f;
			float angle = (float)rand() / (float)RAND_MAX * TWO_PI;
			dir = vec2(cosf(angle), sinf(angle));
		}
		s_aPoints[SectionId][i].vel = dir * (SpeedScale * base * sizeFactor);

		s_aPoints[SectionId][i].rel += s_aPoints[SectionId][i].vel * Delta;
		// bounce within [relRadius, 1-relRadius]
		if(s_aPoints[SectionId][i].rel.x < relRadiusX) { s_aPoints[SectionId][i].rel.x = relRadiusX; s_aPoints[SectionId][i].vel.x *= -1; }
        if(s_aPoints[SectionId][i].rel.x > 1.0f - relRadiusX) { s_aPoints[SectionId][i].rel.x = 1.0f - relRadiusX; s_aPoints[SectionId][i].vel.x *= -1; }
        if(s_aPoints[SectionId][i].rel.y < relRadiusY) { s_aPoints[SectionId][i].rel.y = relRadiusY; s_aPoints[SectionId][i].vel.y *= -1; }
        if(s_aPoints[SectionId][i].rel.y > 1.0f - relRadiusY) { s_aPoints[SectionId][i].rel.y = 1.0f - relRadiusY; s_aPoints[SectionId][i].vel.y *= -1; }
	}

	// lines
	Graphics()->TextureClear();
	Graphics()->LinesBegin();
	const ColorRGBA Col = color_cast<ColorRGBA>(ColorHSLA(g_Config.m_MRXBgLinesColor, true));
	for(int i = 0; i < Points; i++)
	{
		const vec2 pos_i(Area.x + s_aPoints[SectionId][i].rel.x * Area.w, Area.y + s_aPoints[SectionId][i].rel.y * Area.h);
		for(int j = i + 1; j < Points; j++)
		{
			const vec2 pos_j(Area.x + s_aPoints[SectionId][j].rel.x * Area.w, Area.y + s_aPoints[SectionId][j].rel.y * Area.h);
			const float dist = distance(pos_i, pos_j);
			if(dist < LineDist)
			{
				const float a = LinesAlphaBase * (1.0f - dist / LineDist) * Col.a;
				Graphics()->SetColor(Col.r, Col.g, Col.b, a);
				IGraphics::CLineItem l(pos_i.x, pos_i.y, pos_j.x, pos_j.y);
				Graphics()->LinesDraw(&l, 1);
			}
		}
	}
	Graphics()->LinesEnd();

	if(g_Config.m_MRXBgLinesDots)
	{
		Graphics()->TextureClear();
		Graphics()->QuadsBegin();
		for(int i = 0; i < Points; i++)
		{
			const vec2 pos(Area.x + s_aPoints[SectionId][i].rel.x * Area.w, Area.y + s_aPoints[SectionId][i].rel.y * Area.h);
            const float a = std::min(1.0f, LinesAlphaBase + 0.1f) * Col.a;
			Graphics()->SetColor(Col.r, Col.g, Col.b, a);
			Graphics()->DrawCircle(pos.x, pos.y, Radius * 0.5f, 24);
		}
		Graphics()->QuadsEnd();
	}
}