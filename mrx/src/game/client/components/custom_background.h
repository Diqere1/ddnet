#ifndef GAME_CLIENT_COMPONENTS_CUSTOM_BACKGROUND_H
#define GAME_CLIENT_COMPONENTS_CUSTOM_BACKGROUND_H

#include <game/client/component.h>
#include <engine/graphics.h>

class CCustomBackground : public CComponent
{
private:
	IGraphics::CTextureHandle m_Texture;
	bool m_NeedReload;
	char m_aCurrentPath[256];

public:
	CCustomBackground();
	
	int Sizeof() const override { return sizeof(*this); }
	void OnRender() override;
	void OnInit() override;
	
	void LoadTexture();
	void UnloadTexture();
};

#endif
