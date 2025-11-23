#include "arraylist.h"
#include <algorithm>
#include <engine/shared/config.h>
#include <game/client/gameclient.h>

namespace {
struct Feature
{
	const char *name;
	ColorRGBA color;
};

static const Feature FEATURES[] = {
	{"Move Fly Bot", ColorRGBA(0.0f, 0.8f, 1.0f, 1.0f)},
	{"Balance Bot", ColorRGBA(0.8f, 0.0f, 0.8f, 1.0f)},
	{"PseudoAled", ColorRGBA(0.0f, 1.0f, 0.0f, 1.0f)},
	{"AutoAled", ColorRGBA(1.0f, 0.0f, 0.0f, 1.0f)},
	{"AvoidFreeze", ColorRGBA(1.0f, 0.5f, 0.0f, 1.0f)},
	{"AutoPseudo", ColorRGBA(0.0f, 1.0f, 0.5f, 1.0f)},
	{"PixelWalk", ColorRGBA(0.2f, 0.8f, 0.6f, 1.0f)},
	{"Advanced DeepFly", ColorRGBA(0.6f, 0.2f, 0.8f, 1.0f)},
	{"AutoHookDummy", ColorRGBA(1.0f, 1.0f, 0.0f, 1.0f)},
	{"SlowWalk", ColorRGBA(0.8f, 0.6f, 0.4f, 1.0f)},
	{"TripleHookHolder", ColorRGBA(0.0f, 0.6f, 0.8f, 1.0f)},
	{"AutoTripleFly", ColorRGBA(0.8f, 0.2f, 0.2f, 1.0f)},
	{"DummyHookCursor", ColorRGBA(0.8f, 0.2f, 0.2f, 1.0f)},
	{"Aimbot", ColorRGBA(0.8f, 0.2f, 0.2f, 1.0f)},
	{"AutoHookFly", ColorRGBA(0.1f, 0.8f, 0.6f, 1.0f)},
	{"DummyThrow", ColorRGBA(0.1f, 0.8f, 0.6f, 1.0f)},
	{"DuoPseudo", ColorRGBA(0.0f, 0.2f, 1.0f, 1.0f)},
	{"ReverseCopy", ColorRGBA(0.0f, 0.2f, 1.0f, 1.0f)}};


} // namespace

void CArrayList::OnReset()
{
    m_X = Graphics()->ScreenWidth();
    m_Padding = 4.0f;
    m_EntryHeight = 20.0f;
    m_TextSize = 16.0f;

    // Инициализация флагов видимости (по умолчанию все пункты показываются)
    if(!m_VisInitialized)
{
    int Mask = g_Config.m_MRXArrayListMask;
    for(int i = 0; i < (int)EFeature::COUNT; ++i)
        m_aVisible[i] = (Mask >> i) & 1;
    m_VisInitialized = true;
}
}

bool CArrayList::GetFeatureVisible(EFeature Id) const
{
    int idx = static_cast<int>(Id);
    if(idx < 0 || idx >= (int)EFeature::COUNT)
        return true; // безопасно по умолчанию
    return m_aVisible[idx];
}

void CArrayList::SetFeatureVisible(EFeature Id, bool Visible)
{
    int idx = static_cast<int>(Id);
    if(idx < 0 || idx >= (int)EFeature::COUNT)
        return;
		m_aVisible[idx] = Visible;
		if(Visible)
			g_Config.m_MRXArrayListMask |= (1 << idx);
		else
			g_Config.m_MRXArrayListMask &= ~(1 << idx);
}

std::vector<CArrayList::ArrayListItem> CArrayList::GetEnabledFeatures() const
{
	std::vector<ArrayListItem> Features;

    // MoveFly Bot
    if(g_Config.m_MRXMoveflyBot && GetFeatureVisible(EFeature::MoveFlyBot))
        Features.push_back({FEATURES[0].name, FEATURES[0].color});

    // Balance Bot
    if(g_Config.m_MRXBalanceBot && GetFeatureVisible(EFeature::BalanceBot))
        Features.push_back({FEATURES[1].name, FEATURES[1].color});

    // PseudoAled
    if(g_Config.m_MRXDummyAled && GetFeatureVisible(EFeature::PseudoAled))
        Features.push_back({FEATURES[2].name, FEATURES[2].color});

    // AutoAled
    if(g_Config.m_MRXAutoAled && GetFeatureVisible(EFeature::AutoAled))
        Features.push_back({FEATURES[3].name, FEATURES[3].color});

    // AvoidFreeze
    if(g_Config.m_MRXAvoidEnable && GetFeatureVisible(EFeature::AvoidFreeze))
        Features.push_back({FEATURES[4].name, FEATURES[4].color});

    // AutoPseudo
    if(g_Config.m_MRXDummyPseudo && GetFeatureVisible(EFeature::AutoPseudo))
        Features.push_back({FEATURES[5].name, FEATURES[5].color});

    // PixelWalk
    if(g_Config.m_MRXPixelWalk && GetFeatureVisible(EFeature::PixelWalk))
        Features.push_back({FEATURES[6].name, FEATURES[6].color});

    // Advanced DeepFly
    if(g_Config.m_MRXAdvancedDeepFly && GetFeatureVisible(EFeature::AdvancedDeepFly))
        Features.push_back({FEATURES[7].name, FEATURES[7].color});

    // AutoHookDummy
    if(g_Config.m_MRXAutoHookDummy && GetFeatureVisible(EFeature::AutoHookDummy))
        Features.push_back({FEATURES[8].name, FEATURES[8].color});

    // SlowWalk
    if(g_Config.m_MRXSlowWalk && GetFeatureVisible(EFeature::SlowWalk))
        Features.push_back({FEATURES[9].name, FEATURES[9].color});

    // TripleHookHolder
    if(g_Config.m_MRXTripleHookHolder && GetFeatureVisible(EFeature::TripleHookHolder))
        Features.push_back({FEATURES[10].name, FEATURES[10].color});

    // AutoTripleFly
    if(g_Config.m_MRXAutotriplefly && GetFeatureVisible(EFeature::AutoTripleFly))
        Features.push_back({FEATURES[11].name, FEATURES[11].color});

    // DummyHookCursor
    if(g_Config.m_MRXDummyHookCursor && GetFeatureVisible(EFeature::DummyHookCursor))
        Features.push_back({FEATURES[12].name, FEATURES[12].color});

    // Aimbot
    if(g_Config.m_MRXAimbotEnable && GetFeatureVisible(EFeature::Aimbot))
        Features.push_back({FEATURES[13].name, FEATURES[13].color});

    // AutoHookFly
    if(g_Config.m_MRXHookFlyAuto && GetFeatureVisible(EFeature::AutoHookFly))
        Features.push_back({FEATURES[14].name, FEATURES[14].color});

    // DummyThrow
    if(g_Config.m_MRXThrowDummy && GetFeatureVisible(EFeature::DummyThrow))
        Features.push_back({FEATURES[15].name, FEATURES[15].color});

	//DuoPseudo
	if(g_Config.m_MRXDuoPseudo && GetFeatureVisible(EFeature::DuoPseudo))
		Features.push_back({FEATURES[16].name, FEATURES[16].color});

	if(g_Config.m_MRXReverseDummyCopyMoves && GetFeatureVisible(EFeature::ReverseCopy))
		Features.push_back({FEATURES[17].name, FEATURES[17].color});

	// Sort alphabetically
	std::sort(Features.begin(), Features.end());
	return Features;
}

void CArrayList::RenderFeature(float X, float Y, const char *Name, const ColorRGBA &Color, bool AlignRight) const
{
	float TextWidth = Ui()->TextRender()->TextWidth(m_TextSize, Name, -1);
	float BoxWidth = TextWidth + m_Padding * 2;
	float BoxHeight = m_TextSize + m_Padding * 2;

	float RenderX = AlignRight ? X - BoxWidth : X;

	// Main background
	CUIRect MainBg = {RenderX, Y, BoxWidth, BoxHeight};
	MainBg.Draw(ColorRGBA(0.1f, 0.1f, 0.1f, 0.85f), AlignRight ? IGraphics::CORNER_R : IGraphics::CORNER_L, 3.0f);

	// Accent line
	if(g_Config.m_MRXArrayListLine)
	{
		Graphics()->TextureClear();
		Graphics()->QuadsBegin();
		Graphics()->SetColor(Color.r, Color.g, Color.b, 1.0f);
		IGraphics::CQuadItem QuadLine(AlignRight ? RenderX : RenderX + BoxWidth - 2.0f, Y, 2.0f, BoxHeight);
		Graphics()->QuadsDrawTL(&QuadLine, 1);
		Graphics()->QuadsEnd();
	}

	// Feature background
	if(g_Config.m_MRXArrayListGradient)
	{
		// Gradient overlay
		Graphics()->TextureClear();
		Graphics()->QuadsBegin();
		Graphics()->SetColor4(
			ColorRGBA(Color.r, Color.g, Color.b, 0.1f),
			ColorRGBA(Color.r, Color.g, Color.b, 0.1f),
			ColorRGBA(Color.r, Color.g, Color.b, 0.0f),
			ColorRGBA(Color.r, Color.g, Color.b, 0.0f)
		);
		IGraphics::CFreeformItem Freeform(
			RenderX, Y,
			RenderX + BoxWidth, Y,
			RenderX, Y + BoxHeight,
			RenderX + BoxWidth, Y + BoxHeight
		);
		Graphics()->QuadsDrawFreeform(&Freeform, 1);
		Graphics()->QuadsEnd();
	}

	// Text with shadow
	float TextRenderX = AlignRight ? RenderX + BoxWidth - m_Padding - TextWidth : RenderX + m_Padding;
	Ui()->TextRender()->TextColor(0.0f, 0.0f, 0.0f, 0.5f);
	Ui()->TextRender()->Text(TextRenderX + 1, Y + m_Padding + 1, m_TextSize, Name, -1.0f);
	Ui()->TextRender()->TextColor(1.0f, 1.0f, 1.0f, 1.0f);
	Ui()->TextRender()->Text(TextRenderX, Y + m_Padding, m_TextSize, Name, -1.0f);
}

void CArrayList::OnRender()
{
	if(!g_Config.m_MRXArrayList)
		return;

	// Показывать ArrayList только когда играем на сервере
	if(Client()->State() != IClient::STATE_ONLINE && !m_IsPreview)
		return;

	auto Features = GetEnabledFeatures();
	if(Features.empty())
		return;

	float Points[4];
	Graphics()->GetScreen(Points, Points + 1, Points + 2, Points + 3);

	float ScreenWidth = Graphics()->ScreenWidth();
	float ScreenHeight = Graphics()->ScreenHeight();
	Graphics()->MapScreen(0.0f, 0.0f, ScreenWidth, ScreenHeight);

	// Вычисляем базовую высоту для ArrayList
	float BoxHeight = m_TextSize + m_Padding * 2;
	float TotalHeight = Features.size() * BoxHeight;
	
	// Используем конфигурационные переменные для позиционирования
	float Y;
	float X = 0.0f;
	bool AlignRight = false;

	if(g_Config.m_MRXArrayListX || g_Config.m_MRXArrayListY)
	{
		X = (g_Config.m_MRXArrayListX / 10000.0f) * ScreenWidth;
		Y = (g_Config.m_MRXArrayListY / 10000.0f) * ScreenHeight;

		AlignRight = X > ScreenWidth / 2.0f;

		float MaxWidth = 0.0f;
		for(const auto &Feature : Features)
		{
			float Width = Ui()->TextRender()->TextWidth(m_TextSize, Feature.name, -1.0f) + m_Padding * 2;
			MaxWidth = std::max(MaxWidth, Width);
		}
		
		if(AlignRight)
			X = std::clamp(X, MaxWidth, ScreenWidth);
		else
			X = std::clamp(X, 0.0f, ScreenWidth - MaxWidth);

		Y = std::clamp(Y, 0.0f, ScreenHeight - TotalHeight);
	}
	else
	{
		Y = (ScreenHeight - TotalHeight) / 2;
	}

	for(const auto &Feature : Features)
	{
		RenderFeature(X, Y, Feature.name, Feature.color, AlignRight);
		Y += BoxHeight;
	}

	// Восстанавливаем состояние экрана
	Graphics()->MapScreen(Points[0], Points[1], Points[2], Points[3]);
}