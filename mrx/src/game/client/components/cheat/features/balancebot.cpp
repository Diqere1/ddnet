#include "balancebot.h"

#include <engine/shared/config.h>
#include <game/client/gameclient.h>
#include <limits>
#include <optional>

bool CBalanceBot::IsValidVerticalPosition(const vec2 &targetPos, const vec2 &localPos) const
{
	switch(g_Config.m_MRXBalanceBotVFilter)
	{
	case 0: return targetPos.y > localPos.y;
	case 1: return targetPos.y < localPos.y;
	case 2: [[fallthrough]];
	default: return true;
	}
}

bool CBalanceBot::CanReachTarget(const vec2 &localPos, const vec2 &targetPos) const
{
	if(!IsValidVerticalPosition(targetPos, localPos))
		return false;

	vec2 hookPos = localPos;
	vec2 scanDir = targetPos - localPos;

	vec2 exDirection = normalize(scanDir);
	vec2 oldPos = hookPos + exDirection * 28.0f * 1.5f;
	vec2 newPos = oldPos;

	bool DoBreak = false;
	do
	{
		oldPos = newPos;
		newPos = oldPos + exDirection * this->GameClient()->m_aTuning->m_HookFireSpeed;

		if(distance(hookPos, newPos) > this->GameClient()->m_aTuning->m_HookLength)
		{
			newPos = hookPos + normalize(newPos - hookPos) * this->GameClient()->m_aTuning->m_HookLength;
			DoBreak = true;
		}

		int TeleNr = 0;
		const int Hit = this->GameClient()->Collision()->IntersectLineTeleHook(oldPos, newPos, nullptr, nullptr, &TeleNr);

		vec2 closestPoint;
		if(closest_point_on_line(oldPos, newPos, targetPos, closestPoint))
		{
			if(distance(targetPos, closestPoint) < 28.0f + 2.0f)
				return true;
		}

		if(Hit)
			return false;

		newPos.x = round_to_int(newPos.x);
		newPos.y = round_to_int(newPos.y);

		if(oldPos == newPos)
			break;

		exDirection.x = round_to_int(exDirection.x * 256.0f) / 256.0f;
		exDirection.y = round_to_int(exDirection.y * 256.0f) / 256.0f;
	} while(!DoBreak);

	return true;
}

std::optional<CBalanceBot::TargetInfo> CBalanceBot::FindTarget(int BotClientId, const vec2 &localPos, float maxDistance) const
{
	float minDist = std::numeric_limits<float>::max();
	std::optional<CBalanceBot::TargetInfo> bestTarget;

	if(BotClientId < 0 || BotClientId >= MAX_CLIENTS)
		return std::nullopt;

	const CGameClient::CClientData &BotClient = this->GameClient()->m_aClients[BotClientId];

	if(!BotClient.m_Active || BotClient.m_Team == TEAM_SPECTATORS)
		return std::nullopt;

	for(int i = 0; i < MAX_CLIENTS; ++i)
	{
		const CGameClient::CClientData *pTargetClientData = &this->GameClient()->m_aClients[i];

		if(!pTargetClientData->m_Active ||
			i == BotClientId ||
			pTargetClientData->m_Team == TEAM_SPECTATORS ||
			this->GameClient()->IsOtherTeam(i))
		{
			continue;
		}

		const vec2 &playerPos = pTargetClientData->m_Predicted.m_Pos;

		const float dist = distance(playerPos, localPos);
		if(dist < minDist && dist < maxDistance)
		{
			minDist = dist;
			bestTarget.emplace(i, playerPos, dist);
		}
	}

	return bestTarget;
}

CBalanceBot::PlayerState CBalanceBot::getPlayerState(int ClientId, bool precise) const
{
	if(precise)
	{
		const auto &client = this->GameClient()->m_aClients[ClientId];
		const vec2 pos = client.m_Predicted.m_Pos;
		const vec2 vel = client.m_Predicted.m_Vel;
		return {pos, vel, pos + vel};
	}
	else
	{
		const auto &charInfo = this->GameClient()->m_Snap.m_aCharacters[ClientId];
		if(charInfo.m_Active)
		{
			const CNetObj_Character &pChar = charInfo.m_Cur;
			const vec2 pos = vec2(pChar.m_X, pChar.m_Y);
			const vec2 vel = vec2(pChar.m_VelX / 256.0f, pChar.m_VelY / 256.0f);
			return {pos, vel, pos + vel};
		}
		return {{0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f}};
	}
}

void CBalanceBot::Balance()
{
	const int BotClientId = this->GameClient()->m_aLocalIds[g_Config.m_ClDummy];
	if(BotClientId < 0 || BotClientId >= MAX_CLIENTS)
		return;

	const bool preciseMode = (g_Config.m_MRXBalanceBotPrecise == 1);
	const bool legitMode = (g_Config.m_MRXBalanceBotPrecise == 2) || g_Config.m_MRXBalanceBotLegit;
	const auto state = getPlayerState(BotClientId, preciseMode);
	if(state.pos.x == 0 && state.pos.y == 0)
		return;

	auto &dummyInputData = this->GameClient()->m_Controls.m_aInputData[g_Config.m_ClDummy];
	const int playerDirection = this->GameClient()->m_Controls.m_aInputData[0].m_Direction;

	auto ResetLegitBotState = [this]() {
		m_LegitMoveKeyDown = false;
		m_LegitLastDirection = 0;
	};

	dummyInputData.m_Direction = 0;

	const auto maybeTarget = FindTarget(BotClientId, state.predictedPos, this->GameClient()->m_aTuning->m_HookLength);
	if(!maybeTarget.has_value())
	{
		if(legitMode && playerDirection != 0)
			dummyInputData.m_Direction = playerDirection;
		ResetLegitBotState();
		return;
	}

	const auto &target = maybeTarget.value();



	if(target.distance < 185.0f)
	{
		const float horizontalDist = state.predictedPos.x - target.pos.x;
		const float horizontalDistAbs = std::abs(horizontalDist);

		if(!legitMode)
		{
			const float stopDistance = g_Config.m_MRXBalanceBotPrecise ? 0.25f : 0.5f;
			if(horizontalDistAbs > stopDistance)
				dummyInputData.m_Direction = (horizontalDist > 0) ? -1 : 1;
		}
		else
		{
			if(playerDirection != 0)
			{
				dummyInputData.m_Direction = playerDirection;
				ResetLegitBotState();
				return;
			}

			
			const float comfortZone = 5.5f;
			const float emergencyZone = 9.5f;
			

			const auto now = std::chrono::steady_clock::now();
			const int requiredDirection = (horizontalDist > 0) ? -1 : 1;

			
			if(horizontalDistAbs > emergencyZone)
			{
				dummyInputData.m_Direction = requiredDirection;
				
				ResetLegitBotState();
				m_LegitMoveActionTime = now;
			}
		
			else
			{
				
				if(m_LegitMoveKeyDown)
				{
					const auto timeSinceMoveStart = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_LegitMoveActionTime);

					
					if(timeSinceMoveStart >= m_LegitMoveDuration || horizontalDistAbs <= comfortZone)
					{
						
						m_LegitMoveKeyDown = false;
						m_LegitLastDirection = 0;
						m_LegitMoveActionTime = now;
						m_LegitMoveDuration = std::chrono::milliseconds(m_RestTimeDistribution(m_RandomEngine));
					}
					else
					{
					
						dummyInputData.m_Direction = m_LegitLastDirection;
					}
				}
				
				else
				{
					const auto timeSinceRestStart = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_LegitMoveActionTime);

					
					if(horizontalDistAbs > comfortZone && timeSinceRestStart >= m_LegitMoveDuration)
					{
						
						m_LegitMoveKeyDown = true;
						m_LegitLastDirection = requiredDirection;
						dummyInputData.m_Direction = requiredDirection;

						
						m_LegitMoveActionTime = now;
						m_LegitMoveDuration = std::chrono::milliseconds(m_HoldTimeDistribution(m_RandomEngine));
					}
				}
			}
		}
	}
	else
	{
		if(legitMode && playerDirection != 0)
			dummyInputData.m_Direction = playerDirection;
		ResetLegitBotState();
	}
}