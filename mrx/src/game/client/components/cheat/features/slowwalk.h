#pragma once
#include <game/client/component.h>

class CSlowWalk :public CComponent
{
public:
	void OnRender() override;
	void SlowWalk();
	void HandleSlowWalk();
	void HandleSlowWalkFor(int Dummy);
	virtual int Sizeof() const override { return sizeof(*this); }
	int FunStepCounter = 0;
	int PixelWalkCounter = 0;

};