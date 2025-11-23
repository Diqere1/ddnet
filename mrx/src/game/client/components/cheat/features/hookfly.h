#ifndef GAME_CLIENT_COMPONENTS_CHEAT_FEATURES_HOOKFLY_H
#define GAME_CLIENT_COMPONENTS_CHEAT_FEATURES_HOOKFLY_H

#include <game/client/component.h>

class CHookFly : public CComponent
{
public:
	virtual int Sizeof() const override { return sizeof(*this); }
	
	void HandleHookFly();

private:
	// Core logic
	void HandleAutoHookFly();
	void HandleManualHookFly();
	
	// Helper functions
	bool GetPlayerAndDummyPositions(vec2 &PlayerPos, vec2 &DummyPos, int &DummyId);
	bool IsPlayerBelowDummy(const vec2 &PlayerPos, const vec2 &DummyPos);
	void SetDummyHookTarget(const vec2 &Direction);
	void ActivateDummyHook(bool Active);
	
	// State variables
	bool m_LastAutoHookState = false;
	bool m_LastManualHookState = false;
	bool m_AutoRehookQueued = false;
};

#endif
