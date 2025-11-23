#ifndef GAME_CLIENT_COMPONENTS_CHEAT_FEATURES_TAS_H
#define GAME_CLIENT_COMPONENTS_CHEAT_FEATURES_TAS_H

#include <game/client/component.h>
#include <game/client/prediction/gameworld.h>
#include <generated/protocol.h>
#include <engine/console.h>
#include <vector>

class CTas : public CComponent
{
public:
    // Состояния TAS
    enum EState
    {
        STATE_NONE,
        STATE_RECORDING,
        STATE_PLAYING,
        STATE_PAUSED
    };

    // Кадр записи
    struct STasFrame
    {
        CNetObj_PlayerInput m_Input;
        int m_Tick;
        vec2 m_Pos;
        vec2 m_Vel;
        int m_Flags; // Freeze, ground, etc
    };

    // Снапшот для rewind
    struct STasSnapshot
    {
        int m_Tick;
        size_t m_FrameIndex;
        // TODO: Полное состояние персонажа для rewind
    };

private:
    EState m_State;
    std::vector<STasFrame> m_vRecording;
    std::vector<STasSnapshot> m_vSnapshots;
    
    int m_CurrentTick;
    int m_StartTick;
    float m_TickAccumulator;
    int m_SnapshotInterval;
    
    // TAS World для симуляции
    CGameWorld *m_pTasWorld;
    bool m_TasWorldInitialized;
    int m_LocalClientId;

public:
    CTas();
    
    virtual int Sizeof() const override { return sizeof(*this); }
    virtual void OnInit() override;
    virtual void OnReset() override;
    virtual void OnRender() override;
    virtual void OnRelease() override;
    virtual void OnConsoleInit() override;
    
    // Основные методы
    void StartRecording();
    void StopRecording();
    void StartPlayback();
    void StopPlayback();
    void TogglePause();
    
    void Rewind(int Ticks);
    void Forward(int Ticks);
    void StepForward();
    void StepBackward();
    
    bool SaveToFile(const char *pFilename);
    bool LoadFromFile(const char *pFilename);
    void Clear();
    
    // Геттеры
    bool IsActive() const { return m_State != STATE_NONE; }
    bool IsRecording() const { return m_State == STATE_RECORDING; }
    bool IsPlaying() const { return m_State == STATE_PLAYING; }
    bool IsPaused() const { return m_State == STATE_PAUSED; }
    int GetCurrentTick() const { return m_CurrentTick; }
    int GetTotalTicks() const { return (int)m_vRecording.size(); }
    EState GetState() const { return m_State; }

private:
    void InitTasWorld();
    void DestroyTasWorld();
    void RecordFrame();
    void SimulateTick();
    void CreateSnapshot();
    void RestoreSnapshot(int Tick);
    void RenderPath();
    void RenderUI();
    void ApplyRecordedInputToMainPlayer();
    
    // Консольные команды
    static void ConRecord(IConsole::IResult *pResult, void *pUserData);
    static void ConStop(IConsole::IResult *pResult, void *pUserData);
    static void ConPlay(IConsole::IResult *pResult, void *pUserData);
    static void ConPause(IConsole::IResult *pResult, void *pUserData);
    static void ConRewind(IConsole::IResult *pResult, void *pUserData);
    static void ConForward(IConsole::IResult *pResult, void *pUserData);
    static void ConStep(IConsole::IResult *pResult, void *pUserData);
    static void ConSave(IConsole::IResult *pResult, void *pUserData);
    static void ConLoad(IConsole::IResult *pResult, void *pUserData);
    static void ConClear(IConsole::IResult *pResult, void *pUserData);
    static void ConApply(IConsole::IResult *pResult, void *pUserData);
};

#endif
