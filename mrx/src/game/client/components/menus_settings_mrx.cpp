#include <game/localization.h>
#include <game/client/ui_scrollregion.h>
#include <game/client/gameclient.h>
#include <engine/input.h>
#include <engine/console.h>
#include <generated/client_data.h>
#include <algorithm>
#include <cmath>
#include <functional>
#include <vector>
#include <base/system.h>
#include <cstdlib>

#include "menus.h"

// Forward declaration for asset scanning
typedef std::function<void()> TMenuAssetScanLoadedFunc;
struct SMenuAssetScanUser
{
    void *m_pUser;
    TMenuAssetScanLoadedFunc m_LoadedFunc;
};

// ===== Shared style constants (T-client style) =====
namespace {
// Fonts and sizes
const float FONT_SIZE = 14.0f;
const float EDITBOX_FONT_SIZE = 12.0f;
const float LINE_SIZE = 20.0f;
const float COLOR_PICKER_LINE_SIZE = 25.0f;
const float HEADLINE_FONT_SIZE = 20.0f;
const float STANDARD_FONT_SIZE = 14.0f;

// Layout
const float HEADLINE_HEIGHT = HEADLINE_FONT_SIZE + 0.0f;
const float MARGIN = 10.0f;
const float MARGIN_SMALL = 5.0f;
const float MARGIN_EXTRA_SMALL = 2.5f;
const float MARGIN_BETWEEN_SECTIONS = 30.0f;
const float MARGIN_BETWEEN_VIEWS = 33.0f;
const float MARGIN_BLOCKS = 15.0f;

// Color picker specifics
const float COLOR_PICKER_LABEL_SIZE = 13.0f;
const float COLOR_PICKER_LINE_SPACING = 5.0f;
} // namespace

enum
{
    MRX_TAB_AIMBOT = 0,
    MRX_TAB_AVOID,
    MRX_TAB_TAS,
    MRX_TAB_MISC,
    MRX_TAB_VISUALS,
    MRX_TAB_FLY_HELPER,
    MRX_TAB_WHEEL_BINDS, // <--- новая вкладка
    NUMBER_OF_MRX_TABS
};

int CMenus::RenderTablist(CUIRect TabBar)
{
    // Tab button containers
    static CButtonContainer s_aPageTabs[NUMBER_OF_MRX_TABS] = {};
    static int s_CurMrxTab = 0;

    // Calculate tab width
    float TabWidth = TabBar.w / NUMBER_OF_MRX_TABS;
    CUIRect Tab;

    // Render tab buttons
    for(int i = 0; i < NUMBER_OF_MRX_TABS; i++)
    {
        TabBar.VSplitLeft(TabWidth, &Tab, &TabBar);

        // Calculate corners for rounded edges
        int Corners = 0;
        if(i == 0)
            Corners |= IGraphics::CORNER_L;
        if(i == NUMBER_OF_MRX_TABS - 1)
            Corners |= IGraphics::CORNER_R;

        // Get tab text
        const char *pTabText = "";
        switch(i)
        {
            case MRX_TAB_AIMBOT: pTabText = "Aimbot"; break;
            case MRX_TAB_AVOID: pTabText = "Avoid"; break;
            case MRX_TAB_MISC: pTabText = "Misc"; break;
            case MRX_TAB_VISUALS: pTabText = "Visuals"; break;
            case MRX_TAB_FLY_HELPER: pTabText = "Fly Helper"; break;
            case MRX_TAB_WHEEL_BINDS: pTabText = "Wheel Binds"; break;
            case MRX_TAB_TAS: pTabText = "TAS"; break;
        }

        if(i == MRX_TAB_AIMBOT)
{
             // Подготовим цвет в зависимости от активного состояния
    const bool Active = (s_CurMrxTab == i);
    const ColorRGBA ColActive = ColorRGBA(1.0f, 1.0f, 1.0f, 1.0f);
    const ColorRGBA ColInactive = ColorRGBA(1.0f, 1.0f, 1.0f, 0.75f);
    const ColorRGBA IconCol = Active ? ColActive : ColInactive;

    // Размер иконки и позиция
    const float IconSize = Tab.h * 0.6f; // 60% высоты таба
    const float IconX = Tab.x + 8.0f;    // небольшой отступ слева
    const float IconY = Tab.y + (Tab.h - IconSize) * 0.5f; // по центру по вертикали

    // Ставим пресет иконок и рисуем глиф
    ITextRender *pTR = Ui()->TextRender();
    const auto PrevFlags = pTR->GetRenderFlags();
    pTR->SetFontPreset(EFontPreset::ICON_FONT);
    pTR->TextColor(IconCol);
    pTR->Text(IconX, IconY, IconSize, FontIcons::FONT_ICON_CROSSHAIR);

    // ВАЖНО: вернуть пресет на обычный текст
    pTR->SetFontPreset(EFontPreset::DEFAULT_FONT);
    pTR->SetRenderFlags(PrevFlags);
    pTR->TextColor(1,1,1,1);
}

        if(DoButton_MenuTab(&s_aPageTabs[i], Localize(pTabText), s_CurMrxTab == i, &Tab, Corners))
            s_CurMrxTab = i;
    }

    return s_CurMrxTab;
}

#pragma region Aimbot
// Шаблоны функций для каждой вкладки
void CMenus::RenderTabPageAimbot(CUIRect MainView)
{
    // --- Стандартная настройка прокрутки и фона ---
    static CScrollRegion s_ScrollRegion;
    static std::vector<CUIRect> s_SectionBoxes;
    static vec2 s_PrevScrollOffset(0.0f, 0.0f);
    vec2 ScrollOffset(0.0f, 0.0f);
    CScrollRegionParams ScrollParams;
    ScrollParams.m_ScrollUnit = 120.0f;
    ScrollParams.m_Flags = CScrollRegionParams::FLAG_CONTENT_STATIC_WIDTH;
    ScrollParams.m_ScrollbarMargin = 5.0f;
    s_ScrollRegion.Begin(&MainView, &ScrollOffset, &ScrollParams);

    MainView.y += ScrollOffset.y;
    MainView.VSplitRight(5.0f, &MainView, nullptr);
    MainView.VSplitLeft(2.0f, nullptr, &MainView);

    // Сохраняем область контента для скролла
    CUIRect ContentArea = MainView;

    // Разделение на две колонки (адаптивно)
    float GapSize = MARGIN_BLOCKS*1.3;

    const float MinLeftWidth = 200.0f;
    const float MinRightWidth = 180.0f;

    // Доступная ширина под обе колонки и один межколоночный зазор
    float AvailableWidth = MainView.w - GapSize;
    float LeftWidth = 0.0f;

    if(AvailableWidth >= (MinLeftWidth + MinRightWidth))
    {
        // Нормальное/среднее разрешение: левая шире (контролов больше)
        LeftWidth = std::max(MinLeftWidth, AvailableWidth * 0.55f);
    }
    else
    {
        // Узкое окно: делим поровну с нижними пределами
        LeftWidth = std::max(MinLeftWidth * 0.8f, AvailableWidth * 0.5f);
    }

    CUIRect LeftView;
    MainView.VSplitLeft(LeftWidth, &LeftView, &MainView);
    MainView.VSplitLeft(GapSize, nullptr, &MainView);
    CUIRect RightView = MainView;

    LeftView.VSplitLeft(4.0f, nullptr, &LeftView);
    RightView.VSplitRight(MARGIN_SMALL, &RightView, nullptr);

    // Отрисовка фонов прошлых секций
    {
        for(size_t i = 0; i < s_SectionBoxes.size(); ++i)
        {
            RenderBackgroundLines(s_SectionBoxes[i], (int)i);
            CUIRect Section = s_SectionBoxes[i];
            float Padding = MARGIN_BETWEEN_VIEWS * 0.6666f;
            Section.w += Padding;
            Section.h += Padding;
            Section.x -= Padding * 0.5f;
            Section.y -= Padding * 0.5f;
            Section.y -= s_PrevScrollOffset.y - ScrollOffset.y;
            float Shade = 0.0f;
            Section.Draw(ColorRGBA(Shade, Shade, Shade, 0.25f), IGraphics::CORNER_ALL, 10.0f);
        }
        s_PrevScrollOffset = ScrollOffset;
        s_SectionBoxes.clear();
    }


    // ***** Левая колонка ***** //
    CUIRect LeftColumn = LeftView;

    // --- Секция: Aimbot ---
    {
        CUIRect Header;
        LeftColumn.HSplitTop(MARGIN, nullptr, &LeftColumn);
        {
            CUIRect Bg = LeftColumn;
            Bg.VMargin(MARGIN_SMALL, &Bg);
            s_SectionBoxes.push_back(Bg);
        }
        LeftColumn.HSplitTop(HEADLINE_HEIGHT, &Header, &LeftColumn);
        Ui()->DoLabel(&Header, Localize("Aimbot"), HEADLINE_FONT_SIZE, TEXTALIGN_ML);
        LeftColumn.HSplitTop(MARGIN_SMALL, nullptr, &LeftColumn);

        DoCheckBoxRow(LeftColumn, LINE_SIZE, Localize("Enable Aimbot"), g_Config.m_MRXAimbotEnable);
        LeftColumn.HSplitTop(MARGIN_EXTRA_SMALL, nullptr, &LeftColumn);

        // FOV
        DoSliderRow(LeftColumn, LINE_SIZE, Localize("FOV"), g_Config.m_MRXAimbotFov, 1, 360, "", "");
        LeftColumn.HSplitTop(MARGIN_EXTRA_SMALL, nullptr, &LeftColumn);

        // Silent
        DoCheckBoxRow(LeftColumn, LINE_SIZE, Localize("Silent"), g_Config.m_MRXAimbotSilent);
        LeftColumn.HSplitTop(MARGIN_EXTRA_SMALL, nullptr, &LeftColumn);

        // Auto Hook
        static CButtonContainer s_HookVisibleKeyButton;
        static int s_HookVisibleKey = 0;
        static int s_HookVisibleMod = 0;
        DoKeyBindRow(LeftColumn, LINE_SIZE, Localize("AutoHook"), "+toggle mrx_aimbot_autohook 1 0", s_HookVisibleKeyButton, s_HookVisibleKey, s_HookVisibleMod);
        LeftColumn.HSplitTop(MARGIN_EXTRA_SMALL, nullptr, &LeftColumn);

        // Auto Hammer Hit — теперь как бинд
        static CButtonContainer s_AutoHitKeyButton;
        static int s_AutoHitKey = 0;
        static int s_AutoHitMod = 0;
        DoKeyBindRow(LeftColumn, LINE_SIZE, Localize("AutoHammerHit"), "+mrx_aimbot_autohit", s_AutoHitKeyButton, s_AutoHitKey, s_AutoHitMod);
        LeftColumn.HSplitTop(MARGIN_EXTRA_SMALL, nullptr, &LeftColumn);

        // Edge scan
        DoCheckBoxRow(LeftColumn, LINE_SIZE, Localize("Edge Scan"), g_Config.m_MRXAimbotEdge);
        LeftColumn.HSplitTop(MARGIN_EXTRA_SMALL, nullptr, &LeftColumn);

        // Accuracy
        DoSliderRow(LeftColumn, LINE_SIZE, Localize("Accuracy"), g_Config.m_MRXAimbotAccuracy, 1, 100, "", "");
        LeftColumn.HSplitTop(MARGIN_EXTRA_SMALL, nullptr, &LeftColumn);

        static CButtonContainer s_AimToggleKeyButton;
        static int s_AimKey = 0;
        static int s_AimMod = 0;
        DoKeyBindRow(LeftColumn, LINE_SIZE, "Toggle AimBot", "toggle mrx_aimbot_enable 1 0", s_AimToggleKeyButton, s_AimKey, s_AimMod);

        s_SectionBoxes.back().h = LeftColumn.y - s_SectionBoxes.back().y;
    }

    

    // ***** Правая колонка ***** //
    CUIRect RightColumn = RightView;

    // --- Секция: ESP/Visual ---
    {
        CUIRect Header;
        RightColumn.HSplitTop(MARGIN, nullptr, &RightColumn);
        {
            CUIRect Bg = RightColumn;
            Bg.VMargin(MARGIN_SMALL, &Bg);
            s_SectionBoxes.push_back(Bg);
        }
        RightColumn.HSplitTop(HEADLINE_HEIGHT, &Header, &RightColumn);
        Ui()->DoLabel(&Header, Localize("ESP/Visual"), HEADLINE_FONT_SIZE, TEXTALIGN_ML);
        RightColumn.HSplitTop(MARGIN_SMALL, nullptr, &RightColumn);

        // ESP Enable
        DoCheckBoxRow(RightColumn, LINE_SIZE, Localize("Enable ESP"), g_Config.m_MRXEspEnable);
        RightColumn.HSplitTop(MARGIN_EXTRA_SMALL, nullptr, &RightColumn);

        // Draw FOV
        DoCheckBoxRow(RightColumn, LINE_SIZE, Localize("Draw FOV"), g_Config.m_MRXEspDrawFov);
        RightColumn.HSplitTop(MARGIN_EXTRA_SMALL, nullptr, &RightColumn);

        // Draw Box
        DoCheckBoxRow(RightColumn, LINE_SIZE, Localize("Draw Box"), g_Config.m_MRXEspDrawBox);
        RightColumn.HSplitTop(MARGIN_EXTRA_SMALL, nullptr, &RightColumn);

        //GlowPlayer
        DoCheckBoxRow(RightColumn, LINE_SIZE, Localize("Glow Player"), g_Config.m_MRXEspGlowPlayer);
        RightColumn.HSplitTop(MARGIN_EXTRA_SMALL, nullptr, &RightColumn);

        s_SectionBoxes.back().h = RightColumn.y - s_SectionBoxes.back().y;
    }


    // Завершение скролла
    const float EndY = std::max(LeftColumn.y, RightColumn.y);
    CUIRect ScrollRegion = ContentArea;
    ScrollRegion.y = ContentArea.y;
    ScrollRegion.h = EndY - ContentArea.y;
    s_ScrollRegion.AddRect(ScrollRegion);
    s_ScrollRegion.End();
}
#pragma endregion
#pragma region Avoid
void CMenus::RenderTabPageAvoid(CUIRect MainView)
{
    // Shared layout
    static CScrollRegion s_ScrollRegion;
    static std::vector<CUIRect> s_SectionBoxes;
    static vec2 s_PrevScrollOffset(0.0f, 0.0f);
    vec2 ScrollOffset(0.0f, 0.0f);
    CScrollRegionParams ScrollParams;
    ScrollParams.m_ScrollUnit = 120.0f;
    ScrollParams.m_Flags = CScrollRegionParams::FLAG_CONTENT_STATIC_WIDTH;
    ScrollParams.m_ScrollbarMargin = 5.0f;
    s_ScrollRegion.Begin(&MainView, &ScrollOffset, &ScrollParams);

    // Учёт прокрутки и поля под скроллбар
    MainView.y += ScrollOffset.y;
    MainView.VSplitRight(5.0f, &MainView, nullptr);
    MainView.VSplitLeft(2.0f, nullptr, &MainView);

    // Разделение на две колонки (адаптивно)
    CUIRect LeftView, RightView;
    float GapSize = MARGIN_BLOCKS*1.3;

    const float MinLeftWidth = 200.0f;
    const float MinRightWidth = 180.0f;

    // Доступная ширина под обе колонки и один межколоночный зазор
    float AvailableWidth = MainView.w - GapSize;
    float LeftWidth = 0.0f;

    if(AvailableWidth >= (MinLeftWidth + MinRightWidth))
    {
        // Нормальное/среднее разрешение: левая шире (контролов больше)
        LeftWidth = std::max(MinLeftWidth, AvailableWidth * 0.55f);
    }
    else
    {
        // Узкое окно: делим поровну с нижними пределами
        LeftWidth = std::max(MinLeftWidth * 0.8f, AvailableWidth * 0.5f);
    }

    MainView.VSplitLeft(LeftWidth, &LeftView, &MainView);
    MainView.VSplitLeft(GapSize, nullptr, &MainView);
    RightView = MainView; // остаток под правую колонку

    LeftView.VSplitLeft(4.0f, nullptr, &LeftView);
    RightView.VSplitRight(MARGIN_SMALL, &RightView, nullptr);

    // Отрисовка фонов прошлых секций
    for(size_t i = 0; i < s_SectionBoxes.size(); ++i)
    {
        RenderBackgroundLines(s_SectionBoxes[i], (int)i);
        CUIRect Section = s_SectionBoxes[i];
        float Padding = MARGIN_BETWEEN_VIEWS * 0.6666f;
        Section.w += Padding;
        Section.h += Padding;
        Section.x -= Padding * 0.5f;
        Section.y -= Padding * 0.5f;
        Section.y -= s_PrevScrollOffset.y - ScrollOffset.y;
        float Shade = 0.0f;
        Section.Draw(ColorRGBA(Shade, Shade, Shade, 0.25f), IGraphics::CORNER_ALL, 10.0f);
    }
    s_PrevScrollOffset = ScrollOffset;
    s_SectionBoxes.clear();

    // ***** Левая колонка ***** //
    CUIRect Column = LeftView;

    // --- Секция: AvoidFreeze (Legit) ---
    {
        CUIRect Bg;
        Column.HSplitTop(MARGIN, nullptr, &Column);
        Bg = Column;
        Bg.VMargin(MARGIN_SMALL, &Bg);
        s_SectionBoxes.push_back(Bg);

        CUIRect Header;
        Column.HSplitTop(HEADLINE_HEIGHT, &Header, &Column);
        Ui()->DoLabel(&Header, Localize("AvoidFreeze (Legit)"), HEADLINE_FONT_SIZE, TEXTALIGN_ML);
        Column.HSplitTop(MARGIN_SMALL, nullptr, &Column);

        DoCheckBoxRow(Column, LINE_SIZE, "Enable", g_Config.m_MRXAvoidEnable);
        DoCheckBoxRow(Column, LINE_SIZE, "Hook assistance", g_Config.m_MRXAvoidHookAssist);
        DoCheckBoxRow(Column, LINE_SIZE, "Direction assistance", g_Config.m_MRXAvoidDirectionAssist);
        static CButtonContainer s_AvoidToggleKeyButton;
        static int s_AvoidKey = 0;
        static int s_AvoidMod = 0;
        DoKeyBindRow(Column, LINE_SIZE, "Toggle Avoid", "toggle mrx_avoid_enable 0 1", s_AvoidToggleKeyButton, s_AvoidKey, s_AvoidMod);

        Column.HSplitTop(MARGIN_EXTRA_SMALL, nullptr, &Column);

        DoSliderRow(Column, LINE_SIZE, "Check ticks (direction)", g_Config.m_MRXAvoidCheckTicks, 1, 20, "", "40l+30lr");
        DoSliderRow(Column, LINE_SIZE, "Check ticks (hook)", g_Config.m_MRXAvoidCheckTicksHook, 1, 30, "", "40l+30lr");

        Column.HSplitTop(MARGIN_EXTRA_SMALL, nullptr, &Column);

        s_SectionBoxes.back().h = Column.y - s_SectionBoxes.back().y;
    }

    // ***** Правая колонка ***** //
    Column = RightView;

    // --- Секция: Tiles (перенесена) ---
    {
        CUIRect Bg;
        Column.HSplitTop(MARGIN, nullptr, &Column);
        Bg = Column;
        Bg.VMargin(MARGIN_SMALL, &Bg);
        s_SectionBoxes.push_back(Bg);

        CUIRect Header;
        Column.HSplitTop(HEADLINE_HEIGHT, &Header, &Column);
        Ui()->DoLabel(&Header, Localize("Tiles"), HEADLINE_FONT_SIZE, TEXTALIGN_ML);
        Column.HSplitTop(MARGIN_SMALL, nullptr, &Column);

            DoCheckBoxRow(Column, LINE_SIZE, "Freeze", g_Config.m_MRXAvoidTileFreeze);
            DoCheckBoxRow(Column, LINE_SIZE, "Death", g_Config.m_MRXAvoidTileDeath);
            DoCheckBoxRow(Column, LINE_SIZE, "Tele", g_Config.m_MRXAvoidTileTele);
            DoCheckBoxRow(Column, LINE_SIZE, "Unfreeze", g_Config.m_MRXAvoidTileUnfreeze);

        s_SectionBoxes.back().h = Column.y - s_SectionBoxes.back().y;
    }

    Column.HSplitTop(MARGIN_BETWEEN_VIEWS, nullptr, &Column); // Отступ между блоками

    // End scroll
    CUIRect ScrollRegion;
    ScrollRegion.x = MainView.x;
    ScrollRegion.y = maximum(LeftView.y, RightView.y) + MARGIN * 2.0f;
    ScrollRegion.w = MainView.w;
    ScrollRegion.h = 0.0f;
    s_ScrollRegion.AddRect(ScrollRegion);
    s_ScrollRegion.End();
}
#pragma endregion

#pragma region Misc
void CMenus::RenderTabPageMisc(CUIRect MainView)
{
    // Прокрутка и секции
    static CScrollRegion s_ScrollRegion;
    static std::vector<CUIRect> s_SectionBoxes;
    static vec2 s_PrevScrollOffset(0.0f, 0.0f);
    vec2 ScrollOffset(0.0f, 0.0f);
    CScrollRegionParams ScrollParams;
    ScrollParams.m_ScrollUnit = 120.0f;
    ScrollParams.m_Flags = CScrollRegionParams::FLAG_CONTENT_STATIC_WIDTH;
    ScrollParams.m_ScrollbarMargin = 5.0f;
    s_ScrollRegion.Begin(&MainView, &ScrollOffset, &ScrollParams);

    // Учёт прокрутки и поля под скроллбар
    MainView.y += ScrollOffset.y;
    MainView.VSplitRight(5.0f, &MainView, nullptr);
    MainView.VSplitLeft(2.0f, nullptr, &MainView);

    // Адаптивное разделение на три колонки с минимальными размерами
    CUIRect LeftView, MidView, RightView;
    const float GapSize1 = MARGIN_BLOCKS;
    const float GapSize2 = MARGIN_BLOCKS * 1.3f; 

    const float TotalGapWidth = GapSize1 + GapSize2;
    const float AvailableContentWidth = MainView.w - TotalGapWidth;

    const float ColumnWidth = AvailableContentWidth / 3.0f;

    MainView.VSplitLeft(ColumnWidth, &LeftView, &MainView);
    MainView.VSplitLeft(GapSize1, nullptr, &MainView);
    MainView.VSplitLeft(ColumnWidth, &MidView, &MainView);
    MainView.VSplitLeft(GapSize2, nullptr, &MainView);
    RightView = MainView;
    
    LeftView.VSplitLeft(4.0f, nullptr, &LeftView);
    MidView.VSplitLeft(4.0f, nullptr, &MidView);
    RightView.VSplitRight(MARGIN_SMALL, &RightView, nullptr);

    // Отрисовка фонов прошлых секций
    for(size_t i = 0; i < s_SectionBoxes.size(); ++i)
    {
        RenderBackgroundLines(s_SectionBoxes[i], (int)i);
        CUIRect Section = s_SectionBoxes[i];
        float Padding = MARGIN_BETWEEN_VIEWS * 0.6666f;
        Section.w += Padding;
        Section.h += Padding;
        Section.x -= Padding * 0.5f;
        Section.y -= Padding * 0.5f;
        Section.y -= s_PrevScrollOffset.y - ScrollOffset.y;
        float Shade = 0.0f;
        Section.Draw(ColorRGBA(Shade, Shade, Shade, 0.25f), IGraphics::CORNER_ALL, 10.0f);
    }
    s_PrevScrollOffset = ScrollOffset;
    s_SectionBoxes.clear();

    // ***** LeftView ***** //
    
    CUIRect Column = LeftView;
    CUIRect Header;
    
    // Секция: Misc
    Column.HSplitTop(MARGIN, nullptr, &Column);
    { CUIRect Bg = Column; Bg.VMargin(MARGIN_SMALL, &Bg); s_SectionBoxes.push_back(Bg); }
    Column.HSplitTop(HEADLINE_HEIGHT, &Header, &Column);
    Ui()->DoLabel(&Header, Localize("Misc Settings"), HEADLINE_FONT_SIZE, TEXTALIGN_ML);
    Column.HSplitTop(MARGIN_SMALL, nullptr, &Column);

    DoCheckBoxRow(Column, LINE_SIZE, "Auto aled", g_Config.m_MRXAutoAled);
    DoCheckBoxRow(Column, LINE_SIZE, "Dummy aled", g_Config.m_MRXDummyAled);
    DoCheckBoxRow(Column, LINE_SIZE, "Dummy hookcursor", g_Config.m_MRXDummyHookCursor);
    DoCheckBoxRow(Column, LINE_SIZE, "Hook only dummy", g_Config.m_MRXAutoHookDummy);
    DoCheckBoxRow(Column, LINE_SIZE, "NullMovement", g_Config.m_MRXNullMovement);
    DoCheckBoxRow(Column, LINE_SIZE, "ReverseCopy", g_Config.m_MRXReverseDummyCopyMoves);
    DoCheckBoxRow(Column, LINE_SIZE, "Hide chat emote", g_Config.m_MRXHideChatEmote);

    // Pseudofire main toggle with two small checkboxes on the right (LBM/RBM)
    {
        CUIRect Row, MainToggle, BottomArea, LbmRect, RbmRect, TargetRow;

        // Верхняя строка: основной переключатель MRXDuoPseudo
        Column.HSplitTop(LINE_SIZE, &Row, &Column);
        DoCheckBoxRow(Row, LINE_SIZE, Localize("MRXDuoPseudo"), g_Config.m_MRXDuoPseudo);

        if(g_Config.m_MRXDuoPseudo)
        {
            Column.HSplitTop(LINE_SIZE, &TargetRow, &Column);
            DoCircleToggleRow(TargetRow, LINE_SIZE, Localize("MRXDuoPseudo Target"), g_Config.m_MRXDuoPseudoTarget);

            
            // Toggles for AutoAim and Allow First Hit Outside 63
            DoCircleToggleRow(Column, LINE_SIZE, "MRXDuoPseudo AutoAim", g_Config.m_MRXDuoPseudoAutoAim);
            DoCircleToggleRow(Column, LINE_SIZE, "MRXDuoPseudo Fire", g_Config.m_MRXDuoPseudoFire);
            
            Column.HSplitTop(LINE_SIZE, &BottomArea, &Column);
        }
    }
    
    DoGearToggleRow(Column, LINE_SIZE, "Balance Bot Settings", g_Config.m_MRXShowBalanceBot);

    s_SectionBoxes.back().h = Column.y - s_SectionBoxes.back().y;

    // --- Section: SpoofClient ---
    {
        CUIRect Bg;
        Column.HSplitTop(MARGIN_BETWEEN_SECTIONS, nullptr, &Column);
        Bg = Column;
        Bg.VMargin(MARGIN_SMALL, &Bg);
        s_SectionBoxes.push_back(Bg);

        CUIRect Header;
        Column.HSplitTop(HEADLINE_HEIGHT, &Header, &Column);
        Ui()->DoLabel(&Header, Localize("SpoofClient"), HEADLINE_FONT_SIZE, TEXTALIGN_ML);
        Column.HSplitTop(MARGIN_SMALL, nullptr, &Column);

        // Preset dropdown (applies only version number/string and enables spoof)
        DoCheckBoxRow(Column, LINE_SIZE, "SpoofClient", g_Config.m_MRXSpoofedVersion);

        {
            static const char *s_aPresetNames[] = {
                "DDNet 19.4",
                "DDNet 19.3",
            };
            static int s_SpoofPreset = 0;
            static CUi::SDropDownState s_SpoofPresetDropDown;
            static CScrollRegion s_SpoofPresetScrollRegion;
            s_SpoofPresetDropDown.m_SelectionPopupContext.m_pScrollRegion = &s_SpoofPresetScrollRegion;
            int NewPreset = DoLabeledDropDownRow(Column, LINE_SIZE, Localize("Preset:"), s_SpoofPreset, s_aPresetNames, std::size(s_aPresetNames), s_SpoofPresetDropDown);
            if(NewPreset != s_SpoofPreset)
            {
                s_SpoofPreset = NewPreset;
                IConsole *pConsole = GameClient()->Console();
                if(s_SpoofPreset == 1) // 19.4
                {
                    pConsole->ExecuteLine("mrx_spoofversionnumber 19040");
                    pConsole->ExecuteLine("mrx_spoofversionstring \"19.4\"");
                }
                else if(s_SpoofPreset == 2) // 19.3
                {
                    pConsole->ExecuteLine("mrx_spoofversionnumber 19030");
                    pConsole->ExecuteLine("mrx_spoofversionstring \"19.3\"");
                }
            }
        }

        // String inputs: Version string and Version number (as text)
        {
            static bool s_InitedSpoof = false;
            static CLineInputBuffered<64> s_InputVersionStr;
            static CLineInputBuffered<16> s_InputVersionNum;
            if(!s_InitedSpoof)
            {
                s_InputVersionStr.Set(g_Config.m_MRXSpoofedVersionString);
                char aNumBuf[16];
                str_format(aNumBuf, sizeof(aNumBuf), "%d", g_Config.m_MRXSpoofedVersionNumber);
                s_InputVersionNum.Set(aNumBuf);
                s_InitedSpoof = true;
            }

            // Version string
            CUIRect Row, Edit;
            Column.HSplitTop(LINE_SIZE, &Row, &Column);
            Row.VSplitRight(120.0f, &Row, &Edit); // Edit — правая часть фиксированной ширины
            Ui()->DoLabel(&Row, Localize("Version string"), 12, TEXTALIGN_ML);
            if(Ui()->DoClearableEditBox(&s_InputVersionStr, &Edit, EDITBOX_FONT_SIZE))
            {
                str_copy(g_Config.m_MRXSpoofedVersionString, s_InputVersionStr.GetString(), sizeof(g_Config.m_MRXSpoofedVersionString));
            }

            Column.HSplitTop(MARGIN_EXTRA_SMALL, nullptr, &Column);

            // Version number (text input -> int)
            Column.HSplitTop(LINE_SIZE, &Row, &Column);
            Row.VSplitRight(120.0f, &Row, &Edit);
            Ui()->DoLabel(&Row, Localize("Version number"), 12, TEXTALIGN_ML);
            if(Ui()->DoClearableEditBox(&s_InputVersionNum, &Edit, EDITBOX_FONT_SIZE))
            {
                int NewVal = std::atoi(s_InputVersionNum.GetString());
                g_Config.m_MRXSpoofedVersionNumber = NewVal;
                char aNumBuf2[16];
                str_format(aNumBuf2, sizeof(aNumBuf2), "%d", g_Config.m_MRXSpoofedVersionNumber);
                s_InputVersionNum.Set(aNumBuf2);
            }
        }
    
        s_SectionBoxes.back().h = Column.y - s_SectionBoxes.back().y;
    
    }


    // ***** MidView ***** //
    Column = MidView;
    
    if(g_Config.m_MRXShowBalanceBot)
    {
        Column.HSplitTop(MARGIN, nullptr, &Column);
        {
            CUIRect Bg = Column;
            Bg.VMargin(MARGIN_SMALL, &Bg);
            s_SectionBoxes.push_back(Bg);
        }
        Column.HSplitTop(HEADLINE_HEIGHT, &Header, &Column);
        Ui()->DoLabel(&Header, Localize("BalanceBot Settings"), HEADLINE_FONT_SIZE, TEXTALIGN_ML);
        Column.HSplitTop(MARGIN_SMALL, nullptr, &Column);

        static CButtonContainer s_BalanceBotKeyButton2;
        static int s_BalanceBotKey2 = 0;
        static int s_BalanceBotModifier2 = 0;
        DoKeyBindRow(Column, LINE_SIZE, "Balancebot key", "toggle mrx_balancebot 0 1", s_BalanceBotKeyButton2, s_BalanceBotKey2, s_BalanceBotModifier2);

        Column.HSplitTop(MARGIN_EXTRA_SMALL, nullptr, &Column);

        {
            static const char *s_BalanceBotModeNames[] = {
                Localize("Below"),
                Localize("Above"),
                Localize("Both")};
            static CUi::SDropDownState s_BalanceBotModeDropDown;
            static CScrollRegion s_BalanceBotModeScrollRegion;
            s_BalanceBotModeDropDown.m_SelectionPopupContext.m_pScrollRegion = &s_BalanceBotModeScrollRegion;
            g_Config.m_MRXBalanceBotVFilter = DoLabeledDropDownRow(Column, LINE_SIZE, Localize("Mode:"), g_Config.m_MRXBalanceBotVFilter, s_BalanceBotModeNames, std::size(s_BalanceBotModeNames), s_BalanceBotModeDropDown);
        }

        Column.HSplitTop(MARGIN_EXTRA_SMALL, nullptr, &Column);

        {
            static const char *s_BalanceBotPreciseNames[] = {
                Localize("Normal"),
                Localize("Precise"),
                Localize("Legit")};
            static CUi::SDropDownState s_BalanceBotPreciseDropDown;
            static CScrollRegion s_BalanceBotPreciseScrollRegion;
            s_BalanceBotPreciseDropDown.m_SelectionPopupContext.m_pScrollRegion = &s_BalanceBotPreciseScrollRegion;
            g_Config.m_MRXBalanceBotPrecise = DoLabeledDropDownRow(Column, LINE_SIZE, Localize("Precise:"), g_Config.m_MRXBalanceBotPrecise, s_BalanceBotPreciseNames, std::size(s_BalanceBotPreciseNames), s_BalanceBotPreciseDropDown);
        }

        s_SectionBoxes.back().h = Column.y - s_SectionBoxes.back().y;
    }

    // --- Section: Auto Team ---
    {
        CUIRect Bg;
        Column.HSplitTop(MARGIN_BETWEEN_SECTIONS, nullptr, &Column);
        Bg = Column;
        Bg.VMargin(MARGIN_SMALL, &Bg);
        s_SectionBoxes.push_back(Bg);

        CUIRect Header;
        Column.HSplitTop(HEADLINE_HEIGHT, &Header, &Column);
        Ui()->DoLabel(&Header, Localize("Auto Team"), HEADLINE_FONT_SIZE, TEXTALIGN_ML);
        Column.HSplitTop(MARGIN_SMALL, nullptr, &Column);

        DoCheckBoxRow(Column, LINE_SIZE, "Auto Team", g_Config.m_MRXAutoTeamEnable);
        Column.HSplitTop(MARGIN_EXTRA_SMALL, nullptr, &Column);
        
        DoSliderRow(Column, LINE_SIZE, "Team", g_Config.m_MRXAutoTeamNumber, 1, 63, "", "40l+30lr");
        Column.HSplitTop(MARGIN_EXTRA_SMALL, nullptr, &Column);
        
        DoCheckBoxRow(Column, LINE_SIZE, "Invite Dummy", g_Config.m_MRXAutoTeamInviteDummy);
        Column.HSplitTop(MARGIN_EXTRA_SMALL, nullptr, &Column);
        
        // Mode selection: Auto on join or By bind
        {
            static const char *s_aModeNames[] = {
                "Auto",
                "Bind"
            };
            static CUi::SDropDownState s_ModeDropDown;
            static CScrollRegion s_ModeScrollRegion;
            s_ModeDropDown.m_SelectionPopupContext.m_pScrollRegion = &s_ModeScrollRegion;
            int NewMode = DoLabeledDropDownRow(Column, LINE_SIZE, Localize("Mode:"), g_Config.m_MRXAutoTeamMode, s_aModeNames, std::size(s_aModeNames), s_ModeDropDown);
            if(NewMode != g_Config.m_MRXAutoTeamMode)
            {
                g_Config.m_MRXAutoTeamMode = NewMode;
            }
        }
        
        // Show bind input only if "By bind" mode is selected
        if(g_Config.m_MRXAutoTeamMode == 1)
        {
            Column.HSplitTop(MARGIN_EXTRA_SMALL, nullptr, &Column);
            static CButtonContainer s_AutoTeamBindButton;
            static int s_AutoTeamBindKey = 0;
            static int s_AutoTeamBindModifier = 0;
            DoKeyBindRow(Column, LINE_SIZE, "Bind", "toggle mrx_auto_team_execute 0 1", s_AutoTeamBindButton, s_AutoTeamBindKey, s_AutoTeamBindModifier);
        }
        
        s_SectionBoxes.back().h = Column.y - s_SectionBoxes.back().y;
    }

    // ***** RightView ***** //
    Column = RightView;
    Column.HSplitTop(MARGIN, nullptr, &Column);
    {
        CUIRect Bg = Column;
        Bg.VMargin(MARGIN_SMALL, &Bg);
        s_SectionBoxes.push_back(Bg);
    }
    Column.HSplitTop(HEADLINE_HEIGHT, &Header, &Column);
    Ui()->DoLabel(&Header, Localize("Binds"), HEADLINE_FONT_SIZE, TEXTALIGN_ML);
    Column.HSplitTop(MARGIN_SMALL, nullptr, &Column);

    static CButtonContainer s_AdvancedDeepFlyKeyButton;
    static int s_AdvancedDeepFlyKey = 0;
    static int s_AdvancedDeepFlyModifier = 0;
    DoKeyBindRow(Column, LINE_SIZE, "Advanced deepfly", "+toggle mrx_advanceddeepfly_manual 1 0", s_AdvancedDeepFlyKeyButton, s_AdvancedDeepFlyKey, s_AdvancedDeepFlyModifier);
    Column.HSplitTop(MARGIN_EXTRA_SMALL, nullptr, &Column);
        
    static CButtonContainer s_SlowWalkKeyButton;
    static int s_SlowWalkKey = 0;
    static int s_SlowWalkModifier = 0;
    DoKeyBindRow(Column, LINE_SIZE, "SlowWalk key", "toggle mrx_slow_walk 1 0", s_SlowWalkKeyButton, s_SlowWalkKey, s_SlowWalkModifier);
    Column.HSplitTop(MARGIN_EXTRA_SMALL, nullptr, &Column);

    static CButtonContainer s_PixelWalkKeyButton;
    static int s_PixelWalkKey = 0;
    static int s_PixelWalkModifier = 0;
    DoKeyBindRow(Column, LINE_SIZE, "Pixelwalk key", "+toggle mrx_pixelwalk 1 0", s_PixelWalkKeyButton, s_PixelWalkKey, s_PixelWalkModifier);
    Column.HSplitTop(MARGIN_EXTRA_SMALL, nullptr, &Column);

    static CButtonContainer s_LeftKeyButton;
    static int s_LeftKey = 0;
    static int s_LeftModifier = 0;
    DoKeyBindRow(Column, LINE_SIZE, "Left+jump key", "+jump; +left", s_LeftKeyButton, s_LeftKey, s_LeftModifier);
    Column.HSplitTop(MARGIN_EXTRA_SMALL, nullptr, &Column);

    static CButtonContainer s_RightKeyButton;
    static int s_RightKey = 0;
    static int s_RightModifier = 0;
    DoKeyBindRow(Column, LINE_SIZE, "Rigth+jump key", "+jump; +right", s_RightKeyButton, s_RightKey, s_RightModifier);
    Column.HSplitTop(MARGIN_EXTRA_SMALL, nullptr, &Column);

    static CButtonContainer s_DummyRightKeyButton;
    static int s_DummyRightKey = 0;
    static int s_DummyRightModifier = 0;
    DoKeyBindRow(Column, LINE_SIZE, "DummyRight key", "cl_dummy_right", s_DummyRightKeyButton, s_DummyRightKey, s_DummyRightModifier);
    Column.HSplitTop(MARGIN_EXTRA_SMALL, nullptr, &Column);

    static CButtonContainer s_DummyLeftKeyButton;
    static int s_DummyLefttKey = 0;
    static int s_DummyLeftModifier = 0;
    DoKeyBindRow(Column, LINE_SIZE, "DummyLeft key", "cl_dummy_left", s_DummyLeftKeyButton, s_DummyLefttKey, s_DummyLeftModifier);
    Column.HSplitTop(MARGIN_EXTRA_SMALL, nullptr, &Column);

    //45aim
    float CurrentMaxDistance = g_Config.m_ClMouseMaxDistance;
    int CurrentMouseSens = g_Config.m_InpMousesens;

    char aCommand[256];

    str_format(aCommand, sizeof(aCommand),
        "+toggle cl_mouse_max_distance 2.0 %.2f; " // 2.0 - для 45deg, %.2f - текущее значение (float)
        "+toggle inp_mousesens 1 %d; " // 1 - для 45deg, %d - текущее значение (int)
        "+showhookcoll",
        CurrentMaxDistance,
        CurrentMouseSens);

    static CButtonContainer s_Aim45KeyButton;
    static int s_Aim45Key = 0;
    static int s_Aim45Modifier = 0;
    DoKeyBindRow(Column, LINE_SIZE, "45° Aiming key", aCommand, s_Aim45KeyButton, s_Aim45Key, s_Aim45Modifier);
    Column.HSplitTop(MARGIN_EXTRA_SMALL, nullptr, &Column);

    static CButtonContainer s_DummyThrowButton;
    static int s_DummyThrowKey = 0;
    static int s_DummyThrowModifier = 0;
    DoKeyBindRow(Column, LINE_SIZE, "DummyThrow key", "toggle mrx_throw_dummy 1 0", s_DummyThrowButton, s_DummyThrowKey, s_DummyThrowModifier);
    Column.HSplitTop(MARGIN_EXTRA_SMALL, nullptr, &Column);

    //balancebot
    static CButtonContainer s_BalanceBotKeyButton;
    static int s_BalanceBotKey = 0;
    static int s_BalanceBotModifier = 0;
    DoKeyBindRow(Column, LINE_SIZE, "Balancebot key", "toggle mrx_balancebot 0 1", s_BalanceBotKeyButton, s_BalanceBotKey, s_BalanceBotModifier);

    //ReverseCopy
    static CButtonContainer s_ReverseCopyKeyButton;
    static int s_ReverseCopyKey = 0;
    static int s_ReverseCopyModifier = 0;
    DoKeyBindRow(Column, LINE_SIZE, "ReverseCopy key", "toggle mrx_reverse_dummy_copy_moves 0 1", s_ReverseCopyKeyButton, s_ReverseCopyKey, s_ReverseCopyModifier);
    s_SectionBoxes.back().h = Column.y - s_SectionBoxes.back().y;

    // --- Section: Auto Dummy Connect ---
    {
        CUIRect Bg;
        Column.HSplitTop(MARGIN_BETWEEN_SECTIONS, nullptr, &Column);
        Bg = Column;
        Bg.VMargin(MARGIN_SMALL, &Bg);
        s_SectionBoxes.push_back(Bg);

        CUIRect Header;
        Column.HSplitTop(HEADLINE_HEIGHT, &Header, &Column);
        Ui()->DoLabel(&Header, Localize("Auto Dummy Connect"), HEADLINE_FONT_SIZE, TEXTALIGN_ML);
        Column.HSplitTop(MARGIN_SMALL, nullptr, &Column);

        DoCheckBoxRow(Column, LINE_SIZE, "Auto Dummy Connect", g_Config.m_MRXAutoDummyConnect);
        Column.HSplitTop(MARGIN_EXTRA_SMALL, nullptr, &Column);
        DoSliderRow(Column, LINE_SIZE, "Interval (ms)", g_Config.m_MRXAutoDummyConnectCheckInterval, 500, 10000, "", "25l+25lr");
        // Округление к ближайшему шагу 50 мс
        {
            const int StepMs = 50;
            int &Val = g_Config.m_MRXAutoDummyConnectCheckInterval;
            // Приведение к кратности шага с округлением
            Val = std::clamp(((Val + StepMs / 2) / StepMs) * StepMs, 500, 10000);
        }
        
        s_SectionBoxes.back().h = Column.y - s_SectionBoxes.back().y;
    }

    // Завершение прокрутки - используем правильную формулу как в других вкладках
        const float MaxEndY = maximum(maximum(LeftView.y, MidView.y), RightView.y);
        CUIRect ScrollRegion;
        ScrollRegion.x = MainView.x;
        ScrollRegion.y = MainView.y;
        ScrollRegion.w = MainView.w;
        ScrollRegion.h = MaxEndY - MainView.y; // Правильная высота без дополнительных отступов
        s_ScrollRegion.AddRect(ScrollRegion);
        s_ScrollRegion.End();
}
#pragma endregion

#pragma region Visuals
void CMenus::RenderTabPageVisuals(CUIRect MainView)
{
    // Прокрутка и секции
    static CScrollRegion s_ScrollRegion;
    static std::vector<CUIRect> s_SectionBoxes;
    static vec2 s_PrevScrollOffset(0.0f, 0.0f);
    vec2 ScrollOffset(0.0f, 0.0f);
    CScrollRegionParams ScrollParams;
    ScrollParams.m_ScrollUnit = 120.0f;
    ScrollParams.m_Flags = CScrollRegionParams::FLAG_CONTENT_STATIC_WIDTH;
    ScrollParams.m_ScrollbarMargin = 5.0f;
    s_ScrollRegion.Begin(&MainView, &ScrollOffset, &ScrollParams);

    // Учёт прокрутки и поля под скроллбар
    MainView.y += ScrollOffset.y;
    MainView.VSplitRight(5.0f, &MainView, nullptr);
    MainView.VSplitLeft(2.0f, nullptr, &MainView);

    CUIRect LeftView, MidView, RightView;
    const float GapSize1 = MARGIN_BLOCKS;
    const float GapSize2 = MARGIN_BLOCKS * 1.3f;

    const float TotalGapWidth = GapSize1 + GapSize2;
    const float AvailableContentWidth = MainView.w - TotalGapWidth;

    const float ColumnWidth = AvailableContentWidth / 3.0f;

    MainView.VSplitLeft(ColumnWidth, &LeftView, &MainView);
    MainView.VSplitLeft(GapSize1, nullptr, &MainView);
    MainView.VSplitLeft(ColumnWidth, &MidView, &MainView);
    MainView.VSplitLeft(GapSize2, nullptr, &MainView);
    RightView = MainView;
    
    LeftView.VSplitLeft(4.0f, nullptr, &LeftView);
    MidView.VSplitLeft(4.0f, nullptr, &MidView);
    RightView.VSplitRight(MARGIN_SMALL, &RightView, nullptr);

    // Отрисовка фонов прошлых секций
    for(size_t i = 0; i < s_SectionBoxes.size(); ++i)
    {
        RenderBackgroundLines(s_SectionBoxes[i], (int)i);
        CUIRect Section = s_SectionBoxes[i];
        float Padding = MARGIN_BETWEEN_VIEWS * 0.6666f;
        Section.w += Padding;
        Section.h += Padding;
        Section.x -= Padding * 0.5f;
        Section.y -= Padding * 0.5f;
        Section.y -= s_PrevScrollOffset.y - ScrollOffset.y;
        float Shade = 0.0f;
        Section.Draw(ColorRGBA(Shade, Shade, Shade, 0.25f), IGraphics::CORNER_ALL, 10.0f);
    }
    s_PrevScrollOffset = ScrollOffset;
    s_SectionBoxes.clear();

    // ***** LeftView ***** //
    CUIRect Column = LeftView;
    CUIRect Header;

    // Секция: Visual Settings
    Column.HSplitTop(MARGIN, nullptr, &Column);
    { CUIRect Bg = Column; Bg.VMargin(MARGIN_SMALL, &Bg); s_SectionBoxes.push_back(Bg); }
    Column.HSplitTop(HEADLINE_HEIGHT, &Header, &Column);
    Ui()->DoLabel(&Header, Localize("Visual Settings"), HEADLINE_FONT_SIZE, TEXTALIGN_ML);
    Column.HSplitTop(MARGIN_SMALL, nullptr, &Column);

    DoCheckBoxRow(Column, LINE_SIZE, "Text output when sameX", g_Config.m_MRXOverlaySameX);
    DoCheckBoxRow(Column, LINE_SIZE, "Show Katan Frozen", g_Config.m_MRXFreezeKatana);
    DoCheckBoxRow(Column, LINE_SIZE, "Crosshair in the center", g_Config.m_MRXCenterEnable);
    DoCheckBoxRow(Column, LINE_SIZE, "Animated chat", g_Config.m_MRXChatAnim);
    //m_MRXFreezeKatana
    if(g_Config.m_MRXCenterEnable)
    {
    DoSliderRow(Column, LINE_SIZE, "Thickness Crosshair", g_Config.m_MRXCenterWidth, 1, 10, "", "30l+30lr");
    static CButtonContainer s_CenterLineColorReset;
    DoLine_ColorPicker(&s_CenterLineColorReset, COLOR_PICKER_LINE_SIZE, COLOR_PICKER_LABEL_SIZE, COLOR_PICKER_LINE_SPACING,
    &Column, Localize("Color Crosshair"), &g_Config.m_MRXCenterColor, ColorRGBA(1.0f, 0.16f, 0.16f, 1.0f), false, nullptr, false);
    }
    DoSliderRow(Column, LINE_SIZE, "Cursor Scale", g_Config.m_MRXCursorScale, 1, 500, "", "30l+30lr");
    s_SectionBoxes.back().h = Column.y - s_SectionBoxes.back().y;

    
        Column.HSplitTop(MARGIN_BETWEEN_SECTIONS, nullptr, &Column);
    { CUIRect Bg = Column; Bg.VMargin(MARGIN_SMALL, &Bg); s_SectionBoxes.push_back(Bg); }
    Column.HSplitTop(HEADLINE_HEIGHT, &Header, &Column);
    Ui()->DoLabel(&Header, Localize("BackgroundLines"), HEADLINE_FONT_SIZE, TEXTALIGN_ML);
    Column.HSplitTop(MARGIN_SMALL, nullptr, &Column);

    DoCheckBoxRow(Column, LINE_SIZE, "Enable background lines", g_Config.m_MRXBgLinesEnable);
    DoCheckBoxRow(Column, LINE_SIZE, "Draw dots", g_Config.m_MRXBgLinesDots);
    DoSliderRow(Column, LINE_SIZE, "Points", g_Config.m_MRXBgLinesPoints, 3, 128, "", "40l+40lr");
    DoSliderRow(Column, LINE_SIZE, "Distance", g_Config.m_MRXBgLinesDistance, 1, 500, "", "40l+40lr");
    DoSliderRow(Column, LINE_SIZE, "Speed", g_Config.m_MRXBgLinesSpeed, 1, 200, "", "40l+40lr");
    DoSliderRow(Column, LINE_SIZE, "Radius", g_Config.m_MRXBgLinesRadius, 1, 20, "", "40l+40lr");
    DoSliderRow(Column, LINE_SIZE, "Alpha", g_Config.m_MRXBgLinesAlpha, 0, 100, "%", "40l+40lr");

    static CButtonContainer s_BgLinesColorReset;
    DoLine_ColorPicker(&s_BgLinesColorReset, COLOR_PICKER_LINE_SIZE, COLOR_PICKER_LABEL_SIZE, COLOR_PICKER_LINE_SPACING,
    &Column, Localize("Color"), &g_Config.m_MRXBgLinesColor, ColorRGBA(1,1,1,1), false, nullptr, true);
    
    s_SectionBoxes.back().h = Column.y - s_SectionBoxes.back().y;

    // Секция: Custom Background Image
    Column.HSplitTop(MARGIN_BETWEEN_SECTIONS, nullptr, &Column);
    { CUIRect Bg = Column; Bg.VMargin(MARGIN_SMALL, &Bg); s_SectionBoxes.push_back(Bg); }
    Column.HSplitTop(HEADLINE_HEIGHT, &Header, &Column);
    Ui()->DoLabel(&Header, Localize("Custom Background"), HEADLINE_FONT_SIZE, TEXTALIGN_ML);
    Column.HSplitTop(MARGIN_SMALL, nullptr, &Column);

    // Enable checkbox
    DoCheckBoxRow(Column, LINE_SIZE, "Enable custom background", g_Config.m_MRXCustomBgEnable);
    Column.HSplitTop(MARGIN_EXTRA_SMALL, nullptr, &Column);

    // Scan backgrounds if needed
    if(m_BackgroundListNeedsUpdate)
    {
        m_vBackgroundList.clear();
        SMenuAssetScanUser User;
        User.m_pUser = this;
        User.m_LoadedFunc = []() {};
        Storage()->ListDirectory(IStorage::TYPE_ALL, "backgrounds", BackgroundScan, &User);
        std::sort(m_vBackgroundList.begin(), m_vBackgroundList.end());
        
        // Find current background index
        m_CurrentBackgroundIndex = -1;
        for(size_t i = 0; i < m_vBackgroundList.size(); i++)
        {
            if(str_comp(m_vBackgroundList[i].m_aPath, g_Config.m_MRXCustomBgPath) == 0)
            {
                m_CurrentBackgroundIndex = (int)i;
                break;
            }
        }
        m_BackgroundListNeedsUpdate = false;
    }

    // Background selector button (like ArrayList items)
    {
        CUIRect Row;
        Column.HSplitTop(LINE_SIZE, &Row, &Column);
        
        // Split row for button and folder icon
        CUIRect ButtonRow, FolderButton;
        Row.VSplitRight(30.0f, &ButtonRow, &FolderButton);
        FolderButton.VMargin(2.0f, &FolderButton);
        ButtonRow.VSplitRight(2.0f, &ButtonRow, nullptr);
        
        static CButtonContainer s_BtnBackgroundSelector;
        static bool s_BackgroundSelectorOpen = false;
        
        // Display current background name or "Select background"
        char aButtonText[128];
        if(m_CurrentBackgroundIndex >= 0 && m_CurrentBackgroundIndex < (int)m_vBackgroundList.size())
            str_format(aButtonText, sizeof(aButtonText), "%s", m_vBackgroundList[m_CurrentBackgroundIndex].m_aName);
        else
            str_copy(aButtonText, Localize("Select background"));
        
        if(DoButton_Menu(&s_BtnBackgroundSelector, aButtonText, s_BackgroundSelectorOpen ? 1 : 0, &ButtonRow, BUTTONFLAG_LEFT, nullptr, IGraphics::CORNER_ALL, 5.0f, 0.0f, ColorRGBA(1.0f, 1.0f, 1.0f, 0.22f)))
            s_BackgroundSelectorOpen ^= 1;
        
        // Folder open button
        static CButtonContainer s_FolderButton;
        // Используем DoButton_FontIcon для консистентного стиля с иконкой
        if(Ui()->DoButton_FontIcon(&s_FolderButton, "⚙️", 0, &FolderButton, BUTTONFLAG_LEFT))
        {
            char aBuf[IO_MAX_PATH_LENGTH];
            Storage()->GetCompletePath(IStorage::TYPE_SAVE, "backgrounds", aBuf, sizeof(aBuf));
            if(!Storage()->FolderExists("backgrounds", IStorage::TYPE_SAVE))
                Storage()->CreateFolder("backgrounds", IStorage::TYPE_SAVE);
            Client()->ViewFile(aBuf);
        }

        if(s_BackgroundSelectorOpen && !m_vBackgroundList.empty())
        {
            Column.HSplitTop(MARGIN_SMALL, nullptr, &Column);
            CUIRect Box = Column;
            const int BackgroundCount = (int)m_vBackgroundList.size();
            const float ItemHeight = 80.0f; // Height for thumbnail + name
            float NeededHeight = ItemHeight * BackgroundCount + 8.0f;
            Box.HSplitTop(NeededHeight, &Box, &Column);
            Box.Draw(ColorRGBA(0.0f, 0.0f, 0.0f, 0.15f), IGraphics::CORNER_ALL, 4.0f);
            Box.Margin(4.0f, &Box);

            for(int i = 0; i < BackgroundCount; i++)
            {
                CUIRect ItemRow;
                Box.HSplitTop(ItemHeight, &ItemRow, &Box);
                if(ItemRow.h <= 0.0f)
                    break;

                const auto &BgItem = m_vBackgroundList[i];
                bool IsSelected = (i == m_CurrentBackgroundIndex);

                // Draw item background
                ColorRGBA ItemColor = IsSelected ? ColorRGBA(0.3f, 0.5f, 0.8f, 0.3f) : ColorRGBA(0.0f, 0.0f, 0.0f, 0.0f);
                ItemRow.Draw(ItemColor, IGraphics::CORNER_ALL, 4.0f);

                // Draw thumbnail
                CUIRect Thumbnail, TextArea;
                ItemRow.VSplitLeft(ItemHeight - 8.0f, &Thumbnail, &TextArea);
                Thumbnail.Margin(4.0f, &Thumbnail);
                
                if(BgItem.m_RenderTexture.IsValid() && !BgItem.m_RenderTexture.IsNullTexture())
                {
                    Graphics()->TextureSet(BgItem.m_RenderTexture);
                    Graphics()->QuadsBegin();
                    Graphics()->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
                    IGraphics::CQuadItem QuadItem(Thumbnail.x, Thumbnail.y, Thumbnail.w, Thumbnail.h);
                    Graphics()->QuadsDrawTL(&QuadItem, 1);
                    Graphics()->QuadsEnd();
                }
                else
                {
                    Thumbnail.Draw(ColorRGBA(0.2f, 0.2f, 0.2f, 0.5f), IGraphics::CORNER_ALL, 4.0f);
                }

                // Draw name
                TextArea.VMargin(4.0f, &TextArea);
                Ui()->DoLabel(&TextArea, BgItem.m_aName, 12.0f, TEXTALIGN_ML);

                // Handle click
                static std::vector<CButtonContainer> s_BackgroundButtons;
                if(s_BackgroundButtons.size() < m_vBackgroundList.size())
                    s_BackgroundButtons.resize(m_vBackgroundList.size());
                
                if(DoButton_Menu(&s_BackgroundButtons[i], "", 0, &ItemRow, BUTTONFLAG_LEFT, nullptr, IGraphics::CORNER_ALL, 4.0f, 0.0f, ColorRGBA(1.0f, 1.0f, 1.0f, 0.0f)))
                {
                    m_CurrentBackgroundIndex = i;
                    str_copy(g_Config.m_MRXCustomBgPath, BgItem.m_aPath, sizeof(g_Config.m_MRXCustomBgPath));
                    m_NeedReloadCustomBg = true;
                    s_BackgroundSelectorOpen = false;
                }
            }

            Column.HSplitTop(MARGIN_SMALL, nullptr, &Column);
        }
        else if(s_BackgroundSelectorOpen && m_vBackgroundList.empty())
        {
            Column.HSplitTop(MARGIN_SMALL, nullptr, &Column);
            CUIRect InfoRow;
            Column.HSplitTop(LINE_SIZE * 2, &InfoRow, &Column);
            InfoRow.Draw(ColorRGBA(0.0f, 0.0f, 0.0f, 0.15f), IGraphics::CORNER_ALL, 4.0f);
            InfoRow.Margin(4.0f, &InfoRow);
            Ui()->DoLabel(&InfoRow, Localize("No backgrounds found in data/backgrounds/"), 10.0f, TEXTALIGN_MC);
            Column.HSplitTop(MARGIN_SMALL, nullptr, &Column);
        }
    }
    Column.HSplitTop(MARGIN_EXTRA_SMALL, nullptr, &Column);

    // Alpha slider
    DoSliderRow(Column, LINE_SIZE, "Opacity", g_Config.m_MRXCustomBgAlpha, 0, 100, "%", "40l+40lr");
    Column.HSplitTop(MARGIN_EXTRA_SMALL, nullptr, &Column);

    // Blur slider
    DoSliderRow(Column, LINE_SIZE, "Blur", g_Config.m_MRXCustomBgBlur, 0, 100, "%", "40l+40lr");
    Column.HSplitTop(MARGIN_EXTRA_SMALL, nullptr, &Column);

    // Center-focused blur slider
    DoSliderRow(Column, LINE_SIZE, "Center Focus Blur", g_Config.m_MRXCustomBgBlurCenter, 0, 100, "%", "40l+40lr");
    Column.HSplitTop(MARGIN_EXTRA_SMALL, nullptr, &Column);

    s_SectionBoxes.back().h = Column.y - s_SectionBoxes.back().y;
    // Запомним нижнюю границу левой колонки для расчёта области прокрутки
    const float LeftEndY = Column.y;


    // ***** MidView ***** //
    Column = MidView;

    Column.HSplitTop(MARGIN, nullptr, &Column);
{ CUIRect Bg = Column; Bg.VMargin(MARGIN_SMALL, &Bg); s_SectionBoxes.push_back(Bg); }

Ui()->DoLabel_AutoLineSize(Localize("Preview"), 14.0f, TEXTALIGN_ML, &Column, LINE_SIZE);
Column.HSplitTop(MARGIN_SMALL, nullptr, &Column);

CUIRect PreviewButtonRow;
Column.HSplitTop(LINE_SIZE, &PreviewButtonRow, &Column);
static CButtonContainer s_OpenPreviewButton;
if(DoButton_Menu(&s_OpenPreviewButton, Localize("Open overlay preview"), 0, &PreviewButtonRow, BUTTONFLAG_LEFT, nullptr, IGraphics::CORNER_ALL, 5.0f, 0.0f, ColorRGBA(1.0f, 1.0f, 1.0f, 0.22f)))
{
    m_MRXOverlayPreviewOpen = true;
}

s_SectionBoxes.back().h = Column.y - s_SectionBoxes.back().y;

    // Секция настроек Predicted Trajectory в средней колонке (MidView)
        Column.HSplitTop(MARGIN_BETWEEN_SECTIONS, nullptr, &Column);
        { CUIRect Bg = Column; Bg.VMargin(MARGIN_SMALL, &Bg); s_SectionBoxes.push_back(Bg); }
        Column.HSplitTop(HEADLINE_HEIGHT, &Header, &Column);
        Ui()->DoLabel(&Header, Localize("Predicted Trajectory"), HEADLINE_FONT_SIZE, TEXTALIGN_ML);
        Column.HSplitTop(MARGIN_SMALL, nullptr, &Column);

        // Включение фичи
        DoCheckBoxRow(Column, LINE_SIZE, Localize("Enable"), g_Config.m_MRXPredictTraj);

        // Количество тиков
        DoSliderRow(Column, LINE_SIZE, Localize("Ticks"), g_Config.m_MRXPredictTrajTicks, 0, 200, "", "40l+30lr");

        Column.HSplitTop(MARGIN_EXTRA_SMALL, nullptr, &Column);

        // Режим линии: Solid/Dashed
        {
            static const char *s_ModeNames[] = {
                Localize("Solid"),
                Localize("Dashed")
            };
            static CUi::SDropDownState s_ModeDropDown;
            static CScrollRegion s_ModeScrollRegion;
            s_ModeDropDown.m_SelectionPopupContext.m_pScrollRegion = &s_ModeScrollRegion;
            g_Config.m_MRXPredictTrajMode = DoLabeledDropDownRow(Column, LINE_SIZE, Localize("Mode:"), g_Config.m_MRXPredictTrajMode, s_ModeNames, std::size(s_ModeNames), s_ModeDropDown);
        }

        Column.HSplitTop(MARGIN_EXTRA_SMALL, nullptr, &Column);

        // Цвета линии
        {
            static CButtonContainer s_ColorBaseReset;
            DoLine_ColorPicker(&s_ColorBaseReset, COLOR_PICKER_LINE_SIZE, COLOR_PICKER_LABEL_SIZE, COLOR_PICKER_LINE_SPACING,
                &Column, Localize("Base color"), &g_Config.m_MRXPredictTrajColor, ColorRGBA(0.18f, 0.56f, 0.85f, 1.0f), false, nullptr, false);

            static CButtonContainer s_ColorFreezeReset;
            DoLine_ColorPicker(&s_ColorFreezeReset, COLOR_PICKER_LINE_SIZE, COLOR_PICKER_LABEL_SIZE, COLOR_PICKER_LINE_SPACING,
                &Column, Localize("Freeze color"), &g_Config.m_MRXPredictTrajColorFreeze, ColorRGBA(1.0f, 0.16f, 0.16f, 1.0f), false, nullptr, false);
        }

        s_SectionBoxes.back().h = Column.y - s_SectionBoxes.back().y;
    

    
    
    
    // Запомним нижнюю границу средней колонки
    const float MidEndY = Column.y;

    // ***** RightView ***** //
    Column = RightView;

    // Секция: MRX Overlay Text
    Column.HSplitTop(MARGIN, nullptr, &Column);
    { CUIRect Bg = Column; Bg.VMargin(MARGIN_SMALL, &Bg); s_SectionBoxes.push_back(Bg); }
    Column.HSplitTop(HEADLINE_HEIGHT, &Header, &Column);
    TextRender()->TextColor(1.0f, 1.0f, 1.0f, 1.0f);
    Ui()->DoLabel(&Header, Localize("MRX Overlay Text"), HEADLINE_FONT_SIZE, TEXTALIGN_ML);
    TextRender()->TextColor(1.0f, 1.0f, 1.0f, 1.0f);
    Column.HSplitTop(MARGIN_SMALL, nullptr, &Column);

    // Контент раздела

    // Включение и общие параметры
    DoCheckBoxRow(Column, LINE_SIZE, "Enable Overlay Text", g_Config.m_MRXOverlayText);
    Column.HSplitTop(MARGIN_SMALL, nullptr, &Column);
    DoSliderRow(Column, LINE_SIZE, "Font size", g_Config.m_MRXOverlayTextSize, 6, 72, "", "50l+50lr");
    Column.HSplitTop(MARGIN_SMALL, nullptr, &Column);

    // Глобальные цветовые настройки убраны — цвета/фон задаются для каждого текста отдельно ниже

    // Поля ввода текстов (компактно, с раскрывающимися настройками)
    Ui()->DoLabel_AutoLineSize(Localize("Texts"), 14.0f, TEXTALIGN_ML, &Column, LINE_SIZE);
    Column.HSplitTop(MARGIN_SMALL, nullptr, &Column);

    struct STextSlotUI
    {
        const char *m_pHeader;
        char *m_pCfgText;
        int *m_pX;
        int *m_pY;
        CLineInputBuffered<256> *m_pInput;
        CButtonContainer *m_pHeaderBtn;
        bool *m_pExpanded;
        unsigned int *m_pTextColor; // HSLA packed
    };

    static CLineInputBuffered<256> s_TextInputs[5];
    static CButtonContainer s_HeaderBtns[5];
    static bool s_Expanded[5] = {false, false, false, false, false};

    STextSlotUI aSlots[5] = {
        {" Text 0", g_Config.m_MRXText, &g_Config.m_MRXTextX, &g_Config.m_MRXTextY, &s_TextInputs[0], &s_HeaderBtns[0], &s_Expanded[0], &g_Config.m_MRXTextColor},
        {" Text 1", g_Config.m_MRXText1, &g_Config.m_MRXText1X, &g_Config.m_MRXText1Y, &s_TextInputs[1], &s_HeaderBtns[1], &s_Expanded[1], &g_Config.m_MRXText1Color},
        {" Text 2", g_Config.m_MRXText2, &g_Config.m_MRXText2X, &g_Config.m_MRXText2Y, &s_TextInputs[2], &s_HeaderBtns[2], &s_Expanded[2], &g_Config.m_MRXText2Color},
        {" Text 3", g_Config.m_MRXText3, &g_Config.m_MRXText3X, &g_Config.m_MRXText3Y, &s_TextInputs[3], &s_HeaderBtns[3], &s_Expanded[3], &g_Config.m_MRXText3Color},
        {" Text 4", g_Config.m_MRXText4, &g_Config.m_MRXText4X, &g_Config.m_MRXText4Y, &s_TextInputs[4], &s_HeaderBtns[4], &s_Expanded[4], &g_Config.m_MRXText4Color},
    };

    for(int i = 0; i < 5; i++)
    {
        if(i > 0)
            Column.HSplitTop(MARGIN_SMALL, nullptr, &Column);
        CUIRect Row; Column.HSplitTop(LINE_SIZE, &Row, &Column);
        // Кнопка-заголовок раздела (раскрыть/свернуть) — капсульный стиль
        char aHdr[64];
        str_format(aHdr, sizeof(aHdr), "%s%s", (*aSlots[i].m_pExpanded ? " ▾ " : " ▸ "), Localize(aSlots[i].m_pHeader));
        {
            // фон-капсула без реакции на hover
            CUIRect Pill = Row;
            Pill.Margin(2.0f, &Pill);
            const float Rounding = Pill.h * 0.5f;
            Pill.Draw(ColorRGBA(1.0f, 1.0f, 1.0f, 0.22f), IGraphics::CORNER_ALL, Rounding);

            // текст слева
            CUIRect TextRect = Pill;
            Ui()->DoLabel(&TextRect, aHdr, 12.0f, TEXTALIGN_ML);

            // логика клика без стандартного фона
            if(Ui()->DoButtonLogic(aSlots[i].m_pHeaderBtn, 0, &Row, BUTTONFLAG_LEFT))
                *aSlots[i].m_pExpanded = !*aSlots[i].m_pExpanded;
        }

        if(*aSlots[i].m_pExpanded)
        {
            // Ввод текста
            CUIRect Edit; Column.HSplitTop(LINE_SIZE, &Edit, &Column);
            aSlots[i].m_pInput->SetBuffer(aSlots[i].m_pCfgText, 256);
            Ui()->DoClearableEditBox(aSlots[i].m_pInput, &Edit, 12.0f);
            Column.HSplitTop(MARGIN_SMALL, nullptr, &Column);

            // Текущая позиция (перетаскивайте в превью)
            {
                CUIRect Row; Column.HSplitTop(LINE_SIZE, &Row, &Column);
                char aBuf[64];
                str_format(aBuf, sizeof(aBuf), "Position: %d%%, %d%%", *aSlots[i].m_pX, *aSlots[i].m_pY);
                Ui()->DoLabel(&Row, aBuf, 12.0f, TEXTALIGN_ML);
                Column.HSplitTop(MARGIN_SMALL, nullptr, &Column);
            }

            // Цвет текста
            {
                static CButtonContainer s_TextColReset[5];
                // Disable alpha editing for overlay text colors (no alpha slider)
                DoLine_ColorPicker(&s_TextColReset[i], COLOR_PICKER_LINE_SIZE, COLOR_PICKER_LABEL_SIZE, COLOR_PICKER_LINE_SPACING, &Column, Localize(""), aSlots[i].m_pTextColor, ColorRGBA(1,1,1,1), false, nullptr, false);
                Column.HSplitTop(MARGIN_SMALL, nullptr, &Column);
            }
        }
    }

    // небольшой отступ перед превью
    Column.HSplitTop(MARGIN_SMALL, nullptr, &Column);

    s_SectionBoxes.back().h = Column.y - s_SectionBoxes.back().y;
    
    Column.HSplitTop(MARGIN_BETWEEN_SECTIONS, nullptr, &Column);
    { CUIRect Bg = Column; Bg.VMargin(MARGIN_SMALL, &Bg); s_SectionBoxes.push_back(Bg); }
    Column.HSplitTop(HEADLINE_HEIGHT, &Header, &Column);
    Ui()->DoLabel(&Header, Localize("ArrayList"), HEADLINE_FONT_SIZE, TEXTALIGN_ML);
    Column.HSplitTop(MARGIN_SMALL, nullptr, &Column);

    DoCheckBoxRow(Column, LINE_SIZE, "ArrayList", g_Config.m_MRXArrayList);
    DoCheckBoxRow(Column, LINE_SIZE, "ArrayList Gradient", g_Config.m_MRXArrayListGradient);
    DoCheckBoxRow(Column, LINE_SIZE, "ArrayList Line", g_Config.m_MRXArrayListLine);

    // ArrayList items inline toggle list (no dropdown to avoid crashes)
    {
        CUIRect Row;
        Column.HSplitTop(LINE_SIZE, &Row, &Column);
        static CButtonContainer s_BtnArrayListItems;
        static bool s_ArrayListItemsOpen = false;
        if(DoButton_Menu(&s_BtnArrayListItems, Localize("ArrayList items"), s_ArrayListItemsOpen ? 1 : 0, &Row, BUTTONFLAG_LEFT, nullptr, IGraphics::CORNER_ALL, 5.0f, 0.0f, ColorRGBA(1.0f, 1.0f, 1.0f, 0.22f)))
            s_ArrayListItemsOpen ^= 1;

        if(s_ArrayListItemsOpen)
        {
            Column.HSplitTop(MARGIN_SMALL, nullptr, &Column);
            CUIRect Box = Column;
            constexpr int FeatureCount = (int)CArrayList::EFeature::COUNT;
            float NeededHeight = LINE_SIZE * FeatureCount + 8.0f; // запас под отступы
            Box.HSplitTop(NeededHeight, &Box, &Column);
            Box.Draw(ColorRGBA(0.0f, 0.0f, 0.0f, 0.15f), IGraphics::CORNER_ALL, 4.0f);
            Box.Margin(4.0f, &Box);

            auto AddVisRow = [&](const char *pName, CArrayList::EFeature Id, CButtonContainer &Btn)
            {
                CUIRect R;
                Box.HSplitTop(LINE_SIZE, &R, &Box);
                if(R.h <= 0.0f)
                    return;

                bool Visible = GameClient()->m_ArrayList.GetFeatureVisible(Id);

                // Иконка ON/OFF
                ITextRender *pTR = Ui()->TextRender();
                unsigned PrevFlags = pTR->GetRenderFlags();
                pTR->SetFontPreset(EFontPreset::ICON_FONT);
                const char *pIcon = Visible ? FontIcons::FONT_ICON_TOGGLE_ON : FontIcons::FONT_ICON_TOGGLE_OFF;
                const float IconSize = 14.0f;
                if(Visible)
                    pTR->TextColor(1.0f, 1.0f, 1.0f, 1.0f);
                else
                    pTR->TextColor(0.6f, 0.6f, 0.6f, 0.6f);
                pTR->Text(R.x, R.y + (R.h - IconSize) * 0.5f, IconSize, pIcon);
                pTR->SetFontPreset(EFontPreset::DEFAULT_FONT);
                pTR->SetRenderFlags(PrevFlags);
                pTR->TextColor(1.0f, 1.0f, 1.0f, 1.0f);

                CUIRect LabelRect = R;
                LabelRect.x += 20.0f;
                Ui()->DoLabel(&LabelRect, pName, 12.0f, TEXTALIGN_ML);

                if(DoButton_Menu(&Btn, "", 0, &R, BUTTONFLAG_LEFT, nullptr, IGraphics::CORNER_ALL, 4.0f, 0.0f, ColorRGBA(1.0f, 1.0f, 1.0f, 0.0f)))
                    GameClient()->m_ArrayList.SetFeatureVisible(Id, !Visible);
            };

            static CButtonContainer s_F0, s_F1, s_F2, s_F3, s_F4, s_F5, s_F6, s_F7, s_F8, s_F9, s_F10, s_F11, s_F12, s_F13, s_F14, s_F15, s_F16, s_F17;
            AddVisRow("Move Fly Bot",       CArrayList::EFeature::MoveFlyBot,       s_F0);
            AddVisRow("Balance Bot",        CArrayList::EFeature::BalanceBot,       s_F1);
            AddVisRow("PseudoAled",         CArrayList::EFeature::PseudoAled,       s_F2);
            AddVisRow("AutoAled",           CArrayList::EFeature::AutoAled,         s_F3);
            AddVisRow("AvoidFreeze",        CArrayList::EFeature::AvoidFreeze,      s_F4);
            AddVisRow("AutoPseudo",         CArrayList::EFeature::AutoPseudo,       s_F5);
            AddVisRow("PixelWalk",          CArrayList::EFeature::PixelWalk,        s_F6);
            AddVisRow("Advanced DeepFly",   CArrayList::EFeature::AdvancedDeepFly,  s_F7);
            AddVisRow("AutoHookDummy",      CArrayList::EFeature::AutoHookDummy,    s_F8);
            AddVisRow("SlowWalk",           CArrayList::EFeature::SlowWalk,         s_F9);
            AddVisRow("TripleHookHolder",   CArrayList::EFeature::TripleHookHolder, s_F10);
            AddVisRow("AutoTripleFly",      CArrayList::EFeature::AutoTripleFly,    s_F11);
            AddVisRow("DummyHookCursor",    CArrayList::EFeature::DummyHookCursor,  s_F12);
            AddVisRow("Aimbot",             CArrayList::EFeature::Aimbot,           s_F13);
            AddVisRow("AutoHookFly",        CArrayList::EFeature::AutoHookFly,      s_F14);
            AddVisRow("DummyThrow",         CArrayList::EFeature::DummyThrow,       s_F15);
            AddVisRow("DuoPseudo",             CArrayList::EFeature::DuoPseudo,           s_F16);
            AddVisRow("ReverseCopy",        CArrayList::EFeature::ReverseCopy,         s_F17);

            Column.HSplitTop(MARGIN_SMALL, nullptr, &Column);
        }
    }

    s_SectionBoxes.back().h = Column.y - s_SectionBoxes.back().y;

    const float RightEndY = Column.y; // нижняя граница правой колонки

    // Завершение прокрутки - определяем максимальную высоту контента
    const float MaxEndY = maximum(maximum(LeftEndY, MidEndY), RightEndY);
    CUIRect ScrollRegion;
    ScrollRegion.x = MainView.x;
    ScrollRegion.y = MaxEndY + MARGIN * 2.0f;
    ScrollRegion.w = MainView.w;
    ScrollRegion.h = 0.0f;
    s_ScrollRegion.AddRect(ScrollRegion);
    s_ScrollRegion.End();
}
#pragma endregion

#pragma region FlyHelper
void CMenus::RenderTabPageFlyHelper(CUIRect MainView)
{
    // Прокрутка и секции
    static CScrollRegion s_ScrollRegion;
    static std::vector<CUIRect> s_SectionBoxes;
    static vec2 s_PrevScrollOffset(0.0f, 0.0f);
    vec2 ScrollOffset(0.0f, 0.0f);
    CScrollRegionParams ScrollParams;
    ScrollParams.m_ScrollUnit = 120.0f;
    ScrollParams.m_Flags = CScrollRegionParams::FLAG_CONTENT_STATIC_WIDTH;
    ScrollParams.m_ScrollbarMargin = 5.0f;
    s_ScrollRegion.Begin(&MainView, &ScrollOffset, &ScrollParams);

    MainView.y += ScrollOffset.y;
    MainView.VSplitRight(5.0f, &MainView, nullptr);
    MainView.VSplitLeft(2.0f, nullptr, &MainView);

    // Адаптивное разделение на три колонки с минимальными размерами
    CUIRect LeftView, MidView, RightView;
    const float GapSize1 = MARGIN_BLOCKS;
    const float GapSize2 = MARGIN_BLOCKS * 1.3f;

    const float TotalGapWidth = GapSize1 + GapSize2;
    const float AvailableContentWidth = MainView.w - TotalGapWidth;

    const float ColumnWidth = AvailableContentWidth / 3.0f;

    MainView.VSplitLeft(ColumnWidth, &LeftView, &MainView);
    MainView.VSplitLeft(GapSize1, nullptr, &MainView);
    MainView.VSplitLeft(ColumnWidth, &MidView, &MainView);
    MainView.VSplitLeft(GapSize2, nullptr, &MainView);
    RightView = MainView;
    
    LeftView.VSplitLeft(4.0f, nullptr, &LeftView);
    MidView.VSplitLeft(4.0f, nullptr, &MidView);
    RightView.VSplitRight(MARGIN_SMALL, &RightView, nullptr);

    // Фоны секций
    for(size_t i = 0; i < s_SectionBoxes.size(); ++i)
    {
        RenderBackgroundLines(s_SectionBoxes[i], (int)i);
        CUIRect Section = s_SectionBoxes[i];
        float Padding = MARGIN_BETWEEN_VIEWS * 0.6666f;
        Section.w += Padding;
        Section.h += Padding;
        Section.x -= Padding * 0.5f;
        Section.y -= Padding * 0.5f;
        Section.y -= s_PrevScrollOffset.y - ScrollOffset.y;
        float Shade = 0.0f;
        Section.Draw(ColorRGBA(Shade, Shade, Shade, 0.25f), IGraphics::CORNER_ALL, 10.0f);
    }
    s_PrevScrollOffset = ScrollOffset;
    s_SectionBoxes.clear();

    // ***** LeftView ***** //
    CUIRect Column = LeftView;
    CUIRect Header, Button;

    // Секция: Autopseudo
    Column.HSplitTop(MARGIN, nullptr, &Column);
    { CUIRect Bg = Column; Bg.VMargin(MARGIN_SMALL, &Bg); s_SectionBoxes.push_back(Bg); }
    Column.HSplitTop(HEADLINE_HEIGHT, &Header, &Column);
    Ui()->DoLabel(&Header, Localize("FlyHelpers"), HEADLINE_FONT_SIZE, TEXTALIGN_ML);
    Column.HSplitTop(MARGIN_SMALL, nullptr, &Column);

    bool bAllEnabled = g_Config.m_MRXDummyPseudo && g_Config.m_MRXCopy_nomove && g_Config.m_MRXCopy;
    DoCheckBoxRow(Column, LINE_SIZE, "Autopseudo", g_Config.m_MRXDummyPseudo);
    DoSliderRow(Column, LINE_SIZE, "TimeTargetHit", g_Config.m_MRXTargetHit, 1, 15, "ticks");
    DoCheckBoxRow(Column, LINE_SIZE, "HammerMove", g_Config.m_MRXCopy);
    DoGearToggleRow(Column, LINE_SIZE, "AutoTriplefly Settings", g_Config.m_MRXShowAutoTripleFly);
    DoGearToggleRow(Column, LINE_SIZE, "Movefly Settings", g_Config.m_MRXShowMoveFlyBot);
    DoGearToggleRow(Column, LINE_SIZE, "Hookfly Settings", g_Config.m_MRXShowHookFly);
    
    s_SectionBoxes.back().h = Column.y - s_SectionBoxes.back().y;
    const float LeftEndY = Column.y;
    
// ***** MidView ***** //
Column = MidView;
    
if(g_Config.m_MRXShowMoveFlyBot)
{
    // Секция: Movefly Settings
    Column.HSplitTop(MARGIN, nullptr, &Column);
    { CUIRect Bg = Column; Bg.VMargin(MARGIN_SMALL, &Bg); s_SectionBoxes.push_back(Bg); }
    Column.HSplitTop(HEADLINE_HEIGHT, &Header, &Column);
    Ui()->DoLabel(&Header, Localize("Movefly Settings"), HEADLINE_FONT_SIZE, TEXTALIGN_ML);
    Column.HSplitTop(MARGIN_SMALL, nullptr, &Column);

    DoCheckBoxRow(Column, LINE_SIZE, "Movefly direction", g_Config.m_MRXMoveflyBotDirect);
    // Movefly Distance (helper)
    DoSliderRow(Column, LINE_SIZE, "Movefly Distance", g_Config.m_MRXMoveflyBotDistance, 0, 50, "units");

    Column.HSplitTop(MARGIN_SMALL, nullptr, &Column);

    // Movefly Key (helper)
    static CButtonContainer s_MoveflyKeyButton;
    static int s_MoveflyKey = 0;
    static int s_MoveflyModifier = 0;
    DoKeyBindRow(Column, LINE_SIZE, "Movefly key", "toggle mrx_movefly 0 1; dummy_reset", s_MoveflyKeyButton, s_MoveflyKey, s_MoveflyModifier);

    s_SectionBoxes.back().h = Column.y - s_SectionBoxes.back().y;
}

if(g_Config.m_MRXShowAutoTripleFly)
{
// Секция: AutoTripleFly
Column.HSplitTop(MARGIN_BETWEEN_SECTIONS, nullptr, &Column);
{ CUIRect Bg = Column; Bg.VMargin(MARGIN_SMALL, &Bg); s_SectionBoxes.push_back(Bg); }
Column.HSplitTop(HEADLINE_HEIGHT, &Header, &Column);
Ui()->DoLabel(&Header, Localize("AutoTripleFly"), HEADLINE_FONT_SIZE, TEXTALIGN_ML);
Column.HSplitTop(MARGIN_SMALL, nullptr, &Column);

    DoCheckBoxRow(Column, LINE_SIZE, "AutoTripleFly", g_Config.m_MRXAutotriplefly);
    CUIRect Row, Label;
    Column.HSplitTop(MARGIN_SMALL, nullptr, &Column);

    static const char *s_aModeItems[] = {
        Localize("AutoTripleControl"),
        Localize("AutoTripleDefaultCopy")};
    static CUi::SDropDownState s_ModeDropDownState;
    static CScrollRegion s_ModeDropDownScroll;
    s_ModeDropDownState.m_SelectionPopupContext.m_pScrollRegion = &s_ModeDropDownScroll;
    g_Config.m_MRXAutotripleflyMode = DoLabeledDropDownRow(Column, LINE_SIZE, Localize("AutoTripleFly Mode:"), g_Config.m_MRXAutotripleflyMode, s_aModeItems, std::size(s_aModeItems), s_ModeDropDownState);
    
    Column.HSplitTop(MARGIN_SMALL, nullptr, &Column);

    static const char *s_aDistItems[] = {
        Localize("fixed distance after hook hit"),
        Localize("Set distance")};
    static CUi::SDropDownState s_DistDropDownState;
    static CScrollRegion s_DistDropDownScroll;
    s_DistDropDownState.m_SelectionPopupContext.m_pScrollRegion = &s_DistDropDownScroll;
    g_Config.m_MRXAutotripleflyManual = DoLabeledDropDownRow(Column, LINE_SIZE, Localize("AutoTripleFly Distance:"), g_Config.m_MRXAutotripleflyManual, s_aDistItems, std::size(s_aDistItems), s_DistDropDownState);
    
    if(g_Config.m_MRXAutotripleflyManual == 1)
    {
        DoSliderRow(Column, LINE_SIZE, "AutoTripleFly Distance", g_Config.m_MRXAutotripleflyDistance, 10, 380, "units");
    }
    
    Column.HSplitTop(MARGIN_SMALL, nullptr, &Column);

    // Клавиша TripleThrow
    static CButtonContainer s_TripleThrowKeyBtn;
    static int s_TripleThrowKey = 0, s_TripleThrowMod = 0;
    DoKeyBindRow(Column, LINE_SIZE, "TripleThrow key", "+toggle mrx_autotriplefly_throw 1 0", s_TripleThrowKeyBtn, s_TripleThrowKey, s_TripleThrowMod);
    Column.HSplitTop(MARGIN_EXTRA_SMALL, nullptr, &Column);


    // Клавиша TripleThrowBoth
    static CButtonContainer s_TripleThrowBothKeyBtn;
    static int s_TripleThrowBothKey = 0, s_TripleThrowBothMod = 0;
    DoKeyBindRow(Column, LINE_SIZE, "TripleThrowBoth key", "+toggle mrx_autotriplefly_throw_both 1 0", s_TripleThrowBothKeyBtn, s_TripleThrowBothKey, s_TripleThrowBothMod);
    Column.HSplitTop(MARGIN_EXTRA_SMALL, nullptr, &Column);


    // Кулдаун
    DoSliderRow(Column, LINE_SIZE, "Throw hook Cooldown", g_Config.m_MRXAutotripleflyCooldown, 0, 200, "ticks");
    Column.HSplitTop(MARGIN_EXTRA_SMALL, nullptr, &Column);




    // Клавиша TripleHookHolder
    static CButtonContainer s_TripleHookHolderKeyBtn;
    static int s_TripleHookHolderKey = 0, s_TripleHookHolderMod = 0;
    DoKeyBindRow(Column, LINE_SIZE, "TripleHookHolder key", "toggle mrx_triplehookholder 0 1", s_TripleHookHolderKeyBtn, s_TripleHookHolderKey, s_TripleHookHolderMod);

    // Кейбиндеры для TripleFly режимов (Fun / Normal)
    Column.HSplitTop(MARGIN_SMALL, nullptr, &Column);
    static CButtonContainer s_TripleFlyFunKeyBtn;
    static int s_TripleFlyFunKey = 0, s_TripleFlyFunMod = 0;
    DoKeyBindRow(Column, LINE_SIZE, "TripleFlyFun key", "mrx_triplefly_fun", s_TripleFlyFunKeyBtn, s_TripleFlyFunKey, s_TripleFlyFunMod);

    Column.HSplitTop(MARGIN_EXTRA_SMALL, nullptr, &Column);
    static CButtonContainer s_TripleFlyNormalKeyBtn;
    static int s_TripleFlyNormalKey = 0, s_TripleFlyNormalMod = 0;
    DoKeyBindRow(Column, LINE_SIZE, "TripleFlyNormal key", "mrx_triplefly_normal", s_TripleFlyNormalKeyBtn, s_TripleFlyNormalKey, s_TripleFlyNormalMod);


s_SectionBoxes.back().h = Column.y - s_SectionBoxes.back().y;
}

if(g_Config.m_MRXShowHookFly)
{
// Секция: HookFly
Column.HSplitTop(MARGIN_BETWEEN_SECTIONS, nullptr, &Column);
{ CUIRect Bg = Column; Bg.VMargin(MARGIN_SMALL, &Bg); s_SectionBoxes.push_back(Bg); }
Column.HSplitTop(HEADLINE_HEIGHT, &Header, &Column);
Ui()->DoLabel(&Header, Localize("HookFly"), HEADLINE_FONT_SIZE, TEXTALIGN_ML);
Column.HSplitTop(MARGIN_SMALL, nullptr, &Column);

DoCheckBoxRow(Column, LINE_SIZE, "Auto HookFly ", g_Config.m_MRXHookFlyAuto);
DoSliderRow(Column, LINE_SIZE, "AutoHookFly Distance", g_Config.m_MRXHookFlyAutoDistance, 0, 250, "units");

Column.HSplitTop(MARGIN_SMALL, nullptr, &Column);

// Ключи для HookFly
static CButtonContainer s_HookFlyAutoKeyBtn;
static int s_HookFlyAutoKey = 0, s_HookFlyAutoMod = 0;
DoKeyBindRow(Column, LINE_SIZE, "AutoHookFly key", "toggle mrx_hookfly_auto 0 1", s_HookFlyAutoKeyBtn, s_HookFlyAutoKey, s_HookFlyAutoMod);

Column.HSplitTop(MARGIN_EXTRA_SMALL, nullptr, &Column);
static CButtonContainer s_HookFlyManualKeyBtn;
static int s_HookFlyManualKey = 0, s_HookFlyManualMod = 0;
DoKeyBindRow(Column, LINE_SIZE, "ManualHookFly key", "+toggle mrx_hookfly_manual 1 0", s_HookFlyManualKeyBtn, s_HookFlyManualKey, s_HookFlyManualMod);

s_SectionBoxes.back().h = Column.y - s_SectionBoxes.back().y;
}

const float MidEndY = Column.y;



    // ***** RightView ***** //
    Column = RightView;

    // Секция Binds
    Column.HSplitTop(MARGIN, nullptr, &Column);
    { CUIRect Bg = Column; Bg.VMargin(MARGIN_SMALL, &Bg); s_SectionBoxes.push_back(Bg); }
    Column.HSplitTop(HEADLINE_HEIGHT, &Header, &Column);
    Ui()->DoLabel(&Header, Localize("Binds"), HEADLINE_FONT_SIZE, TEXTALIGN_ML);
    Column.HSplitTop(MARGIN_SMALL, nullptr, &Column);

        static CButtonContainer s_MoveflyKeyButton2;
        static int s_MoveflyKey2 = 0;
        static int s_MoveflyModifier2 = 0;
        DoKeyBindRow(Column, LINE_SIZE, "Movefly key", "toggle mrx_movefly 0 1; dummy_reset", s_MoveflyKeyButton2, s_MoveflyKey2, s_MoveflyModifier2);
        
        static CButtonContainer s_HookFlyAutoKeyBtn;
        static int s_HookFlyAutoKey = 0, s_HookFlyAutoMod = 0;
        DoKeyBindRow(Column, LINE_SIZE, "AutoHookFly key", "toggle mrx_hookfly_auto 0 1", s_HookFlyAutoKeyBtn, s_HookFlyAutoKey, s_HookFlyAutoMod);

        Column.HSplitTop(MARGIN_EXTRA_SMALL, nullptr, &Column);
        static CButtonContainer s_HookFlyManualKeyBtn;
        static int s_HookFlyManualKey = 0, s_HookFlyManualMod = 0;
        DoKeyBindRow(Column, LINE_SIZE, "ManualHookFly key", "+toggle mrx_hookfly_manual 0 1", s_HookFlyManualKeyBtn, s_HookFlyManualKey, s_HookFlyManualMod);

       s_SectionBoxes.back().h = Column.y - s_SectionBoxes.back().y;
       const float RightEndY = Column.y;

    

    // Завершение прокрутки
    CUIRect ScrollRegion;
    ScrollRegion.x = MainView.x;
    ScrollRegion.y = maximum(maximum(LeftEndY, MidEndY), RightEndY) + MARGIN * 2.0f;
    ScrollRegion.w = MainView.w;
    ScrollRegion.h = 0.0f;
    s_ScrollRegion.AddRect(ScrollRegion);
    s_ScrollRegion.End();
}
#pragma endregion FlyHelper 

#pragma region WheelBinds
void CMenus::RenderTabPageWheelBinds(CUIRect MainView)
{

    CUIRect LeftView, RightView, Label, Button;
    MainView.VSplitLeft(MainView.w / 2.1f, &LeftView, &RightView);

    const float Radius = minimum(RightView.w, RightView.h) / 2.0f;
    vec2 Pos{RightView.x + RightView.w / 2.0f, RightView.y + RightView.h / 2.0f};
    // Draw Circle
    Graphics()->TextureClear();
    Graphics()->QuadsBegin();
    Graphics()->SetColor(0.0f, 0.0f, 0.0f, 0.3f);
    Graphics()->DrawCircle(Pos.x, Pos.y, Radius, 64);
    Graphics()->QuadsEnd();

    static char s_aBindName[MRX_BINDWHEEL_MAX_NAME];
    static char s_aBindCommand[MRX_BINDWHEEL_MAX_CMD];
    static int s_SelectedBindIndex = -1;
    int HoveringIndex = -1;

    float MouseDist = distance(Pos, Ui()->MousePos());
    if(MouseDist < Radius && MouseDist > Radius * 0.25f)
    {
        int SegmentCount = GameClient()->m_BindWheelMRX.m_vBinds.size();
        float SegmentAngle = 2.0f * pi / SegmentCount;
        float HoveringAngle = angle(Ui()->MousePos() - Pos) + SegmentAngle / 2.0f;
        if(HoveringAngle < 0.0f)
            HoveringAngle += 2.0f * pi;
        HoveringIndex = (int)(HoveringAngle / (2.0f * pi) * SegmentCount);
        if(Ui()->MouseButtonClicked(0))
        {
            s_SelectedBindIndex = HoveringIndex;
            str_copy(s_aBindName, GameClient()->m_BindWheelMRX.m_vBinds[HoveringIndex].m_aName, sizeof(s_aBindName));
            str_copy(s_aBindCommand, GameClient()->m_BindWheelMRX.m_vBinds[HoveringIndex].m_aCommand, sizeof(s_aBindCommand));
        }
        else if(Ui()->MouseButtonClicked(1) && s_SelectedBindIndex >= 0 && HoveringIndex >= 0 && HoveringIndex != s_SelectedBindIndex)
        {
            auto &Binds = GameClient()->m_BindWheelMRX.m_vBinds;
            CBindWheelMRX::CBind BindA = Binds[s_SelectedBindIndex];
            CBindWheelMRX::CBind BindB = Binds[HoveringIndex];
            str_copy(Binds[s_SelectedBindIndex].m_aName, BindB.m_aName, sizeof(Binds[s_SelectedBindIndex].m_aName));
            str_copy(Binds[s_SelectedBindIndex].m_aCommand, BindB.m_aCommand, sizeof(Binds[s_SelectedBindIndex].m_aCommand));
            str_copy(Binds[HoveringIndex].m_aName, BindA.m_aName, sizeof(Binds[HoveringIndex].m_aName));
            str_copy(Binds[HoveringIndex].m_aCommand, BindA.m_aCommand, sizeof(Binds[HoveringIndex].m_aCommand));
        }
        else if(Ui()->MouseButtonClicked(2))
        {
            s_SelectedBindIndex = HoveringIndex;
        }
    }
    else if(MouseDist < Radius && Ui()->MouseButtonClicked(0))
    {
        s_SelectedBindIndex = -1;
        str_copy(s_aBindName, "", sizeof(s_aBindName));
        str_copy(s_aBindCommand, "", sizeof(s_aBindCommand));
    }

    const float Theta = pi * 2.0f / (GameClient()->m_BindWheelMRX.m_vBinds.size() ? GameClient()->m_BindWheelMRX.m_vBinds.size() : 1);
    for(int i = 0; i < static_cast<int>(GameClient()->m_BindWheelMRX.m_vBinds.size()); i++)
    {
        float SegmentFontSize = FONT_SIZE * 1.1f;
        if(i == s_SelectedBindIndex)
        {
            SegmentFontSize = FONT_SIZE * 1.7f;
            TextRender()->TextColor(ColorRGBA(0.5f, 1.0f, 0.75f, 1.0f));
        }
        else if(i == HoveringIndex)
        {
            SegmentFontSize = FONT_SIZE * 1.35f;
        }
        const auto &Bind = GameClient()->m_BindWheelMRX.m_vBinds[i];
        const float Angle = Theta * i;
        vec2 TextPos = direction(Angle);
        TextPos *= Radius * 0.75f;
        float Width = TextRender()->TextWidth(SegmentFontSize, Bind.m_aName);
        TextPos += Pos;
        TextPos.x -= Width / 2.0f;
        TextRender()->Text(TextPos.x, TextPos.y, SegmentFontSize, Bind.m_aName);
        TextRender()->TextColor(TextRender()->DefaultTextColor());
    }

    LeftView.HSplitTop(LINE_SIZE, &Button, &LeftView);
    Button.VSplitLeft(100.0f, &Label, &Button);
    Ui()->DoLabel(&Label, Localize("Name:"), FONT_SIZE, TEXTALIGN_ML);
    static CLineInput s_NameInput;
    s_NameInput.SetBuffer(s_aBindName, sizeof(s_aBindName));
    s_NameInput.SetEmptyText(Localize("Name"));
    Ui()->DoEditBox(&s_NameInput, &Button, EDITBOX_FONT_SIZE);

    LeftView.HSplitTop(MARGIN_SMALL, nullptr, &LeftView);
    LeftView.HSplitTop(LINE_SIZE, &Button, &LeftView);
    Button.VSplitLeft(100.0f, &Label, &Button);
    Ui()->DoLabel(&Label, Localize("Command:"), FONT_SIZE, TEXTALIGN_ML);
    static CLineInput s_BindInput;
    s_BindInput.SetBuffer(s_aBindCommand, sizeof(s_aBindCommand));
    s_BindInput.SetEmptyText(Localize("Command"));
    Ui()->DoEditBox(&s_BindInput, &Button, EDITBOX_FONT_SIZE);

    static CButtonContainer s_AddButton, s_RemoveButton, s_OverrideButton;

    LeftView.HSplitTop(MARGIN_SMALL, nullptr, &LeftView);
    LeftView.HSplitTop(LINE_SIZE, &Button, &LeftView);
    if(DoButton_Menu(&s_OverrideButton, Localize("Override Selected"), 0, &Button, BUTTONFLAG_LEFT, nullptr, IGraphics::CORNER_ALL, 5.0f, 0.0f, ColorRGBA(1.0f, 1.0f, 1.0f, 0.22f)) && s_SelectedBindIndex >= 0)
    {
        CBindWheelMRX::CBind TempBind;
        if(str_length(s_aBindName) == 0)
            str_copy(TempBind.m_aName, "*", sizeof(TempBind.m_aName));
        else
            str_copy(TempBind.m_aName, s_aBindName, sizeof(TempBind.m_aName));
        str_copy(GameClient()->m_BindWheelMRX.m_vBinds[s_SelectedBindIndex].m_aName, TempBind.m_aName, sizeof(TempBind.m_aName));
        str_copy(GameClient()->m_BindWheelMRX.m_vBinds[s_SelectedBindIndex].m_aCommand, s_aBindCommand, sizeof(GameClient()->m_BindWheelMRX.m_vBinds[s_SelectedBindIndex].m_aCommand));
    }
    LeftView.HSplitTop(MARGIN_SMALL, nullptr, &LeftView);
    LeftView.HSplitTop(LINE_SIZE, &Button, &LeftView);
    CUIRect ButtonAdd, ButtonRemove;
    Button.VSplitMid(&ButtonRemove, &ButtonAdd, MARGIN_SMALL);
    if(DoButton_Menu(&s_AddButton, Localize("Add Bind"), 0, &ButtonAdd, BUTTONFLAG_LEFT, nullptr, IGraphics::CORNER_ALL, 5.0f, 0.0f, ColorRGBA(1.0f, 1.0f, 1.0f, 0.22f)))
    {
        CBindWheelMRX::CBind TempBind;
        if(str_length(s_aBindName) == 0)
            str_copy(TempBind.m_aName, "*", sizeof(TempBind.m_aName));
        else
            str_copy(TempBind.m_aName, s_aBindName, sizeof(TempBind.m_aName));
        GameClient()->m_BindWheelMRX.AddBind(TempBind.m_aName, s_aBindCommand);
        s_SelectedBindIndex = static_cast<int>(GameClient()->m_BindWheelMRX.m_vBinds.size()) - 1;
    }
    if(DoButton_Menu(&s_RemoveButton, Localize("Remove Bind"), 0, &ButtonRemove, BUTTONFLAG_LEFT, nullptr, IGraphics::CORNER_ALL, 5.0f, 0.0f, ColorRGBA(1.0f, 1.0f, 1.0f, 0.22f)) && s_SelectedBindIndex >= 0)
    {
        GameClient()->m_BindWheelMRX.RemoveBind(s_SelectedBindIndex);
        s_SelectedBindIndex = -1;
    }

    LeftView.HSplitTop(MARGIN_SMALL, nullptr, &LeftView);
    LeftView.HSplitTop(LINE_SIZE, &Label, &LeftView);
    Ui()->DoLabel(&Label, Localize("The command is run in console not chat"), FONT_SIZE, TEXTALIGN_ML);
    LeftView.HSplitTop(LINE_SIZE, &Label, &LeftView);
    Ui()->DoLabel(&Label, Localize("Use left mouse to select"), FONT_SIZE, TEXTALIGN_ML);
    LeftView.HSplitTop(LINE_SIZE, &Label, &LeftView);
    Ui()->DoLabel(&Label, Localize("Use right mouse to swap with selected"), FONT_SIZE, TEXTALIGN_ML);
    LeftView.HSplitTop(LINE_SIZE, &Label, &LeftView);
    Ui()->DoLabel(&Label, Localize("Use middle mouse select without copy"), FONT_SIZE, TEXTALIGN_ML);

    // Settings pinned to bottom
    LeftView.HSplitTop(MARGIN_SMALL, nullptr, &LeftView);
    CUIRect BottomKeyBind, BottomReset;
    // carve space for keybind at the very bottom
    LeftView.HSplitBottom(LINE_SIZE, &LeftView, &BottomKeyBind);
    // carve space for reset just above keybind
    LeftView.HSplitBottom(LINE_SIZE, &LeftView, &BottomReset);

    static int s_ResetMouse = 0;
    DoCheckBoxRow(BottomReset, LINE_SIZE, "Reset mouse position on open", s_ResetMouse);

    static CButtonContainer s_BindWheelKeyButton;
    static int s_BindWheelKey = 0, s_BindWheelMod = 0;
    DoKeyBindRow(BottomKeyBind, LINE_SIZE, "Bind Wheel Key", "+bindwheel", s_BindWheelKeyButton, s_BindWheelKey, s_BindWheelMod);
}
#pragma endregion WheelBinds

void CMenus::RenderTabPageTAS(CUIRect MainView)
{
    // Прокрутка и секции
    static CScrollRegion s_ScrollRegion;
    static std::vector<CUIRect> s_SectionBoxes;
    static vec2 s_PrevScrollOffset(0.0f, 0.0f);
    vec2 ScrollOffset(0.0f, 0.0f);
    CScrollRegionParams ScrollParams;
    ScrollParams.m_ScrollUnit = 120.0f;
    ScrollParams.m_Flags = CScrollRegionParams::FLAG_CONTENT_STATIC_WIDTH;
    ScrollParams.m_ScrollbarMargin = 5.0f;
    s_ScrollRegion.Begin(&MainView, &ScrollOffset, &ScrollParams);

    // Учёт прокрутки и поля под скроллбар
    MainView.y += ScrollOffset.y;
    MainView.VSplitRight(5.0f, &MainView, nullptr);
    MainView.VSplitLeft(2.0f, nullptr, &MainView);

    CUIRect ContentArea = MainView;

    // Адаптивное разделение на две колонки
    CUIRect LeftView, RightView;
    float GapSize = MARGIN_BLOCKS * 1.3f;

    const float MinLeftWidth = 200.0f;
    const float MinRightWidth = 180.0f;

    float AvailableWidth = MainView.w - GapSize;
    float LeftWidth = 0.0f;

    if(AvailableWidth >= (MinLeftWidth + MinRightWidth))
    {
        LeftWidth = std::max(MinLeftWidth, AvailableWidth * 0.55f);
    }
    else
    {
        LeftWidth = std::max(MinLeftWidth * 0.8f, AvailableWidth * 0.5f);
    }

    MainView.VSplitLeft(LeftWidth, &LeftView, &MainView);
    MainView.VSplitLeft(GapSize, nullptr, &MainView);
    RightView = MainView;

    LeftView.VSplitLeft(4.0f, nullptr, &LeftView);
    RightView.VSplitRight(MARGIN_SMALL, &RightView, nullptr);

    // Отрисовка фонов прошлых секций
    for(size_t i = 0; i < s_SectionBoxes.size(); ++i)
    {
        RenderBackgroundLines(s_SectionBoxes[i], (int)i);
        CUIRect Section = s_SectionBoxes[i];
        float Padding = MARGIN_BETWEEN_VIEWS * 0.6666f;
        Section.w += Padding;
        Section.h += Padding;
        Section.x -= Padding * 0.5f;
        Section.y -= Padding * 0.5f;
        Section.y -= s_PrevScrollOffset.y - ScrollOffset.y;
        float Shade = 0.0f;
        Section.Draw(ColorRGBA(Shade, Shade, Shade, 0.25f), IGraphics::CORNER_ALL, 10.0f);
    }
    s_PrevScrollOffset = ScrollOffset;
    s_SectionBoxes.clear();

    // ***** Левая колонка ***** //
    CUIRect LeftColumn = LeftView;

    // --- Секция: Основные элементы управления ---
    {
        CUIRect Header;
        LeftColumn.HSplitTop(MARGIN, nullptr, &LeftColumn);
        {
            CUIRect Bg = LeftColumn;
            Bg.VMargin(MARGIN_SMALL, &Bg);
            s_SectionBoxes.push_back(Bg);
        }
        LeftColumn.HSplitTop(HEADLINE_HEIGHT, &Header, &LeftColumn);
        Ui()->DoLabel(&Header, Localize("Main Controls"), HEADLINE_FONT_SIZE, TEXTALIGN_ML);
        LeftColumn.HSplitTop(MARGIN_SMALL, nullptr, &LeftColumn);

        // TPS
        DoSliderRow(LeftColumn, LINE_SIZE, Localize("TPS (Ticks per second)"), g_Config.m_MRXTasTps, 1, 200, "", "");
        LeftColumn.HSplitTop(MARGIN_EXTRA_SMALL, nullptr, &LeftColumn);

        // Pause hotkey
        static CButtonContainer s_PauseKeyButton;
        static int s_PauseKey = 0;
        static int s_PauseMod = 0;
        DoKeyBindRow(LeftColumn, LINE_SIZE, Localize("Pause"), "toggle mrx_taspause 0 1", s_PauseKeyButton, s_PauseKey, s_PauseMod);
        LeftColumn.HSplitTop(MARGIN_EXTRA_SMALL, nullptr, &LeftColumn);

        // Load replay hotkey
        static CButtonContainer s_LoadKeyButton;
        static int s_LoadKey = 0;
        static int s_LoadMod = 0;
        DoKeyBindRow(LeftColumn, LINE_SIZE, Localize("Load Replay"), "toggle mrx_loadreplay 1 0", s_LoadKeyButton, s_LoadKey, s_LoadMod);
        LeftColumn.HSplitTop(MARGIN_EXTRA_SMALL, nullptr, &LeftColumn);

        // Record hotkey
        static CButtonContainer s_RecordKeyButton;
        static int s_RecordKey = 0;
        static int s_RecordMod = 0;
        DoKeyBindRow(LeftColumn, LINE_SIZE, Localize("Record"), "toggle mrx_recordreplay 1 0", s_RecordKeyButton, s_RecordKey, s_RecordMod);
        LeftColumn.HSplitTop(MARGIN_EXTRA_SMALL, nullptr, &LeftColumn);

        // Clear replay hotkey
        static CButtonContainer s_ClearKeyButton;
        static int s_ClearKey = 0;
        static int s_ClearMod = 0;
        DoKeyBindRow(LeftColumn, LINE_SIZE, Localize("Clear Replay"), "+toggle mrx_tasrespawn 1 0", s_ClearKeyButton, s_ClearKey, s_ClearMod);
        LeftColumn.HSplitTop(MARGIN_EXTRA_SMALL, nullptr, &LeftColumn);

        // Rewind hotkey
        static CButtonContainer s_RewindKeyButton;
        static int s_RewindKey = 0;
        static int s_RewindMod = 0;
        DoKeyBindRow(LeftColumn, LINE_SIZE, Localize("Rewind"), "+toggle mrx_tasrewind 1 0", s_RewindKeyButton, s_RewindKey, s_RewindMod);
        LeftColumn.HSplitTop(MARGIN_EXTRA_SMALL, nullptr, &LeftColumn);

        // Forward hotkey
        static CButtonContainer s_ForwardKeyButton;
        static int s_ForwardKey = 0;
        static int s_ForwardMod = 0;
        DoKeyBindRow(LeftColumn, LINE_SIZE, Localize("Forward"), "+toggle mrx_tasforward 1 0", s_ForwardKeyButton, s_ForwardKey, s_ForwardMod);
        LeftColumn.HSplitTop(MARGIN_EXTRA_SMALL, nullptr, &LeftColumn);

        s_SectionBoxes.back().h = LeftColumn.y - s_SectionBoxes.back().y;
    }

    LeftColumn.HSplitTop(MARGIN_BETWEEN_VIEWS, nullptr, &LeftColumn);

    // --- Секция: Настройки ---
    {
        CUIRect Header;
        {
            CUIRect Bg = LeftColumn;
            Bg.VMargin(MARGIN_SMALL, &Bg);
            s_SectionBoxes.push_back(Bg);
        }
        LeftColumn.HSplitTop(HEADLINE_HEIGHT, &Header, &LeftColumn);
        Ui()->DoLabel(&Header, Localize("Settings"), HEADLINE_FONT_SIZE, TEXTALIGN_ML);
        LeftColumn.HSplitTop(MARGIN_SMALL, nullptr, &LeftColumn);

        DoCheckBoxRow(LeftColumn, LINE_SIZE, Localize("Enable Sound"), g_Config.m_MRXTasUseSound);
        LeftColumn.HSplitTop(MARGIN_EXTRA_SMALL, nullptr, &LeftColumn);

        DoCheckBoxRow(LeftColumn, LINE_SIZE, Localize("Enable Effects"), g_Config.m_MRXTasShowEffects);
        LeftColumn.HSplitTop(MARGIN_EXTRA_SMALL, nullptr, &LeftColumn);

        DoCheckBoxRow(LeftColumn, LINE_SIZE, Localize("Show Real Aim"), g_Config.m_MRXTasShowAim);
        LeftColumn.HSplitTop(MARGIN_EXTRA_SMALL, nullptr, &LeftColumn);

        DoCheckBoxRow(LeftColumn, LINE_SIZE, Localize("Player Prediction"), g_Config.m_MRXTasPlayerPrediction);
        LeftColumn.HSplitTop(MARGIN_EXTRA_SMALL, nullptr, &LeftColumn);

        s_SectionBoxes.back().h = LeftColumn.y - s_SectionBoxes.back().y;
    }

    LeftColumn.HSplitTop(MARGIN_BETWEEN_VIEWS, nullptr, &LeftColumn);

    // --- Секция: Управление воспроизведением ---
    {
        CUIRect Header;
        {
            CUIRect Bg = LeftColumn;
            Bg.VMargin(MARGIN_SMALL, &Bg);
            s_SectionBoxes.push_back(Bg);
        }
        LeftColumn.HSplitTop(HEADLINE_HEIGHT, &Header, &LeftColumn);
        Ui()->DoLabel(&Header, Localize("Replay Management"), HEADLINE_FONT_SIZE, TEXTALIGN_ML);
        LeftColumn.HSplitTop(MARGIN_SMALL, nullptr, &LeftColumn);

        // Folder button
        {
            CUIRect Button;
            LeftColumn.HSplitTop(LINE_SIZE, &Button, &LeftColumn);
            static CButtonContainer s_FolderButton;
            if(DoButton_Menu(&s_FolderButton, Localize("Open Replays Folder"), 0, &Button))
            {
                // Open replays folder (would need actual implementation)
                dbg_msg("TAS", "Open replays folder");
            }
            LeftColumn.HSplitTop(MARGIN_EXTRA_SMALL, nullptr, &LeftColumn);
        }

        // Replay selector (placeholder - would need actual implementation)
        {
            CUIRect Button;
            LeftColumn.HSplitTop(LINE_SIZE, &Button, &LeftColumn);
            static CButtonContainer s_ReplaySelector;
            DoButton_Menu(&s_ReplaySelector, Localize("Select Replay..."), 0, &Button);
            LeftColumn.HSplitTop(MARGIN_EXTRA_SMALL, nullptr, &LeftColumn);
        }

        // Refresh button
        {
            CUIRect Button;
            LeftColumn.HSplitTop(LINE_SIZE, &Button, &LeftColumn);
            static CButtonContainer s_RefreshButton;
            if(DoButton_Menu(&s_RefreshButton, Localize("Refresh"), 0, &Button))
            {
                dbg_msg("TAS", "Refresh replay list");
            }
            LeftColumn.HSplitTop(MARGIN_EXTRA_SMALL, nullptr, &LeftColumn);
        }

        // Load replay data
        {
            CUIRect Button;
            LeftColumn.HSplitTop(LINE_SIZE, &Button, &LeftColumn);
            static CButtonContainer s_LoadDataButton;
            if(DoButton_Menu(&s_LoadDataButton, Localize("Load Replay Data"), 0, &Button))
            {
                dbg_msg("TAS", "Load replay data");
            }
            LeftColumn.HSplitTop(MARGIN_EXTRA_SMALL, nullptr, &LeftColumn);
        }

        // Verify replay
        {
            CUIRect Button;
            LeftColumn.HSplitTop(LINE_SIZE, &Button, &LeftColumn);
            static CButtonContainer s_VerifyButton;
            if(DoButton_Menu(&s_VerifyButton, Localize("Verify Replay"), 0, &Button))
            {
                dbg_msg("TAS", "Verify replay");
            }
            LeftColumn.HSplitTop(MARGIN_EXTRA_SMALL, nullptr, &LeftColumn);
        }

        // Save replay
        {
            CUIRect Button;
            LeftColumn.HSplitTop(LINE_SIZE, &Button, &LeftColumn);
            static CButtonContainer s_SaveButton;
            if(DoButton_Menu(&s_SaveButton, Localize("Save Replay"), 0, &Button))
            {
                dbg_msg("TAS", "Save replay");
            }
            LeftColumn.HSplitTop(MARGIN_EXTRA_SMALL, nullptr, &LeftColumn);
        }

        // Auto-sync
        DoCheckBoxRow(LeftColumn, LINE_SIZE, Localize("Auto-sync Replay Vault"), g_Config.m_MRXTasReplayVaultAutoSync);
        LeftColumn.HSplitTop(MARGIN_EXTRA_SMALL, nullptr, &LeftColumn);

        // Sync now
        {
            CUIRect Button;
            LeftColumn.HSplitTop(LINE_SIZE, &Button, &LeftColumn);
            static CButtonContainer s_SyncButton;
            if(DoButton_Menu(&s_SyncButton, Localize("Sync Now"), 0, &Button))
            {
                dbg_msg("TAS", "Sync replay vault");
            }
            LeftColumn.HSplitTop(MARGIN_EXTRA_SMALL, nullptr, &LeftColumn);
        }

        s_SectionBoxes.back().h = LeftColumn.y - s_SectionBoxes.back().y;
    }

    // ***** Правая колонка ***** //
    CUIRect RightColumn = RightView;

    // --- Секция: Инструменты ---
    {
        CUIRect Header;
        RightColumn.HSplitTop(MARGIN, nullptr, &RightColumn);
        {
            CUIRect Bg = RightColumn;
            Bg.VMargin(MARGIN_SMALL, &Bg);
            s_SectionBoxes.push_back(Bg);
        }
        RightColumn.HSplitTop(HEADLINE_HEIGHT, &Header, &RightColumn);
        Ui()->DoLabel(&Header, Localize("Tools"), HEADLINE_FONT_SIZE, TEXTALIGN_ML);
        RightColumn.HSplitTop(MARGIN_SMALL, nullptr, &RightColumn);

        DoSliderRow(RightColumn, LINE_SIZE, Localize("Ticks"), g_Config.m_MRXTasTickControlTicks, 1, 100, "", "");
        RightColumn.HSplitTop(MARGIN_EXTRA_SMALL, nullptr, &RightColumn);

        DoCheckBoxRow(RightColumn, LINE_SIZE, Localize("Auto Rewind"), g_Config.m_MRXTasAutoRewind);
        RightColumn.HSplitTop(MARGIN_EXTRA_SMALL, nullptr, &RightColumn);

        DoCheckBoxRow(RightColumn, LINE_SIZE, Localize("Auto Forward"), g_Config.m_MRXTasAutoForward);
        RightColumn.HSplitTop(MARGIN_EXTRA_SMALL, nullptr, &RightColumn);

        DoCheckBoxRow(RightColumn, LINE_SIZE, Localize("Auto Pause"), g_Config.m_MRXTickControlPause);
        RightColumn.HSplitTop(MARGIN_EXTRA_SMALL, nullptr, &RightColumn);

        DoCheckBoxRow(RightColumn, LINE_SIZE, Localize("Step Mode"), g_Config.m_MRXTickControlStep);
        RightColumn.HSplitTop(MARGIN_EXTRA_SMALL, nullptr, &RightColumn);

        s_SectionBoxes.back().h = RightColumn.y - s_SectionBoxes.back().y;
    }

    RightColumn.HSplitTop(MARGIN_BETWEEN_VIEWS, nullptr, &RightColumn);

    // --- Секция: Визуальные эффекты ---
    {
        CUIRect Header;
        {
            CUIRect Bg = RightColumn;
            Bg.VMargin(MARGIN_SMALL, &Bg);
            s_SectionBoxes.push_back(Bg);
        }
        RightColumn.HSplitTop(HEADLINE_HEIGHT, &Header, &RightColumn);
        Ui()->DoLabel(&Header, Localize("Visual Effects"), HEADLINE_FONT_SIZE, TEXTALIGN_ML);
        RightColumn.HSplitTop(MARGIN_SMALL, nullptr, &RightColumn);

        DoCheckBoxRow(RightColumn, LINE_SIZE, Localize("Draw Start/End Position"), g_Config.m_MRXTasDrawStartEndPos);
        RightColumn.HSplitTop(MARGIN_EXTRA_SMALL, nullptr, &RightColumn);

        DoCheckBoxRow(RightColumn, LINE_SIZE, Localize("Draw Replay Path"), g_Config.m_MRXTasDrawPath);
        RightColumn.HSplitTop(MARGIN_EXTRA_SMALL, nullptr, &RightColumn);

        if(g_Config.m_MRXTasDrawPath)
        {
            DoCircleToggleRow(RightColumn, LINE_SIZE, Localize("  Segmented"), g_Config.m_MRXTasDrawPathSegmented);
            RightColumn.HSplitTop(MARGIN_EXTRA_SMALL, nullptr, &RightColumn);

            DoCircleToggleRow(RightColumn, LINE_SIZE, Localize("  Mode (0=Dashed, 1=Solid)"), g_Config.m_MRXTasDrawPathMode);
            RightColumn.HSplitTop(MARGIN_EXTRA_SMALL, nullptr, &RightColumn);

            static CButtonContainer s_TasPathColorReset;
            DoLine_ColorPicker(&s_TasPathColorReset, COLOR_PICKER_LINE_SIZE, COLOR_PICKER_LABEL_SIZE, COLOR_PICKER_LINE_SPACING,
                &RightColumn, Localize("  Path Color"), &g_Config.m_MRXTasDrawPathColor, ColorRGBA(1.0f, 1.0f, 1.0f, 1.0f), false, nullptr, false);
            RightColumn.HSplitTop(MARGIN_EXTRA_SMALL, nullptr, &RightColumn);
        }

        DoCheckBoxRow(RightColumn, LINE_SIZE, Localize("Draw Prediction Path"), g_Config.m_MRXTasDrawPredictionPath);
        RightColumn.HSplitTop(MARGIN_EXTRA_SMALL, nullptr, &RightColumn);

        if(g_Config.m_MRXTasDrawPredictionPath)
        {
            DoCircleToggleRow(RightColumn, LINE_SIZE, Localize("  Mode (0=Dashed, 1=Solid)"), g_Config.m_MRXTasDrawPredictionPathMode);
            RightColumn.HSplitTop(MARGIN_EXTRA_SMALL, nullptr, &RightColumn);

            static CButtonContainer s_TasPredictionLocalColorReset;
            DoLine_ColorPicker(&s_TasPredictionLocalColorReset, COLOR_PICKER_LINE_SIZE, COLOR_PICKER_LABEL_SIZE, COLOR_PICKER_LINE_SPACING,
                &RightColumn, Localize("  Local Color"), &g_Config.m_MRXTasDrawPredictionPathColor, ColorRGBA(0.0f, 1.0f, 0.0f, 1.0f), false, nullptr, false);
            RightColumn.HSplitTop(MARGIN_EXTRA_SMALL, nullptr, &RightColumn);

            static CButtonContainer s_TasPredictionFrozenColorReset;
            DoLine_ColorPicker(&s_TasPredictionFrozenColorReset, COLOR_PICKER_LINE_SIZE, COLOR_PICKER_LABEL_SIZE, COLOR_PICKER_LINE_SPACING,
                &RightColumn, Localize("  Frozen Color"), &g_Config.m_MRXTasDrawPredictionPathColorFrozen, ColorRGBA(0.0f, 0.0f, 1.0f, 1.0f), false, nullptr, false);
            RightColumn.HSplitTop(MARGIN_EXTRA_SMALL, nullptr, &RightColumn);

            static CButtonContainer s_TasPredictionOthersColorReset;
            DoLine_ColorPicker(&s_TasPredictionOthersColorReset, COLOR_PICKER_LINE_SIZE, COLOR_PICKER_LABEL_SIZE, COLOR_PICKER_LINE_SPACING,
                &RightColumn, Localize("  Others Color"), &g_Config.m_MRXTasDrawPredictionPathColorOthers, ColorRGBA(1.0f, 1.0f, 0.0f, 1.0f), false, nullptr, false);
            RightColumn.HSplitTop(MARGIN_EXTRA_SMALL, nullptr, &RightColumn);
        }

        s_SectionBoxes.back().h = RightColumn.y - s_SectionBoxes.back().y;
    }

    // Завершение скролла
    const float EndY = std::max(LeftColumn.y, RightColumn.y);
    CUIRect ScrollRegion = ContentArea;
    ScrollRegion.y = ContentArea.y;
    ScrollRegion.h = EndY - ContentArea.y;
    s_ScrollRegion.AddRect(ScrollRegion);
    s_ScrollRegion.End();
}

void CMenus::RenderMRXTabPage(int PageId, CUIRect MainView)
{
    switch(PageId)
    {
    case MRX_TAB_AIMBOT:
        RenderTabPageAimbot(MainView);
        break;
    case MRX_TAB_AVOID:
        RenderTabPageAvoid(MainView);
        break;
    case MRX_TAB_TAS:
        RenderTabPageTAS(MainView);
        break;
    case MRX_TAB_MISC:
        RenderTabPageMisc(MainView);
        break;
    case MRX_TAB_VISUALS:
        RenderTabPageVisuals(MainView);
        break;
    case MRX_TAB_FLY_HELPER:
        RenderTabPageFlyHelper(MainView);
        break;
    case MRX_TAB_WHEEL_BINDS:
        RenderTabPageWheelBinds(MainView);
        break;
    }
}

void CMenus::RenderSettingsMRX(CUIRect MainView)
{
    if(!g_Config.m_ClPlayerDeffis)
        return;

    // Константы для макета
    const float LineSize = 20.0f;
    const float SectionMargin = 5.0f;

    // анимашки перехода блоков
    static int s_CurrentTab = 0;
    static int s_PreviousTab = -1;
    static float s_TabAnimProgress = 1.0f;
    static float s_TabAnimStartTime = 0.0f;
    const float TAB_ANIM_DURATION = 0.35f; // длительность перехода в секундах

    CUIRect TabBar;
    MainView.HSplitTop(LineSize, &TabBar, &MainView);

    int NewTab = RenderTablist(TabBar);

    if(NewTab != s_CurrentTab)
    {
        s_PreviousTab = s_CurrentTab;
        s_CurrentTab = NewTab;
        s_TabAnimStartTime = Client()->LocalTime();
        s_TabAnimProgress = 0.0f;
    }

    if(s_TabAnimProgress < 1.0f)
    {
        float Elapsed = Client()->LocalTime() - s_TabAnimStartTime;
        s_TabAnimProgress = std::min(Elapsed / TAB_ANIM_DURATION, 1.0f);
    }
    else
    {
        s_PreviousTab = -1;
    }

    MainView.HSplitTop(SectionMargin, nullptr, &MainView);
    MainView.Margin(SectionMargin, &MainView);

    auto Ease = [](float p) -> float {
        return p < 0.5f ? 4.0f * p * p * p : 1.0f - pow(-2.0f * p + 2.0f, 3.0f) / 2.0f;
    };
    const float EasedProgress = Ease(s_TabAnimProgress);

    if(s_PreviousTab != -1 && s_TabAnimProgress < 1.0f)
    {
        const int Dir = (s_CurrentTab > s_PreviousTab) ? 1 : -1;

        Ui()->ClipEnable(&MainView);

        CUIRect OldTabRect = MainView;
        OldTabRect.x -= Dir * MainView.w * EasedProgress;
        RenderMRXTabPage(s_PreviousTab, OldTabRect);

        CUIRect NewTabRect = MainView;
        NewTabRect.x += Dir * MainView.w * (1.0f - EasedProgress);
        RenderMRXTabPage(s_CurrentTab, NewTabRect);

        Graphics()->TextureClear();
        Graphics()->QuadsBegin();
        Graphics()->SetColor(0.0f, 0.0f, 0.0f, EasedProgress * 0.2f); // Затухание до 20% черного
        IGraphics::CQuadItem QuadOld(OldTabRect.x, OldTabRect.y, OldTabRect.w, OldTabRect.h);
        Graphics()->QuadsDrawTL(&QuadOld, 1);
        Graphics()->QuadsEnd();

        Graphics()->TextureClear();
        Graphics()->QuadsBegin();
        Graphics()->SetColor(0.0f, 0.0f, 0.0f, (1.0f - EasedProgress) * 0.2f); // От 20% черного к прозрачному
        IGraphics::CQuadItem QuadNew(NewTabRect.x, NewTabRect.y, NewTabRect.w, NewTabRect.h);
        Graphics()->QuadsDrawTL(&QuadNew, 1);
        Graphics()->QuadsEnd();

        Ui()->ClipDisable();
    }
    else
    {
        RenderMRXTabPage(s_CurrentTab, MainView);
    }
}

void CMenus::OnConsoleInit()
{
    // MRX console commands are now registered in CMRXSimpleConfigs::Init()
    // This method is kept empty to satisfy the override requirement
}
