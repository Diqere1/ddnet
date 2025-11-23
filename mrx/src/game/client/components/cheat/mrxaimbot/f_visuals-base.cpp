#include "f_visuals.h"

void FVisuals::DrawLine(vec2 pos1, vec2 pos2, ColorRGBA color)
{
	Graphics()->LinesBegin();
	const IGraphics::CLineItem LineItem(pos1.x, pos1.y, pos2.x, pos2.y);
	Graphics()->SetColor(color);
	Graphics()->LinesDraw(&LineItem, 1);
	Graphics()->LinesEnd();
}

void FVisuals::DrawRadialGlow(vec2 center, float outerR, ColorRGBA centerColor, ColorRGBA outerColor)
{
    // Radial fan (pie) with per-vertex gradient: center -> outer edge
    const int segments = 96;
    const float step = 2.0f * pi / segments;
    const float innerR = 0.0f; // true center

    Graphics()->TextureClear();
    Graphics()->QuadsBegin();
    for(int i = 0; i < segments; ++i)
    {
        const float a0 = i * step;
        const float a1 = (i + 1) * step;

        const vec2 c0 = center;                   // center point
        const vec2 m0 = center + vec2(cos(a0), sin(a0)) * outerR;
        const vec2 m1 = center + vec2(cos(a1), sin(a1)) * outerR;

        // Build two skinny quads to emulate a triangle fan with color interpolation
        // Quad 1 (center->edge a0)
        Graphics()->SetColor4(centerColor, centerColor, outerColor, outerColor);
        IGraphics::CFreeformItem q1(
            c0.x, c0.y,
            c0.x, c0.y,
            m0.x, m0.y,
            m1.x, m1.y);
        Graphics()->QuadsDrawFreeform(&q1, 1);
    }
    Graphics()->QuadsEnd();
}
void FVisuals::DrawGlowPlayer(vec2 center, ColorRGBA color)
{
    // Approximate tee contour: body (circle), feet (two circles), connectors (lines)
    const float bodyR = PHYS_SIZE * 0.6f;
    const float footR = PHYS_SIZE * 0.25f;
    const float footDX = PHYS_SIZE * 0.35f;
    const float footDY = PHYS_SIZE * 0.45f;

    const vec2 footL = center + vec2(-footDX, footDY);
    const vec2 footRPos = center + vec2(+footDX, footDY);

    // Pure radial glow only (no visible rings or outlines)
    const float glowWidth = PHYS_SIZE * 0.7f;
    const ColorRGBA centerGlow(color.r, color.g, color.b, color.a * 0.45f);
    const ColorRGBA outerGlow(color.r, color.g, color.b, 0.0f);
    DrawRadialGlow(center, bodyR + glowWidth, centerGlow, outerGlow);
}


void FVisuals::DrawCircle(vec2 pos, float size, ColorRGBA color)
{
	const float step = 0.1f;
	const float firstX = cos(-step) * size + pos.x;
	const float firstY = sin(-step) * size + pos.y;

	vec2 prevPos = vec2(firstX, firstY);
	vec2 newPos = prevPos;

	for (float i = 0; i < 2 * pi; i += step)
	{
		const float x = cos(i) * size + pos.x;
		const float y = sin(i) * size + pos.y;

		prevPos = newPos;
		newPos = vec2(x, y);

		DrawLine(prevPos, newPos, color);
	}
}

void FVisuals::DrawBox(vec2 center, float size, ColorRGBA color)
{
	Graphics()->LinesBegin();
	const IGraphics::CLineItem array[4] =
	{
		IGraphics::CLineItem(center.x - size, center.y - size, center.x + size, center.y - size),
		IGraphics::CLineItem(center.x + size, center.y - size, center.x + size, center.y + size),
		IGraphics::CLineItem(center.x + size, center.y + size, center.x - size, center.y + size),
		IGraphics::CLineItem(center.x - size, center.y + size, center.x - size, center.y - size)
	};
	Graphics()->SetColor(color);
	Graphics()->LinesDraw(array, 4);
	Graphics()->LinesEnd();
}
