#include "moveflybot.h"
#include <cmath>
#include <engine/shared/config.h>
#include <game/client/gameclient.h>

void CMoveFlyBot::MoveFly()
{
	if(!g_Config.m_MRXMoveflyBot)
	{
		// Если MoveFlyBot отключен, сбрасываем направление движения дамми
		GameClient()->m_DummyInput.m_Direction = 0;
		GameClient()->m_Controls.m_aInputData[!g_Config.m_ClDummy].m_Direction = 0;
		return;
	}

	int finalDirection = 0; // Итоговое направление для дамми. 0 = стоять на месте.

	// Флаг, который определяет, нужно ли вообще выполнять логику движения
	bool shouldCalculateMovement = true;

	// Проверяем, включен ли новый режим "двигаться только если игрок движется"
	if(g_Config.m_MRXMoveflyBotDirect)
	{
		// В этом режиме проверяем движение основного игрока
		const int playerDirection = GameClient()->m_Controls.m_aInputData[g_Config.m_ClDummy].m_Direction;
		if(playerDirection == 0)
		{
			// Если игрок стоит на месте, то и дамми не должен двигаться
			shouldCalculateMovement = false;
		}
	}

	// Если условия для движения выполнены (старый режим или новый режим с движущимся игроком)
	if(shouldCalculateMovement)
	{
		const vec2 localPos = GameClient()->m_LocalCharacterPos;
		const vec2 dummyPos = GameClient()->m_aClients[GameClient()->m_aLocalIds[!g_Config.m_ClDummy]].m_Predicted.m_Pos;
		const float horizontalDist = dummyPos.x - localPos.x;

		// Основная логика следования за игроком
		if(std::abs(horizontalDist) > g_Config.m_MRXMoveflyBotDistance)
		{
			// Если расстояние больше заданного, вычисляем направление
			finalDirection = (horizontalDist > 0) ? -1 : 1;
		}
		// Если расстояние в пределах нормы, finalDirection остается 0, и дамми остановится
	}

	// Применяем вычисленное направление к дамми
	GameClient()->m_DummyInput.m_Direction = finalDirection;
	GameClient()->m_Controls.m_aInputData[!g_Config.m_ClDummy].m_Direction = finalDirection;
}