#pragma once

#include <game/client/component.h>

class CAutoDummyConnect : public CComponent
{
public:
	CAutoDummyConnect();
	
	virtual int Sizeof() const override { return sizeof(*this); }
	void OnRender() override;
	void OnMessage(int MsgType, void *pRawMsg) override;
	void OnStateChange(int NewState, int OldState) override;
	
private:
	void CheckAndConnectDummy();
	bool IsServerFull();
	void ConnectDummy();
	
	int m_LastCheckTime;
	bool m_IsWaitingForSlot;
	bool m_HasTriedConnect;
	
public:
	// Public method for manual dummy connection (used by other components)
	bool TryConnectDummy();
};
