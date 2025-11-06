#include <game/client/components/cheat/features/autoteam.h>
#include <game/client/component.h>
#include <game/client/gameclient.h>
#include <game/gamecore.h>
#include <engine/shared/config.h>
#include <game/client/components/chat.h>
#include <game/client/components/cheat/features/autodummyconnect.h>

CAutoTeam::CAutoTeam()
{
	m_LastExecuteTime = 0;
	m_HasExecutedOnJoin = false;
	m_IsWaitingForSlot = false;
	m_LastServerCheckTime = 0;
	
	// State machine for dummy invitation
	m_DummyInviteState = 0;
	m_LastDummyCommandTime = 0;
	
	// Debug: Show that constructor was called
	// Note: We can't use GameClient() here as it might not be initialized yet
}

void CAutoTeam::OnRender()
{
	// Debug: Show config status only when needed (removed spam)
	// static int ConfigDebugCounter = 0;
	// if(ConfigDebugCounter++ % 300 == 0) // Every 300 frames (5 seconds)
	// {
	//     IConsole *pConsole = GameClient()->Console();
	//     char aDebugMsg[128];
	//     str_format(aDebugMsg, sizeof(aDebugMsg), "echo \"Auto Team: Enable=%d, Mode=%d, State=%d, HasExecuted=%d\"", 
	//         g_Config.m_MRXAutoTeamEnable, g_Config.m_MRXAutoTeamMode, Client()->State(), m_HasExecutedOnJoin);
	//     pConsole->ExecuteLine(aDebugMsg);
	// }
	
	if(!g_Config.m_MRXAutoTeamEnable)
		return;
		
	if(Client()->State() != IClient::STATE_ONLINE)
		return;
		
	// Auto execute on join (mode 0)
	if(g_Config.m_MRXAutoTeamMode == 0 && !m_HasExecutedOnJoin)
	{
		// Execute immediately when online (no delay)
		// Check if we can execute (server not full)
		if(CanExecuteAutoTeam())
		{
			ExecuteAutoTeam();
			m_HasExecutedOnJoin = true;
			m_LastExecuteTime = time_get();
		}
	}
	
	// Handle dummy invitation state machine
	if(m_DummyInviteState != 0)
	{
		ProcessDummyInviteState();
	}
	
	// Debug for mode 1 (bind mode) - removed spam
	// Mode 1 only executes on bind or manual command, no automatic execution
}

void CAutoTeam::OnMessage(int MsgType, void *pRawMsg)
{
	// Can add message handling later if needed
}

void CAutoTeam::OnStateChange(int NewState, int OldState)
{
	// Reset state when changing client state
	if(NewState == IClient::STATE_ONLINE)
	{
		m_HasExecutedOnJoin = false;
		m_LastExecuteTime = 0;
		m_IsWaitingForSlot = false;
		m_LastServerCheckTime = 0;
		
		// Reset dummy invitation state
		m_DummyInviteState = 0;
		m_LastDummyCommandTime = 0;
	}
}

void CAutoTeam::OnConsoleInit()
{
	// Debug: Show that OnConsoleInit was called
	IConsole *pConsole = GameClient()->Console();
	pConsole->ExecuteLine("echo \"Auto Team: OnConsoleInit called - component initialized!\"");
	
	// Register command for manual execution
	pConsole->Register("mrx_auto_team_execute", "0", CFGFLAG_CLIENT, ConAutoTeamExecute, this, "Execute auto team join");
	
	// Register debug command
	pConsole->Register("mrx_auto_team_debug", "", CFGFLAG_CLIENT, ConAutoTeamDebug, this, "Show auto team debug info");
	
	// Register reset command
	pConsole->Register("mrx_auto_team_reset", "", CFGFLAG_CLIENT, ConAutoTeamReset, this, "Reset auto team execution flag");
}

void CAutoTeam::ExecuteAutoTeam()
{
	if(!g_Config.m_MRXAutoTeamEnable)
		return;
		
	// Check if server has space before executing
	if(!CanExecuteAutoTeam())
		return;
		
	char aTeamCommand[32];
	str_format(aTeamCommand, sizeof(aTeamCommand), "/team %d", g_Config.m_MRXAutoTeamNumber);
	
	// Execute commands in correct order:
	// 1. Send team command to chat
	SendChatCommand(aTeamCommand);
	
	// 2. Start state machine for the rest of the sequence
	m_DummyInviteState = -1; // Special state for main sequence
	m_LastDummyCommandTime = time_get();
}

void CAutoTeam::SendChatCommand(const char *pCommand)
{
	// Send command to chat (keep the / prefix for SendChat)
	const char *pChatCommand = pCommand;
	// Don't remove the / prefix - SendChat expects it
		
	GameClient()->m_Chat.SendChat(0, pChatCommand); // 0 = MODE_ALL
}

void CAutoTeam::ExecuteConsoleCommand(const char *pCommand)
{
	// Execute console command
	IConsole *pConsole = GameClient()->Console();
	pConsole->ExecuteLine(pCommand);
}

void CAutoTeam::SendChatCommandAsDummy(const char *pCommand)
{
	// Save current dummy state
	int OldDummyState = g_Config.m_ClDummy;
	
	// Temporarily switch to dummy mode to send command
	g_Config.m_ClDummy = 1;
	
	// Send the command (it will be sent from dummy)
	SendChatCommand(pCommand);
	
	// Restore original dummy state
	g_Config.m_ClDummy = OldDummyState;
}

bool CAutoTeam::IsDummyConnected()
{
	// Check if dummy is connected
	return Client()->DummyConnected();
}

void CAutoTeam::ConnectDummy()
{
	// Use Auto Dummy Connect component to connect dummy safely
	CAutoDummyConnect *pAutoDummyConnect = &GameClient()->m_AutoDummyConnect;
	if(pAutoDummyConnect)
	{
		pAutoDummyConnect->TryConnectDummy();
	}
	else
	{
		// Fallback to direct connection if component not available
		ExecuteConsoleCommand("dummy_connect");
	}
}

void CAutoTeam::InviteDummyToTeam()
{
	// Check if dummy is connected, if not - try to connect it
	if(!IsDummyConnected())
	{
		// Check if server has space before trying to connect dummy
		if(IsServerFull())
		{
			// Server is full, can't connect dummy - wait for someone to leave
			if(!m_IsWaitingForSlot)
			{
				m_IsWaitingForSlot = true;
			}
			return;
		}
		
		// Try to connect dummy using Auto Dummy Connect logic
		CAutoDummyConnect *pAutoDummyConnect = &GameClient()->m_AutoDummyConnect;
		if(pAutoDummyConnect)
		{
			bool Connected = pAutoDummyConnect->TryConnectDummy();
			if(!Connected)
			{
				// Server is full, can't connect dummy - wait for someone to leave
				return;
			}
		}
		
		// Wait a bit for dummy to connect, will retry on next frame
		return;
	}
	
	// Dummy is connected, start the invitation sequence
	m_DummyInviteState = 1; // Start with switching to player mode
	m_LastDummyCommandTime = time_get();
}

void CAutoTeam::ProcessDummyInviteState()
{
	int CurrentTime = time_get();
	int Delay = 200 * time_freq() / 1000; // 200ms delay between commands
	
	// Check if enough time has passed since last command
	if(CurrentTime - m_LastDummyCommandTime < Delay)
		return;
	
	switch(m_DummyInviteState)
	{
		case -1: // Main sequence: Send lock command
			SendChatCommand("/lock");
			if(g_Config.m_MRXAutoTeamInviteDummy)
			{
				// Check if dummy is connected, if not - try to connect it
				if(!IsDummyConnected())
				{
					// Try to connect dummy
					ConnectDummy();
					m_DummyInviteState = -2; // Wait for dummy to connect
				}
				else
				{
					m_DummyInviteState = 1; // Start dummy invitation sequence
				}
			}
			else
			{
				ExecuteConsoleCommand("cl_dummy 1");
				m_DummyInviteState = 0; // Finished
			}
			m_LastDummyCommandTime = CurrentTime;
			break;
			
		case -2: // Wait for dummy to connect
			if(IsDummyConnected())
			{
				m_DummyInviteState = 1; // Start dummy invitation sequence
			}
			else
			{
				// Keep trying to connect dummy
				ConnectDummy();
			}
			m_LastDummyCommandTime = CurrentTime;
			break;
			
		case 1: // Switch to player mode (to be able to invite)
			ExecuteConsoleCommand("cl_dummy 0");
			m_DummyInviteState = 2;
			m_LastDummyCommandTime = CurrentTime;
			break;
			
		case 2: // Send invite command (as player)
			{
				// Get dummy's real name
				const char *pDummyName = GetDummyName();
				if(pDummyName && pDummyName[0] != '\0')
				{
					char aInviteCommand[64];
					str_format(aInviteCommand, sizeof(aInviteCommand), "/invite %s", pDummyName);
					SendChatCommand(aInviteCommand);
				}
				else
				{
					// Fallback to generic dummy name
					SendChatCommand("/invite dummy");
				}
				m_DummyInviteState = 3;
				m_LastDummyCommandTime = CurrentTime;
			}
			break;
			
		case 3: // Send team command for dummy (to join team) - no need to switch to dummy mode
			{
				char aTeamCommand[32];
				str_format(aTeamCommand, sizeof(aTeamCommand), "/team %d", g_Config.m_MRXAutoTeamNumber);
				SendChatCommandAsDummy(aTeamCommand); // Send as dummy without switching
				m_DummyInviteState = 0; // Finished
			}
			break;
	}
}

const char *CAutoTeam::GetDummyName()
{
	// Get dummy's name from client
	if(Client()->DummyConnected())
	{
		// Try to get dummy's name from the client data
		// The dummy should be the second client (index 1) if connected
		for(int i = 0; i < MAX_CLIENTS; i++)
		{
			if(GameClient()->m_Snap.m_aCharacters[i].m_Active && i != GameClient()->m_Snap.m_LocalClientId)
			{
				// This should be the dummy
				return GameClient()->m_aClients[i].m_aName;
			}
		}
	}
	
	// Fallback: try to get dummy name from config
	if(g_Config.m_ClDummyName[0] != '\0')
	{
		return g_Config.m_ClDummyName;
	}
	
	// Last fallback
	return "dummy";
}

bool CAutoTeam::IsServerFull()
{
	// Считаем количество активных игроков
	int CurrentClients = 0;
	
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(GameClient()->m_Snap.m_aCharacters[i].m_Active)
			CurrentClients++;
	}
	
	// Используем простую проверку - если больше 60 клиентов, считаем сервер заполненным
	return CurrentClients >= 60;
}

bool CAutoTeam::CanExecuteAutoTeam()
{
	if(!g_Config.m_MRXAutoTeamEnable)
		return false;
		
	// Для режима auto (mode 0) не проверяем интервал - выполняем сразу
	if(g_Config.m_MRXAutoTeamMode == 0)
	{
		// Если сервер заполнен, ждем
		if(IsServerFull())
		{
			if(!m_IsWaitingForSlot)
			{
				m_IsWaitingForSlot = true;
			}
			return false;
		}
		
		// Если мы ждали места и оно освободилось
		if(m_IsWaitingForSlot)
		{
			m_IsWaitingForSlot = false;
		}
		
		return true;
	}
	
	// Для режима bind (mode 1) используем минимальный интервал (200ms)
	int CurrentTime = time_get();
	if(CurrentTime - m_LastServerCheckTime < 200 * time_freq() / 1000)
		return false;
		
	m_LastServerCheckTime = CurrentTime;
	
	// Если сервер заполнен, ждем
	if(IsServerFull())
	{
		if(!m_IsWaitingForSlot)
		{
			m_IsWaitingForSlot = true;
		}
		return false;
	}
	
	// Если мы ждали места и оно освободилось
	if(m_IsWaitingForSlot)
	{
		m_IsWaitingForSlot = false;
	}
	
	return true;
}

// Console command handler
void CAutoTeam::ConAutoTeamExecute(IConsole::IResult *pResult, void *pUserData)
{
	CAutoTeam *pAutoTeam = (CAutoTeam *)pUserData;
	pAutoTeam->ExecuteAutoTeam();
}

// Debug command handler
void CAutoTeam::ConAutoTeamDebug(IConsole::IResult *pResult, void *pUserData)
{
	CAutoTeam *pAutoTeam = (CAutoTeam *)pUserData;
	IConsole *pConsole = pAutoTeam->GameClient()->Console();
	
	char aDebugMsg[256];
	str_format(aDebugMsg, sizeof(aDebugMsg), "echo \"Auto Team Debug: Enable=%d, Mode=%d, Team=%d, InviteDummy=%d, ClientState=%d\"", 
		g_Config.m_MRXAutoTeamEnable, 
		g_Config.m_MRXAutoTeamMode, 
		g_Config.m_MRXAutoTeamNumber, 
		g_Config.m_MRXAutoTeamInviteDummy,
		pAutoTeam->Client()->State());
	pConsole->ExecuteLine(aDebugMsg);
}

// Reset command handler
void CAutoTeam::ConAutoTeamReset(IConsole::IResult *pResult, void *pUserData)
{
	CAutoTeam *pAutoTeam = (CAutoTeam *)pUserData;
	pAutoTeam->m_HasExecutedOnJoin = false;
	pAutoTeam->m_LastExecuteTime = 0;
	
	IConsole *pConsole = pAutoTeam->GameClient()->Console();
	pConsole->ExecuteLine("echo \"Auto Team: Reset execution flag - will execute on next check\"");
}
