#pragma once

#include <game/client/gameclient.h>
#include "f_config.h"
#include "f_component.h"

class FVisuals : public FComponent
{
public:
	FVisuals(CGameClient *pClient) :
		FComponent(pClient){}

	void Run(int ClientID, float Angle, vec2 Position);

	// Draw
	void DrawLine(vec2 pos1, vec2 pos2, ColorRGBA color = {1, 1, 1, 1});
	void DrawCircle(vec2 pos, float size, ColorRGBA color = {1, 1, 1, 1});
	void DrawBox(vec2 center, float size, ColorRGBA color = {1, 1, 1, 1});
	void DrawGlowPlayer(vec2 center, ColorRGBA color = {1, 1, 1, 1});
	void DrawRadialGlow(vec2 center, float outerR, ColorRGBA centerColor, ColorRGBA outerColor);

private:
	// Aimbot Fov
	void DrawFov();
	void DrawFovLine(float offset);
};