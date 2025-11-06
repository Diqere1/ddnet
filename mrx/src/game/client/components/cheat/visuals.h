#ifndef GAME_CLIENT_COMPONENTS_CHEAT_MISC_VISUALS_H
#define GAME_CLIENT_COMPONENTS_CHEAT_MISC_VISUALS_H

#include <base/color.h>
#include <game/client/component.h>


class CVisuals : public CComponent
{
public:
	virtual int Sizeof() const override { return sizeof(*this); }

	void DrawLine(float x1, float y1, float x2, float y2, float w, ColorRGBA color);
	void DrawBox(float x, float y, float w, float h, bool fill, bool stroke,
		ColorRGBA fillColor = ColorRGBA(1, 1, 1, 0.3f),
		ColorRGBA strokeColor = ColorRGBA(1, 1, 1, 1.0f));
	void DrawCircle(float x, float y, float radius, bool fill, bool stroke, ColorRGBA color, ColorRGBA strokeColor);

	void DrawCharacter(const CNetObj_Character *pPrev, const CNetObj_Character *pCur, CCharacter *pChar, float IntraTick, bool ShowFreezeBar, bool UseNinjaSkin);
};

#endif