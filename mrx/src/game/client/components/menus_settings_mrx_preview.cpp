// (c) MRX-Client - Fullscreen MRX overlay preview rendering
#include <game/client/components/menus.h>

#include <game/client/animstate.h>
#include <game/client/gameclient.h>
#include <game/client/render.h>
#include <game/client/skin.h>
#include <game/client/components/cheat/visual/arraylist.h>

#include <engine/graphics.h>
#include <engine/shared/config.h>
#include <engine/textrender.h>

#include <game/localization.h>

#include <base/color.h>

#include <algorithm>

using namespace FontIcons;

void CMenus::RenderMRXOverlayFullscreenPreview()
{
	CUIRect Screen = *Ui()->Screen();
	const float ScreenW = std::max(1.0f, (float)Graphics()->ScreenWidth());
	const float ScreenH = std::max(1.0f, (float)Graphics()->ScreenHeight());
	const float UiW = Screen.w;
	const float UiH = Screen.h;
	const float UiScaleX = UiW / ScreenW;
	const float UiScaleY = UiH / ScreenH;

	auto ScreenToUIX = [&](float x) { return Screen.x + x * UiScaleX; };
	auto ScreenToUIY = [&](float y) { return Screen.y + y * UiScaleY; };
	auto UIToScreenX = [&](float x) { return (x - Screen.x) / UiScaleX; };
	auto UIToScreenY = [&](float y) { return (y - Screen.y) / UiScaleY; };
	auto ToPercentX = [&](float uiX, bool AlignRight = false) {
		if(AlignRight)
			uiX += 100.0f * UiScaleX; // Approximate width
		float sx = std::clamp(UIToScreenX(uiX), 0.0f, ScreenW);
		return (int)std::clamp(std::lround((sx / ScreenW) * 100.0 * 100.0), 0l, 20000l);
	};
	auto ToPercentY = [&](float uiY) {
		float sy = std::clamp(UIToScreenY(uiY), 0.0f, ScreenH);
		return (int)std::clamp(std::lround((sy / ScreenH) * 100.0 * 100.0), 0l, 20000l);
	};

	auto GetSlotTextColorLocal = [&](int idx) -> ColorRGBA {
		switch(idx)
		{
		case 0: return color_cast<ColorRGBA>(ColorHSLA(g_Config.m_MRXTextColor, false));
		case 1: return color_cast<ColorRGBA>(ColorHSLA(g_Config.m_MRXText1Color, false));
		case 2: return color_cast<ColorRGBA>(ColorHSLA(g_Config.m_MRXText2Color, false));
		case 3: return color_cast<ColorRGBA>(ColorHSLA(g_Config.m_MRXText3Color, false));
		case 4: return color_cast<ColorRGBA>(ColorHSLA(g_Config.m_MRXText4Color, false));
		default: return color_cast<ColorRGBA>(ColorHSLA(g_Config.m_MRXOverlayTextColor, false));
		}
	};

	Screen.Draw(ColorRGBA(0.0f, 0.0f, 0.0f, 0.25f), IGraphics::CORNER_ALL, 0.0f);

	CUIRect Header, Content;
	Screen.HSplitTop(48.0f, &Header, &Content);
	Header.Margin(8.0f, &Header);

	static CButtonContainer s_CloseButton;
	static int s_DraggingIndex = -1;
	static int s_DraggingArrayList = 0;
	static float s_DragDX = 0.0f;
	static float s_DragDY = 0.0f;
	static float s_DragCurrentX = 0.0f;
	static float s_DragCurrentY = 0.0f;

	CUIRect Title, CloseButton, Hint;
	Header.VSplitRight(40.0f, &Title, &CloseButton);
	
	// Подсказка рядом с кнопкой X
	Title.VSplitRight(300.0f, &Title, &Hint);
	Hint.VMargin(8.0f, &Hint);
	Ui()->DoLabel(&Hint, Localize("Drag text or ArrayList to reposition. Press Escape or click X to close."), 10.0f, TEXTALIGN_MR);
	
	if(Ui()->DoButton_FontIcon(&s_CloseButton, FONT_ICON_XMARK, 0, &CloseButton, BUTTONFLAG_LEFT))
	{
		m_MRXOverlayPreviewOpen = false;
		s_DraggingIndex = -1;
		Ui()->SetActiveItem(nullptr);
		return;
	}

	// Обработка Escape - закрываем превью, но не меню
	if(Ui()->ConsumeHotkey(CUi::HOTKEY_ESCAPE))
	{
		m_MRXOverlayPreviewOpen = false;
		s_DraggingIndex = -1;
		Ui()->SetActiveItem(nullptr);
		return;
	}

	auto MaybeHandleDrag = [&](int SlotIndex, const char *pText, int &XpRef, int &YpRef, float FontUI, float TextWidthUI, float TextHeightUI, float &outUiX, float &outUiY) {
		if(!pText || pText[0] == '\0')
			return;
		const vec2 MousePos = Ui()->MousePos();
		const bool MouseDown = Ui()->MouseButton(0);

		const float TextWidthScreen = TextWidthUI / UiScaleX;
		const float TextHeightScreen = TextHeightUI / UiScaleY;

		float ScreenX = std::clamp((XpRef / 10000.0f) * ScreenW, 0.0f, std::max(0.0f, ScreenW - TextWidthScreen));
		float ScreenY = std::clamp((YpRef / 10000.0f) * ScreenH, 0.0f, std::max(0.0f, ScreenH - TextHeightScreen));
		float UiX = ScreenToUIX(ScreenX);
		float UiY = ScreenToUIY(ScreenY);

		const float Pad = 4.0f;
		CUIRect Hitbox(UiX - Pad, UiY - Pad, TextWidthUI + Pad * 2.0f, TextHeightUI + Pad * 2.0f);

		if(s_DraggingIndex == -1 && MouseDown && Ui()->MouseInside(&Hitbox) && Ui()->ActiveItem() != &s_CloseButton)
		{
			s_DraggingIndex = SlotIndex;
			s_DragDX = MousePos.x - UiX;
			s_DragDY = MousePos.y - UiY;
			s_DragCurrentX = UiX;
			s_DragCurrentY = UiY;
		}

		if(!MouseDown && s_DraggingIndex == SlotIndex)
		{
			XpRef = ToPercentX(s_DragCurrentX);
			YpRef = ToPercentY(s_DragCurrentY);
			s_DraggingIndex = -1;
		}

		if(MouseDown && s_DraggingIndex == SlotIndex)
		{
			float UiNX = MousePos.x - s_DragDX;
			float UiNY = MousePos.y - s_DragDY;
			s_DragCurrentX = std::clamp(UiNX, Screen.x, Screen.x + Screen.w - TextWidthUI);
			s_DragCurrentY = std::clamp(UiNY, Screen.y, Screen.y + Screen.h - TextHeightUI);
			outUiX = s_DragCurrentX;
			outUiY = s_DragCurrentY;
		}
	};

	auto RenderPreviewText = [&](int SlotIndex, const char *pText, int &XpRef, int &YpRef) {
		if(!pText || pText[0] == '\0')
			return;

		// Используем тот же размер шрифта, что и в игре
		const float FontSize = (float)g_Config.m_MRXOverlayTextSize;
		const float TextWidthScreen = Ui()->TextRender()->TextWidth(FontSize, pText, -1);
		const float TextHeightScreen = FontSize;
		
		// Преобразуем размеры в UI координаты для обработки перетаскивания
		const float FontUI = FontSize * UiScaleY;
		const float TextWidthUI = TextWidthScreen * UiScaleX;
		const float TextHeightUI = TextHeightScreen * UiScaleY;
		float UiX = 0.0f, UiY = 0.0f;
		MaybeHandleDrag(SlotIndex, pText, XpRef, YpRef, FontUI, TextWidthUI, TextHeightUI, UiX, UiY);

		// Используем координаты из MaybeHandleDrag, если мы перетаскиваем элемент
		if(s_DraggingIndex != SlotIndex)
		{
			float ScreenX = std::clamp((XpRef / 10000.0f) * ScreenW, 0.0f, std::max(0.0f, ScreenW - TextWidthScreen));
			float ScreenY = std::clamp((YpRef / 10000.0f) * ScreenH, 0.0f, std::max(0.0f, ScreenH - TextHeightScreen));
			UiX = ScreenToUIX(ScreenX);
			UiY = ScreenToUIY(ScreenY);
		}

		const ColorRGBA TxtCol = GetSlotTextColorLocal(SlotIndex);
		Ui()->TextRender()->TextColor(TxtCol.r, TxtCol.g, TxtCol.b, TxtCol.a);
		Ui()->TextRender()->Text(UiX, UiY, FontUI, pText, -1.0f);
		Ui()->TextRender()->TextColor(1, 1, 1, 1);
	};

	// Обработка перемещения ArrayList
	if(g_Config.m_MRXArrayList)
	{
		CArrayList &pArrayList = GameClient()->m_ArrayList;
		const float ArrayListWidth = 100.0f * UiScaleX;
		const float ArrayListHeight = 300.0f * UiScaleY;

		float UiX, UiY;

		const vec2 MousePos = Ui()->MousePos();
		const bool MouseDown = Ui()->MouseButton(0);

		if(s_DraggingArrayList == 1)
		{
			if(MouseDown)
			{
				float UiNX = MousePos.x - s_DragDX;
				float UiNY = MousePos.y - s_DragDY;
				s_DragCurrentX = std::clamp(UiNX, Screen.x, Screen.x + Screen.w - ArrayListWidth);
				s_DragCurrentY = std::clamp(UiNY, Screen.y, Screen.y + Screen.h - ArrayListHeight);
			}
			else
			{
				bool AlignRight = s_DragCurrentX > Screen.x + Screen.w / 2.0f;
				g_Config.m_MRXArrayListX = ToPercentX(s_DragCurrentX, AlignRight);
				g_Config.m_MRXArrayListY = ToPercentY(s_DragCurrentY);
				s_DraggingArrayList = 0;
			}
			UiX = s_DragCurrentX;
			UiY = s_DragCurrentY;
		}
		else
		{
			float ScreenX = (g_Config.m_MRXArrayListX / 10000.0f) * ScreenW;
			float ScreenY = (g_Config.m_MRXArrayListY / 10000.0f) * ScreenH;

			bool AlignRight = ScreenX > ScreenW / 2.0f;
			if(AlignRight)
				ScreenX -= ArrayListWidth / UiScaleX;

			ScreenX = std::clamp(ScreenX, 0.0f, ScreenW - ArrayListWidth / UiScaleX);
			ScreenY = std::clamp(ScreenY, 0.0f, ScreenH - ArrayListHeight / UiScaleY);
			UiX = ScreenToUIX(ScreenX);
			UiY = ScreenToUIY(ScreenY);

			const float Pad = 4.0f;
			CUIRect Hitbox(UiX - Pad, UiY - Pad, ArrayListWidth + Pad * 2.0f, ArrayListHeight + Pad * 2.0f);
			if(MouseDown && Ui()->MouseInside(&Hitbox) && Ui()->ActiveItem() != &s_CloseButton && s_DraggingIndex == -1)
			{
				s_DraggingArrayList = 1;
				s_DragDX = MousePos.x - UiX;
				s_DragDY = MousePos.y - UiY;
				s_DragCurrentX = UiX;
				s_DragCurrentY = UiY;
			}
		}

		// Рисуем рамку вокруг ArrayList для наглядности
		CUIRect ArrayListRect(UiX, UiY, ArrayListWidth, ArrayListHeight);
		ArrayListRect.Draw(ColorRGBA(0.2f, 0.2f, 0.2f, 0.15f), IGraphics::CORNER_ALL, 5.0f);

		// Рендерим сам ArrayList
		GameClient()->m_ArrayList.m_IsPreview = true;
		GameClient()->m_ArrayList.OnRender();
		GameClient()->m_ArrayList.m_IsPreview = false;
	}

	if(g_Config.m_MRXOverlayText)
	{
		RenderPreviewText(0, g_Config.m_MRXText, g_Config.m_MRXTextX, g_Config.m_MRXTextY);
		RenderPreviewText(1, g_Config.m_MRXText1, g_Config.m_MRXText1X, g_Config.m_MRXText1Y);
		RenderPreviewText(2, g_Config.m_MRXText2, g_Config.m_MRXText2X, g_Config.m_MRXText2Y);
		RenderPreviewText(3, g_Config.m_MRXText3, g_Config.m_MRXText3X, g_Config.m_MRXText3Y);
		RenderPreviewText(4, g_Config.m_MRXText4, g_Config.m_MRXText4X, g_Config.m_MRXText4Y);
	}

	if(g_Config.m_MRXOverlaySameX)
	{
		const char *pHint = Localize("X matches another player");
		RenderPreviewText(5, pHint, g_Config.m_MRXOverlaySameXX, g_Config.m_MRXOverlaySameXY);
	}
}
