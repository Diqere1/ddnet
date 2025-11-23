#include "tas.h"

#include <game/client/gameclient.h>
#include <game/client/components/controls.h>
#include <engine/shared/config.h>
#include <engine/graphics.h>
#include <engine/textrender.h>
#include <base/color.h>

CTas::CTas()
{
	m_State = STATE_NONE;
	m_CurrentTick = 0;
	m_StartTick = 0;
	m_TickAccumulator = 0.0f;
	m_SnapshotInterval = 50; // Снапшот каждые 50 тиков (1 секунда)
	m_pTasWorld = nullptr;
	m_TasWorldInitialized = false;
	m_LocalClientId = -1;
}

void CTas::OnInit()
{
	// Инициализация при старте
	m_LocalClientId = GameClient()->m_aLocalIds[g_Config.m_ClDummy];
}

void CTas::OnReset()
{
	// Останавливаем все активности при ресете карты
	if(m_State == STATE_RECORDING || m_State == STATE_PLAYING)
	{
		StopRecording();
		StopPlayback();
	}
	
	DestroyTasWorld();
}

void CTas::OnRelease()
{
	DestroyTasWorld();
}

void CTas::OnConsoleInit()
{
	Console()->Register("tas_record", "", CFGFLAG_CLIENT, ConRecord, this, "Start/stop recording");
	Console()->Register("tas_stop", "", CFGFLAG_CLIENT, ConStop, this, "Stop recording or playback");
	Console()->Register("tas_play", "", CFGFLAG_CLIENT, ConPlay, this, "Play recording");
	Console()->Register("tas_pause", "", CFGFLAG_CLIENT, ConPause, this, "Pause/unpause");
	Console()->Register("tas_rewind", "?i[ticks]", CFGFLAG_CLIENT, ConRewind, this, "Rewind N ticks (default: from config)");
	Console()->Register("tas_forward", "?i[ticks]", CFGFLAG_CLIENT, ConForward, this, "Forward N ticks (default: from config)");
	Console()->Register("tas_step", "?i[direction]", CFGFLAG_CLIENT, ConStep, this, "Step one frame (1=forward, -1=backward)");
	Console()->Register("tas_save", "s[filename]", CFGFLAG_CLIENT, ConSave, this, "Save TAS to file");
	Console()->Register("tas_load", "s[filename]", CFGFLAG_CLIENT, ConLoad, this, "Load TAS from file");
	Console()->Register("tas_clear", "", CFGFLAG_CLIENT, ConClear, this, "Clear current recording");
	Console()->Register("tas_apply", "", CFGFLAG_CLIENT, ConApply, this, "Apply recorded inputs to main player");
}

void CTas::OnRender()
{
	if(!IsActive())
		return;
	
	// Обработка config переменных
	if(g_Config.m_MRXTasPause && m_State == STATE_PLAYING)
	{
		m_State = STATE_PAUSED;
		g_Config.m_MRXTasPause = 0;
	}
	else if(g_Config.m_MRXTasPause && m_State == STATE_PAUSED)
	{
		m_State = STATE_PLAYING;
		g_Config.m_MRXTasPause = 0;
	}
	
	// Обработка rewind/forward из конфига
	if(g_Config.m_MRXTasRewind)
	{
		Rewind(g_Config.m_MRXTasTickControlTicks);
		g_Config.m_MRXTasRewind = 0;
	}
	
	if(g_Config.m_MRXTasForward)
	{
		Forward(g_Config.m_MRXTasTickControlTicks);
		g_Config.m_MRXTasForward = 0;
	}
	
	// Обработка load/record из конфига
	if(g_Config.m_MRXLoadReplay)
	{
		StartPlayback();
		g_Config.m_MRXLoadReplay = 0;
	}
	
	if(g_Config.m_MRXRecordReplay && m_State == STATE_NONE)
	{
		StartRecording();
	}
	else if(!g_Config.m_MRXRecordReplay && m_State == STATE_RECORDING)
	{
		StopRecording();
	}
	
	if(g_Config.m_MRXTasRespawn)
	{
		Clear();
		g_Config.m_MRXTasRespawn = 0;
	}
	
	// Основная логика
	switch(m_State)
	{
	case STATE_RECORDING:
		RecordFrame();
		break;
		
	case STATE_PLAYING:
		{
			// Обновление TPS
			float TPS = (float)g_Config.m_MRXTasTps;
			if(TPS <= 0.0f)
				TPS = 50.0f;
			
			float DeltaTime = Client()->RenderFrameTime();
			m_TickAccumulator += DeltaTime * TPS;
			
			// Ограничение на количество тиков за кадр
			int MaxTicksPerFrame = 20;
			int TicksToSimulate = minimum((int)m_TickAccumulator, MaxTicksPerFrame);
			
			for(int i = 0; i < TicksToSimulate; i++)
			{
				if(m_CurrentTick >= (int)m_vRecording.size())
				{
					// Конец записи
					if(m_CurrentTick > 0)
						m_CurrentTick = (int)m_vRecording.size() - 1;
					break;
				}
				
				SimulateTick();
				m_TickAccumulator -= 1.0f;
			}
		}
		break;
		
	case STATE_PAUSED:
		// В паузе ничего не делаем
		break;
		
	default:
		break;
	}
	
	// Рендер визуализации
	if(IsActive())
	{
		RenderPath();
		RenderUI();
	}
}

void CTas::StartRecording()
{
	if(m_State == STATE_RECORDING)
	{
		Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "tas", "Already recording");
		return;
	}
	
	// Очистить предыдущую запись
	m_vRecording.clear();
	m_vSnapshots.clear();
	
	m_State = STATE_RECORDING;
	m_CurrentTick = 0;
	m_StartTick = Client()->GameTick(g_Config.m_ClDummy);
	m_LocalClientId = GameClient()->m_aLocalIds[g_Config.m_ClDummy];
	
	Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "tas", "Recording started");
}

void CTas::StopRecording()
{
	if(m_State != STATE_RECORDING)
		return;
	
	m_State = STATE_NONE;
	
	char aBuf[128];
	str_format(aBuf, sizeof(aBuf), "Recording stopped. Frames: %d", (int)m_vRecording.size());
	Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "tas", aBuf);
}

void CTas::StartPlayback()
{
	if(m_vRecording.empty())
	{
		Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "tas", "No recording to play. Use tas_record first or tas_load <file>");
		return;
	}
	
	if(m_State == STATE_PLAYING)
	{
		Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "tas", "Already playing");
		return;
	}
	
	// Инициализация TAS World если нужно
	if(!m_TasWorldInitialized)
	{
		InitTasWorld();
	}
	
	m_State = STATE_PLAYING;
	m_CurrentTick = 0;
	m_TickAccumulator = 0.0f;
	
	char aBuf[128];
	str_format(aBuf, sizeof(aBuf), "Playback started. Frames: %d", (int)m_vRecording.size());
	Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "tas", aBuf);
}

void CTas::StopPlayback()
{
	if(m_State != STATE_PLAYING && m_State != STATE_PAUSED)
		return;
	
	m_State = STATE_NONE;
	Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "tas", "Playback stopped");
}

void CTas::TogglePause()
{
	if(m_State == STATE_PLAYING)
	{
		m_State = STATE_PAUSED;
		char aBuf[128];
		str_format(aBuf, sizeof(aBuf), "Paused at tick %d/%d", m_CurrentTick, (int)m_vRecording.size());
		Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "tas", aBuf);
	}
	else if(m_State == STATE_PAUSED)
	{
		m_State = STATE_PLAYING;
		Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "tas", "Resumed");
	}
}

void CTas::RecordFrame()
{
	// Получить текущий инпут из Controls
	const CNetObj_PlayerInput &Input = GameClient()->m_Controls.m_aInputData[g_Config.m_ClDummy];
	
	// Получить текущую позицию и скорость игрока
	vec2 Pos = vec2(0, 0);
	vec2 Vel = vec2(0, 0);
	int Flags = 0;
	
	// Из predicted character
	if(GameClient()->m_aLocalIds[g_Config.m_ClDummy] >= 0)
	{
		Pos = GameClient()->m_PredictedChar.m_Pos;
		Vel = GameClient()->m_PredictedChar.m_Vel;
		
		// Флаги состояния
		if(GameClient()->m_Snap.m_aCharacters[GameClient()->m_aLocalIds[g_Config.m_ClDummy]].m_Active)
		{
			// TODO: Добавить флаги freeze, ground и т.д.
		}
	}
	
	// Создать кадр
	STasFrame Frame;
	Frame.m_Input = Input;
	Frame.m_Tick = m_CurrentTick;
	Frame.m_Pos = Pos;
	Frame.m_Vel = Vel;
	Frame.m_Flags = Flags;
	
	// Добавить в запись
	m_vRecording.push_back(Frame);
	
	// Создать снапшот если нужно
	if(m_CurrentTick > 0 && m_CurrentTick % m_SnapshotInterval == 0)
	{
		CreateSnapshot();
	}
	
	m_CurrentTick++;
}

void CTas::SimulateTick()
{
	// Пока простая версия - просто инкремент тика
	// TODO: Полная симуляция в TasWorld с применением инпута
	m_CurrentTick++;
}

void CTas::InitTasWorld()
{
	// TODO: Создать копию prediction world для симуляции
	// Пока заглушка
	m_TasWorldInitialized = true;
	Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "tas", "TasWorld initialized (stub)");
}

void CTas::DestroyTasWorld()
{
	if(m_pTasWorld)
	{
		delete m_pTasWorld;
		m_pTasWorld = nullptr;
	}
	m_TasWorldInitialized = false;
}

void CTas::CreateSnapshot()
{
	STasSnapshot Snapshot;
	Snapshot.m_Tick = m_CurrentTick;
	Snapshot.m_FrameIndex = m_vRecording.size() - 1;
	m_vSnapshots.push_back(Snapshot);
}

void CTas::RestoreSnapshot(int Tick)
{
	// Найти ближайший снапшот <= Tick
	STasSnapshot *pBestSnapshot = nullptr;
	for(auto &Snapshot : m_vSnapshots)
	{
		if(Snapshot.m_Tick <= Tick)
		{
			if(!pBestSnapshot || Snapshot.m_Tick > pBestSnapshot->m_Tick)
				pBestSnapshot = &Snapshot;
		}
	}
	
	if(pBestSnapshot)
	{
		m_CurrentTick = pBestSnapshot->m_Tick;
		// TODO: Восстановить состояние TasWorld
	}
	else
	{
		m_CurrentTick = 0;
	}
}

void CTas::Rewind(int Ticks)
{
	if(m_vRecording.empty())
		return;
	
	int TargetTick = maximum(0, m_CurrentTick - Ticks);
	
	// Используем снапшоты если есть
	RestoreSnapshot(TargetTick);
	
	// Досимулируем до нужного тика если надо
	while(m_CurrentTick < TargetTick && m_CurrentTick < (int)m_vRecording.size())
	{
		SimulateTick();
	}
	
	char aBuf[128];
	str_format(aBuf, sizeof(aBuf), "Rewinded to tick %d", m_CurrentTick);
	Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "tas", aBuf);
}

void CTas::Forward(int Ticks)
{
	if(m_vRecording.empty())
		return;
	
	int TargetTick = minimum((int)m_vRecording.size(), m_CurrentTick + Ticks);
	
	while(m_CurrentTick < TargetTick)
	{
		SimulateTick();
	}
	
	char aBuf[128];
	str_format(aBuf, sizeof(aBuf), "Forwarded to tick %d", m_CurrentTick);
	Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "tas", aBuf);
}

void CTas::StepForward()
{
	Forward(1);
}

void CTas::StepBackward()
{
	Rewind(1);
}

void CTas::Clear()
{
	m_vRecording.clear();
	m_vSnapshots.clear();
	m_CurrentTick = 0;
	m_State = STATE_NONE;
	Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "tas", "Recording cleared");
}

bool CTas::SaveToFile(const char *pFilename)
{
	if(m_vRecording.empty())
	{
		Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "tas", "No recording to save");
		return false;
	}
	
	// Формируем полный путь к файлу
	char aPath[IO_MAX_PATH_LENGTH];
	str_format(aPath, sizeof(aPath), "tas/%s", pFilename);
	if(!str_endswith(aPath, ".tas"))
		str_append(aPath, ".tas", sizeof(aPath));
	
	// Открываем файл для записи
	IOHANDLE File = Storage()->OpenFile(aPath, IOFLAG_WRITE, IStorage::TYPE_SAVE);
	if(!File)
	{
		char aBuf[256];
		str_format(aBuf, sizeof(aBuf), "Failed to open file: %s", aPath);
		Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "tas", aBuf);
		return false;
	}
	
	// Заголовок файла
	const char aMagic[8] = "DDNTAS";
	io_write(File, aMagic, 8);
	
	int Version = 1;
	io_write(File, &Version, sizeof(Version));
	
	int NumFrames = (int)m_vRecording.size();
	io_write(File, &NumFrames, sizeof(NumFrames));
	
	// Записываем кадры
	for(const auto &Frame : m_vRecording)
	{
		io_write(File, &Frame, sizeof(STasFrame));
	}
	
	io_close(File);
	
	char aBuf[256];
	str_format(aBuf, sizeof(aBuf), "Saved %d frames to %s", NumFrames, aPath);
	Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "tas", aBuf);
	
	return true;
}

bool CTas::LoadFromFile(const char *pFilename)
{
	// Формируем полный путь
	char aPath[IO_MAX_PATH_LENGTH];
	str_format(aPath, sizeof(aPath), "tas/%s", pFilename);
	if(!str_endswith(aPath, ".tas"))
		str_append(aPath, ".tas", sizeof(aPath));
	
	// Открываем файл
	IOHANDLE File = Storage()->OpenFile(aPath, IOFLAG_READ, IStorage::TYPE_ALL);
	if(!File)
	{
		char aBuf[256];
		str_format(aBuf, sizeof(aBuf), "Failed to open file: %s", aPath);
		Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "tas", aBuf);
		return false;
	}
	
	// Читаем заголовок
	char aMagic[8];
	io_read(File, aMagic, 8);
	if(mem_comp(aMagic, "DDNTAS", 6) != 0)
	{
		Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "tas", "Invalid file format");
		io_close(File);
		return false;
	}
	
	int Version = 0;
	io_read(File, &Version, sizeof(Version));
	if(Version != 1)
	{
		char aBuf[128];
		str_format(aBuf, sizeof(aBuf), "Unsupported version: %d", Version);
		Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "tas", aBuf);
		io_close(File);
		return false;
	}
	
	int NumFrames = 0;
	io_read(File, &NumFrames, sizeof(NumFrames));
	
	// Очищаем текущую запись
	m_vRecording.clear();
	m_vSnapshots.clear();
	
	// Читаем кадры
	for(int i = 0; i < NumFrames; i++)
	{
		STasFrame Frame;
		if(io_read(File, &Frame, sizeof(STasFrame)) != sizeof(STasFrame))
		{
			Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "tas", "Error reading frames");
			io_close(File);
			return false;
		}
		m_vRecording.push_back(Frame);
	}
	
	io_close(File);
	
	m_CurrentTick = 0;
	m_State = STATE_NONE;
	
	char aBuf[256];
	str_format(aBuf, sizeof(aBuf), "Loaded %d frames from %s", NumFrames, aPath);
	Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "tas", aBuf);
	
	return true;
}

void CTas::RenderPath()
{
	if(!g_Config.m_MRXTasDrawPath)
		return;
	
	if(m_vRecording.size() < 2)
		return;
	
	ColorRGBA Color = color_cast<ColorRGBA>(ColorHSLA(g_Config.m_MRXTasDrawPathColor));
	
	Graphics()->TextureClear();
	Graphics()->LinesBegin();
	Graphics()->SetColor(Color);
	
	// Рисуем линии между позициями
	for(size_t i = 1; i < m_vRecording.size(); i++)
	{
		vec2 p0 = m_vRecording[i-1].m_Pos;
		vec2 p1 = m_vRecording[i].m_Pos;
		
		IGraphics::CLineItem Line(p0.x, p0.y, p1.x, p1.y);
		Graphics()->LinesDraw(&Line, 1);
	}
	
	Graphics()->LinesEnd();
	
	// Рисуем текущую позицию
	if(m_CurrentTick >= 0 && m_CurrentTick < (int)m_vRecording.size())
	{
		vec2 CurrentPos = m_vRecording[m_CurrentTick].m_Pos;
		
		Graphics()->QuadsBegin();
		Graphics()->SetColor(1.0f, 0.0f, 0.0f, 1.0f);
		IGraphics::CQuadItem Quad(CurrentPos.x, CurrentPos.y, 20.0f, 20.0f);
		Graphics()->QuadsDrawTL(&Quad, 1);
		Graphics()->QuadsEnd();
	}
}

void CTas::RenderUI()
{
	if(!IsActive())
		return;
	
	// UI в верхнем левом углу
	float Width = 300.0f * Graphics()->ScreenAspect();
	float Height = 300.0f;
	float x = 10.0f;
	float y = 10.0f;
	
	char aBuf[256];
	
	// Состояние
	const char *pState = "Unknown";
	switch(m_State)
	{
	case STATE_RECORDING: pState = "Recording"; break;
	case STATE_PLAYING: pState = "Playing"; break;
	case STATE_PAUSED: pState = "Paused"; break;
	case STATE_NONE: pState = "Idle"; break;
	}
	
	str_format(aBuf, sizeof(aBuf), "TAS: %s", pState);
	TextRender()->Text(x, y, 12.0f, aBuf, -1.0f);
	y += 15.0f;
	
	// Прогресс
	str_format(aBuf, sizeof(aBuf), "Frame: %d / %d", m_CurrentTick, (int)m_vRecording.size());
	TextRender()->Text(x, y, 12.0f, aBuf, -1.0f);
	y += 15.0f;
	
	// TPS
	str_format(aBuf, sizeof(aBuf), "TPS: %d", g_Config.m_MRXTasTps);
	TextRender()->Text(x, y, 12.0f, aBuf, -1.0f);
	y += 15.0f;
	
	// Позиция если есть
	if(m_CurrentTick >= 0 && m_CurrentTick < (int)m_vRecording.size())
	{
		const STasFrame &Frame = m_vRecording[m_CurrentTick];
		str_format(aBuf, sizeof(aBuf), "Pos: (%.0f, %.0f)", Frame.m_Pos.x, Frame.m_Pos.y);
		TextRender()->Text(x, y, 12.0f, aBuf, -1.0f);
		y += 15.0f;
		
		str_format(aBuf, sizeof(aBuf), "Vel: (%.2f, %.2f)", Frame.m_Vel.x, Frame.m_Vel.y);
		TextRender()->Text(x, y, 12.0f, aBuf, -1.0f);
	}
}

void CTas::ApplyRecordedInputToMainPlayer()
{
	if(m_vRecording.empty())
	{
		Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "tas", "No recording to apply");
		return;
	}
	
	Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "tas", "Applying recording to main player (not implemented yet)");
	// TODO: Применить инпуты к основному игроку
}

// ========== Консольные команды ==========

void CTas::ConRecord(IConsole::IResult *pResult, void *pUserData)
{
	CTas *pSelf = (CTas *)pUserData;
	if(pSelf->m_State == STATE_RECORDING)
		pSelf->StopRecording();
	else
		pSelf->StartRecording();
}

void CTas::ConStop(IConsole::IResult *pResult, void *pUserData)
{
	CTas *pSelf = (CTas *)pUserData;
	pSelf->StopRecording();
	pSelf->StopPlayback();
}

void CTas::ConPlay(IConsole::IResult *pResult, void *pUserData)
{
	CTas *pSelf = (CTas *)pUserData;
	pSelf->StartPlayback();
}

void CTas::ConPause(IConsole::IResult *pResult, void *pUserData)
{
	CTas *pSelf = (CTas *)pUserData;
	pSelf->TogglePause();
}

void CTas::ConRewind(IConsole::IResult *pResult, void *pUserData)
{
	CTas *pSelf = (CTas *)pUserData;
	int Ticks = pResult->NumArguments() > 0 ? pResult->GetInteger(0) : pSelf->Config()->m_MRXTasTickControlTicks;
	pSelf->Rewind(Ticks);
}

void CTas::ConForward(IConsole::IResult *pResult, void *pUserData)
{
	CTas *pSelf = (CTas *)pUserData;
	int Ticks = pResult->NumArguments() > 0 ? pResult->GetInteger(0) : pSelf->Config()->m_MRXTasTickControlTicks;
	pSelf->Forward(Ticks);
}

void CTas::ConStep(IConsole::IResult *pResult, void *pUserData)
{
	CTas *pSelf = (CTas *)pUserData;
	int Direction = pResult->NumArguments() > 0 ? pResult->GetInteger(0) : 1;
	
	if(Direction > 0)
		pSelf->StepForward();
	else
		pSelf->StepBackward();
}

void CTas::ConSave(IConsole::IResult *pResult, void *pUserData)
{
	CTas *pSelf = (CTas *)pUserData;
	if(pResult->NumArguments() > 0)
	{
		pSelf->SaveToFile(pResult->GetString(0));
	}
	else
	{
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "tas", "Usage: tas_save <filename>");
	}
}

void CTas::ConLoad(IConsole::IResult *pResult, void *pUserData)
{
	CTas *pSelf = (CTas *)pUserData;
	if(pResult->NumArguments() > 0)
	{
		pSelf->LoadFromFile(pResult->GetString(0));
	}
	else
	{
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "tas", "Usage: tas_load <filename>");
	}
}

void CTas::ConClear(IConsole::IResult *pResult, void *pUserData)
{
	CTas *pSelf = (CTas *)pUserData;
	pSelf->Clear();
}

void CTas::ConApply(IConsole::IResult *pResult, void *pUserData)
{
	CTas *pSelf = (CTas *)pUserData;
	pSelf->ApplyRecordedInputToMainPlayer();
}
