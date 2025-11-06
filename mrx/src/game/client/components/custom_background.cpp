#include "custom_background.h"

#include <engine/console.h>
#include <engine/graphics.h>
#include <engine/shared/config.h>
#include <game/client/gameclient.h>

CCustomBackground::CCustomBackground()
{
	m_NeedReload = true;
	m_aCurrentPath[0] = '\0';
}

void CCustomBackground::OnInit()
{
	m_NeedReload = true;
}

void CCustomBackground::LoadTexture()
{
	// Unload old texture if exists
	if(m_Texture.IsValid())
	{
		Graphics()->UnloadTexture(&m_Texture);
		m_Texture = IGraphics::CTextureHandle();
	}

	// Load new texture if path is set
	if(g_Config.m_MRXCustomBgPath[0] != '\0')
	{
		m_Texture = Graphics()->LoadTexture(g_Config.m_MRXCustomBgPath, IStorage::TYPE_ALL, 0);
		str_copy(m_aCurrentPath, g_Config.m_MRXCustomBgPath);
		
		if(!m_Texture.IsValid() || m_Texture.IsNullTexture())
		{
			char aBuf[256];
			str_format(aBuf, sizeof(aBuf), "Failed to load custom background from '%s'", g_Config.m_MRXCustomBgPath);
			Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "custom_background", aBuf);
		}
		else
		{
			char aBuf[256];
			str_format(aBuf, sizeof(aBuf), "Successfully loaded custom background from '%s'", g_Config.m_MRXCustomBgPath);
			Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "custom_background", aBuf);
		}
	}
	else
	{
		m_aCurrentPath[0] = '\0';
	}
	
	m_NeedReload = false;
}

void CCustomBackground::UnloadTexture()
{
	if(m_Texture.IsValid())
	{
		Graphics()->UnloadTexture(&m_Texture);
		m_Texture = IGraphics::CTextureHandle();
	}
	m_aCurrentPath[0] = '\0';
}

void CCustomBackground::OnRender()
{
	// Check if we need to reload the texture
	if(m_NeedReload || str_comp(m_aCurrentPath, g_Config.m_MRXCustomBgPath) != 0)
	{
		LoadTexture();
	}

	// Only render if enabled and texture is valid (not null texture)
	if(!g_Config.m_MRXCustomBgEnable || !m_Texture.IsValid() || m_Texture.IsNullTexture())
		return;

	// This renders independently of ClShowQuads
	Graphics()->MapScreen(0.0f, 0.0f, Graphics()->ScreenWidth(), Graphics()->ScreenHeight());
	
	Graphics()->BlendNormal();
	Graphics()->TextureSet(m_Texture);
	Graphics()->QuadsBegin();
	
	const float Alpha = g_Config.m_MRXCustomBgAlpha / 100.0f;
	Graphics()->SetColor(1.0f, 1.0f, 1.0f, Alpha);
	
	IGraphics::CQuadItem QuadItem(0, 0, Graphics()->ScreenWidth(), Graphics()->ScreenHeight());
	Graphics()->QuadsDrawTL(&QuadItem, 1);
	
	Graphics()->QuadsEnd();
}
