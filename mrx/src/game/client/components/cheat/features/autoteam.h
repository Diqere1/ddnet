#pragma once

#include <game/client/component.h>
#include <engine/console.h>

class CAutoTeam : public CComponent
{
public:
	CAutoTeam();
	
	virtual int Sizeof() const override { return sizeof(*this); }
	void OnRender() override;
	void OnMessage(int MsgType, void *pRawMsg) override;
	void OnStateChange(int NewState, int OldState) override;
	void OnConsoleInit() override;
	
private:
	void ExecuteAutoTeam();
	void SendChatCommand(const char *pCommand);
	void SendChatCommandAsDummy(const char *pCommand);
	void ExecuteConsoleCommand(const char *pCommand);
	bool IsDummyConnected();
	void ConnectDummy();
	void InviteDummyToTeam();
	void ProcessDummyInviteState();
	const char *GetDummyName();
	bool IsServerFull();
	bool CanExecuteAutoTeam();
	
	int m_LastExecuteTime;
	bool m_HasExecutedOnJoin;
	bool m_IsWaitingForSlot;
	int m_LastServerCheckTime;
	
	// State machine for dummy invitation
	int m_DummyInviteState;
	int m_LastDummyCommandTime;
	
	// Console command handler
	static void ConAutoTeamExecute(IConsole::IResult *pResult, void *pUserData);
	static void ConAutoTeamDebug(IConsole::IResult *pResult, void *pUserData);
	static void ConAutoTeamReset(IConsole::IResult *pResult, void *pUserData);
};
