#ifndef GAME_CLIENT_COMPONENTS_CHEAT_FEATURES_BALANCEBOT_H
#define GAME_CLIENT_COMPONENTS_CHEAT_FEATURES_BALANCEBOT_H

#include <base/vmath.h>
#include <chrono>
#include <engine/console.h>
#include <game/client/component.h>
#include <optional>
#include <random>

class CBalanceBot : public CComponent
{
public:
	virtual int Sizeof() const override { return sizeof(*this); }
	virtual void OnReset() override
	{
		m_LegitMoveKeyDown = false;
		m_LegitLastDirection = 0;
	}

	void Balance();

	struct TargetInfo
	{
		int id;
		vec2 pos;
		float distance;

		TargetInfo(int id, const vec2 &pos, float distance) :
			id(id), pos(pos), distance(distance) {}
	};

private:

	bool m_LegitMoveKeyDown = false;
	int m_LegitLastDirection = 0;
	std::chrono::steady_clock::time_point m_LegitMoveActionTime;
	std::chrono::milliseconds m_LegitMoveDuration{0};


	std::mt19937 m_RandomEngine{std::random_device{}()};
	std::uniform_int_distribution<int> m_HoldTimeDistribution{259, 350}; 
	std::uniform_int_distribution<int> m_RestTimeDistribution{100, 220}; 

	struct PlayerState
	{
		vec2 pos;
		vec2 vel;
		vec2 predictedPos;
	};
	PlayerState getPlayerState(int ClientId, bool precise) const;
	bool IsValidVerticalPosition(const vec2 &targetPos, const vec2 &localPos) const;
	bool CanReachTarget(const vec2 &localPos, const vec2 &targetPos) const;
	std::optional<struct TargetInfo> FindTarget(int BotClientId, const vec2 &localPos, float maxDistance) const;
};

#endif
