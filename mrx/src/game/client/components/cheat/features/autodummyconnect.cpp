#include <game/client/components/cheat/features/autodummyconnect.h>
#include <game/client/component.h>
#include <game/client/gameclient.h>
#include <game/gamecore.h>
#include <engine/shared/config.h>

CAutoDummyConnect::CAutoDummyConnect()
{
	m_LastCheckTime = 0;
	m_IsWaitingForSlot = false;
	m_HasTriedConnect = false;
}

void CAutoDummyConnect::OnRender()
{
	if(!g_Config.m_MRXAutoDummyConnect)
		return;
		
	if(Client()->State() != IClient::STATE_ONLINE)
		return;
		
	// Проверяем, нужно ли подключить дамми
	CheckAndConnectDummy();
}

void CAutoDummyConnect::OnMessage(int MsgType, void *pRawMsg)
{
	// Можно добавить обработку сообщений позже, если понадобится
}

void CAutoDummyConnect::OnStateChange(int NewState, int OldState)
{
	// Сбрасываем состояние при смене состояния клиента
	if(NewState == IClient::STATE_ONLINE)
	{
		m_IsWaitingForSlot = false;
		m_HasTriedConnect = false;
		m_LastCheckTime = 0;
	}
}

void CAutoDummyConnect::CheckAndConnectDummy()
{
	if(!g_Config.m_MRXAutoDummyConnect)
		return;
		
	// Проверяем интервал проверки
	int CurrentTime = time_get();
	if(CurrentTime - m_LastCheckTime < g_Config.m_MRXAutoDummyConnectCheckInterval * time_freq() / 1000)
		return;
		
	m_LastCheckTime = CurrentTime;
	
	// Проверяем, есть ли уже дамми подключенный через Client()->DummyConnected()
	if(Client()->DummyConnected())
	{
		m_IsWaitingForSlot = false;
		m_HasTriedConnect = false;
		return;
	}
	
	// Если сервер заполнен, ждем
	if(IsServerFull())
	{
		if(!m_IsWaitingForSlot)
		{
			m_IsWaitingForSlot = true;
			m_HasTriedConnect = false;
		}
		return;
	}
	
	// Если мы ждали места и оно освободилось, или это первая попытка
	if(!m_HasTriedConnect)
	{
		ConnectDummy();
		m_HasTriedConnect = true;
		m_IsWaitingForSlot = false;
	}
}

bool CAutoDummyConnect::IsServerFull()
{
	// Считаем количество активных игроков
	int CurrentClients = 0;
	
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(GameClient()->m_Snap.m_aCharacters[i].m_Active)
			CurrentClients++;
	}
	
	// Используем простую проверку - если больше 60 клиентов, считаем сервер заполненным
	// Это безопасное значение для большинства серверов
	return CurrentClients >= 60;
}

void CAutoDummyConnect::ConnectDummy()
{
	IConsole *pConsole = GameClient()->Console();
	
	// Выполняем команды подключения дамми
	pConsole->ExecuteLine("dummy_connect");
	pConsole->ExecuteLine("cl_dummy 1");
	
	// Отладочная информация
	pConsole->ExecuteLine("echo \"Auto Dummy Connect: Attempting to connect dummy\"");
}

bool CAutoDummyConnect::TryConnectDummy()
{
	// Проверяем, есть ли уже дамми подключенный через Client()->DummyConnected()
	if(Client()->DummyConnected())
	{
		return true; // Дамми уже подключен
	}
	
	// Если сервер заполнен, не подключаем
	if(IsServerFull())
	{
		return false; // Сервер заполнен, подключение невозможно
	}
	
	// Подключаем дамми
	ConnectDummy();
	return true; // Попытка подключения выполнена
}
