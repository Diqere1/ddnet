#include "avoidfreeze.h"
#include <game/client/gameclient.h>
#include <engine/shared/config.h>
#include <limits>
#include <cmath>
#include <vector>
#include <algorithm>

struct SActionAF { int m_Dir; };

// Единственная точка определения "опасности" тайла по центру позиции (идентично серверной/клиентской логике тайлов)
static bool IsDangerAtCenter(CGameClient *pClient, const vec2 &Pos)
{
    const bool freezeBit = g_Config.m_MRXAvoidTileFreeze != 0;
    const bool deathBit = g_Config.m_MRXAvoidTileDeath != 0;
    const bool teleBit = g_Config.m_MRXAvoidTileTele != 0;
    const bool unfreezeBit = g_Config.m_MRXAvoidTileUnfreeze != 0;

    CCollision *pCol = pClient->Collision();
    const int MapIndex = pCol->GetMapIndex(Pos);
    if(MapIndex < 0)
        return false;

    const int TileIdx = pCol->GetTileIndex(MapIndex);
    const int FrontTileIdx = pCol->GetFrontTileIndex(MapIndex);

    if(freezeBit && (TileIdx == TILE_FREEZE || FrontTileIdx == TILE_FREEZE || TileIdx == TILE_DFREEZE || FrontTileIdx == TILE_DFREEZE || TileIdx == TILE_LFREEZE || FrontTileIdx == TILE_LFREEZE))
        return true;

    if(deathBit && (TileIdx == TILE_DEATH || FrontTileIdx == TILE_DEATH))
        return true;

    if(teleBit && (pCol->IsTeleport(MapIndex) || pCol->IsEvilTeleport(MapIndex) || pCol->IsCheckTeleport(MapIndex) || pCol->IsCheckEvilTeleport(MapIndex) || pCol->IsTeleportHook(MapIndex) || pCol->IsTeleportWeapon(MapIndex)))
        return true;

    return false;
}

// Симуляция: удерживаем заданное направление NewDir на всём горизонте, а хук отпускаем на ReleaseTick
// Используется для совместного планирования «хук + направление»
static bool SimulateHookReleaseCandidateWithDir(CGameClient *pClient,
                                                CCharacterCore Core,
                                                const CNetObj_PlayerInput &BaseInput,
                                                int ReleaseTick,
                                                int HorizonTicks,
                                                int NewDir,
                                                int *pFirstHitTick)
{
    if(pFirstHitTick)
        *pFirstHitTick = -1;

    for(int i = 0; i < HorizonTicks; ++i)
    {
        CNetObj_PlayerInput StepInput = BaseInput;
        StepInput.m_Direction = NewDir; // держим направление постоянно
        StepInput.m_Hook = (i >= ReleaseTick) ? 0 : 1; // отпускаем на ReleaseTick

        Core.m_Input = StepInput;
        Core.Tick(true, false);
        Core.Move();

        if(IsDangerAtCenter(pClient, Core.m_Pos))
        {
            if(pFirstHitTick)
                *pFirstHitTick = i + 1;
            return true;
        }
    }
    return false;
}

// Поиск «последнего безопасного тика отпускания хука» при фиксированном направлении NewDir
static int FindLastSafeHookReleaseTickWithDir(CGameClient *pClient,
                                              const CCharacterCore &BaseCore,
                                              const CNetObj_PlayerInput &BaseInput,
                                              int HorizonTicks,
                                              int NewDir)
{
    int lastSafe = -1;
    for(int rel = 0; rel <= HorizonTicks; ++rel)
    {
        int firstHitTick = -1;
        bool hit = SimulateHookReleaseCandidateWithDir(pClient, BaseCore, BaseInput, rel, HorizonTicks, NewDir, &firstHitTick);
        if(!hit)
            lastSafe = rel;
    }
    return lastSafe;
}

// Логика «наград/весов» удалена. Решения принимаются по последнему безопасному тику.

static void SimulateScenarioTicks(CGameClient *pClient, CCharacterCore Core, CNetObj_PlayerInput BaseInput, const SActionAF &Action, int Ticks, float TeeRadius, bool &OutHitFreeze, vec2 *pFreezePos, std::vector<vec2> *pTrajectory, float *pFirstHitTicks)
{
    OutHitFreeze = false;
    if(pFirstHitTicks)
        *pFirstHitTicks = std::numeric_limits<float>::infinity();

    for(int i = 0; i < Ticks; ++i)
    {
        BaseInput.m_Direction = Action.m_Dir;
        
        Core.m_Input = BaseInput;
        Core.Tick(true, false);
        Core.Move();
        vec2 newPos = Core.m_Pos;

        {
            vec2 pos = newPos;

            if(pTrajectory)
                pTrajectory->push_back(pos);
            if(IsDangerAtCenter(pClient, pos))
            {
                OutHitFreeze = true;
                if(pFreezePos)
                    *pFreezePos = pos;
                if(pFirstHitTicks)
                {
                    *pFirstHitTicks = (float)(i + 1);
                }
                return;
            }
        }
    }
}

// Симуляция кандидата отпускания хука на заданном тике ReleaseTick (0 = отпустить сразу)
// Возвращает true, если в течение горизонта был задет опасный тайл; false, если траектория безопасна
static bool SimulateHookReleaseCandidate(CGameClient *pClient,
                                         CCharacterCore Core,
                                         const CNetObj_PlayerInput &BaseInput,
                                         int ReleaseTick,
                                         int HorizonTicks,
                                         float TeeRadius,
                                         int *pFirstHitTick)
{
    if(pFirstHitTick)
        *pFirstHitTick = -1;

    for(int i = 0; i < HorizonTicks; ++i)
    {
        CNetObj_PlayerInput StepInput = BaseInput;
        // До момента ReleaseTick держим хук, после — отпускаем
        StepInput.m_Hook = (i >= ReleaseTick) ? 0 : 1;

        Core.m_Input = StepInput;
        Core.Tick(true, false);
        Core.Move();

        if(IsDangerAtCenter(pClient, Core.m_Pos))
        {
            if(pFirstHitTick)
                *pFirstHitTick = i + 1; // 1‑based индекс тика
            return true; // столкнулись с опасностью
        }
    }

    return false; // безопасно на всём горизонте
}

// Возвращает последний безопасный тик отпускания хука в пределах горизонта
// 0 = отпустить прямо сейчас; значение > 0 означает «досчитать столько тиков и отпустить»
// Если безопасно не отпускать в горизонте, вернёт HorizonTicks (т.е. держать хук весь горизонт)
// Если ни один вариант не безопасен (даже 0), вернёт -1
static int FindLastSafeHookReleaseTick(CGameClient *pClient,
                                       const CCharacterCore &BaseCore,
                                       const CNetObj_PlayerInput &BaseInput,
                                       int HorizonTicks,
                                       float TeeRadius)
{
    int lastSafe = -1;

    // Перебираем все варианты отпускания: от «сейчас» до «после горизонта»
    for(int rel = 0; rel <= HorizonTicks; ++rel)
    {
        int firstHitTick = -1;
        bool hit = SimulateHookReleaseCandidate(pClient, BaseCore, BaseInput, rel, HorizonTicks, TeeRadius, &firstHitTick);
        if(!hit)
        {
            // Безопасно на всём горизонте — обновляем последний безопасный
            lastSafe = rel;
        }
    }

    return lastSafe;
}

// Симуляция кандидата смены направления на заданном тике ApplyTick (0 = сменить сейчас)
// Проверка фриза — строго по центру, чтобы не тормозить заранее
static bool SimulateDirectionApplyCandidate(CGameClient *pClient,
                                            CCharacterCore Core,
                                            const CNetObj_PlayerInput &BaseInput,
                                            int ApplyTick,
                                            int NewDir,
                                            int HorizonTicks)
{
    for(int i = 0; i < HorizonTicks; ++i)
    {
        CNetObj_PlayerInput Step = BaseInput;
        Step.m_Direction = (i >= ApplyTick) ? NewDir : BaseInput.m_Direction;
        Core.m_Input = Step;
        Core.Tick(true, false);
        Core.Move();

        if(IsDangerAtCenter(pClient, Core.m_Pos))
            return true; // столкнулись с фризом в горизонте
    }
    return false; // безопасно на всём горизонте
}

// Находит последний безопасный тик применения NewDir в пределах горизонта
// Возвращает -1, если нет безопасного момента; 0 — сменить прямо сейчас; >0 — можно тянуть до этого тика
static int FindLastSafeDirectionApplyTick(CGameClient *pClient,
                                          const CCharacterCore &BaseCore,
                                          const CNetObj_PlayerInput &BaseInput,
                                          int HorizonTicks,
                                          int NewDir)
{
    int lastSafe = -1;
    for(int apply = 0; apply <= HorizonTicks; ++apply)
    {
        bool hit = SimulateDirectionApplyCandidate(pClient, BaseCore, BaseInput, apply, NewDir, HorizonTicks);
        if(!hit)
            lastSafe = apply;
    }
    return lastSafe;
}

void CAvoidFreeze::HandleAvoidFreeze()
{
    if(!GameClient() || !GameClient()->m_Snap.m_pLocalCharacter || !GameClient()->Collision())
        return;

    if(GameClient()->Client()->State() != IClient::STATE_ONLINE)
        return;

    if(!g_Config.m_MRXAvoidEnable)
        return;

    CNetObj_PlayerInput &Input = GameClient()->m_Controls.m_aInputData[g_Config.m_ClDummy];
    int &Direction = Input.m_Direction;

    CCharacterCore BaseCore = GameClient()->m_PredictedChar;
    BaseCore.m_Input = Input;
    BaseCore.Init(&GameClient()->m_PredictedWorld.m_Core, GameClient()->Collision(), &GameClient()->m_Teams);

    // Радиус аппроксимации тира для быстрого зонда (ассист направления)
    const float TeeRadius = 5.0f;

    // 1) Логика отпуска хука с поиском «последнего безопасного тика» в горизонте
    if(g_Config.m_MRXAvoidEnable && g_Config.m_MRXAvoidHookAssist && (Input.m_Hook & 1))
    {
        // Используем отдельный горизонт для хука
        const int HookTicks = std::max(1, g_Config.m_MRXAvoidCheckTicksHook);

        int lastSafeRelease = FindLastSafeHookReleaseTick(GameClient(), BaseCore, Input, HookTicks, TeeRadius);

        if(lastSafeRelease >= 0)
        {
            // Если безопасно отпустить прямо сейчас — отпускаем
            // Иначе — продолжаем держать хук (отпустим ближе к «последнему безопасному тіку» при последующих пересчётах)
            if(lastSafeRelease == 0)
                Input.m_Hook = 0;
            else
                Input.m_Hook = 1;
        }
    }

    // 2) Быстрый зонд вперёд (для ассиста направления): несколько шагов с окружностной проверкой, чтобы ловить углы
    bool DangerAhead = false;
    {
        const int kSteps = 10;
        for(int s = 1; s <= kSteps && !DangerAhead; ++s)
        {
            float factor = (float)s / (float)kSteps;
            vec2 probe = BaseCore.m_Pos + BaseCore.m_Vel * (1.0f / 50.0f) * (6.0f * factor);
            const int kNumCirclePoints = 16;
            for(int c = 0; c < kNumCirclePoints; ++c)
            {
                float ang = 2.0f * pi * (float)c / (float)kNumCirclePoints;
                vec2 checkPos = probe + vec2(cosf(ang), sinf(ang)) * TeeRadius;
                if(IsDangerAtCenter(GameClient(), checkPos))
                {
                    DangerAhead = true;
                    break;
                }
            }
        }
    }

    const bool WantDirAssist = g_Config.m_MRXAvoidDirectionAssist != 0;
    const bool HookAssist = g_Config.m_MRXAvoidHookAssist != 0;
    const bool HookPressed = (Input.m_Hook & 1) != 0;
    const bool WantHookPlan = HookAssist && HookPressed;

    if(!DangerAhead && !WantDirAssist && !WantHookPlan)
        return;

    // Горизонт предсказания: используем строго значение из конфига без адаптации по скорости
    int BaseTicks = std::max(1, g_Config.m_MRXAvoidCheckTicks);
    int PredictTicks = BaseTicks;

    // Горизонт — только как «окно планирования», не повод тормозить заранее
    // Кандидаты направлений: оставить, остановиться, уйти влево, уйти вправо
    std::vector<int> DirCandidates;
    DirCandidates.push_back(Input.m_Direction);
    DirCandidates.push_back(0);
    DirCandidates.push_back(-1);
    DirCandidates.push_back(1);

    // Для каждого кандидата направления найдём «последний безопасный тик применения»
    int bestDir = Input.m_Direction;
    int bestLastSafe = -2;        // объединённая метрика
    int bestLastSafeDir = -2;     // отдельно по направлению
    int bestLastSafeHook = -2;    // отдельно по хуку (если актуально)
    int bestRank = -1;           // тай-брейк: 2 = текущее, 1 = стоп, 0 = прочее

    for(size_t i = 0; i < DirCandidates.size(); ++i)
    {
        int newDir = DirCandidates[i];
        // Если ассист направления выключен — менять нельзя
        if(!WantDirAssist && newDir != Input.m_Direction)
            continue;

        int lastSafeDir = FindLastSafeDirectionApplyTick(GameClient(), BaseCore, Input, PredictTicks, newDir);
        int lastSafeHook = -1;
        if(WantHookPlan)
        {
            const int HookTicks = std::max(1, g_Config.m_MRXAvoidCheckTicksHook);
            // Оцениваем отпускание хука при удержании этого направления
            lastSafeHook = FindLastSafeHookReleaseTickWithDir(GameClient(), BaseCore, Input, HookTicks, newDir);
        }

        // Объединяем: берём минимум, т.к. оба условия должны быть безопасны
        int combined = lastSafeDir;
        if(WantHookPlan)
        {
            if(lastSafeHook < 0)
                combined = -1; // нет безопасного режима с хуком
            else if(lastSafeDir < 0)
                combined = -1; // нет безопасного по направлению
            else
                combined = std::min(lastSafeDir, lastSafeHook);
        }

        // Простой тай-брейк без «весов»: предпочитаем сохранить ввод игрока,
        // затем остановку, затем всё остальное
        int rank = (newDir == Input.m_Direction) ? 2 : (newDir == 0 ? 1 : 0);

        if(combined > bestLastSafe || (combined == bestLastSafe && rank > bestRank))
        {
            bestLastSafe = combined;
            bestDir = newDir;
            bestRank = rank;
            bestLastSafeDir = lastSafeDir;
            bestLastSafeHook = lastSafeHook;
        }
    }

    // Применяем смену направления с буфером до опасного тика.
    // lastSafe = firstDanger - 1; применяем, когда lastSafe <= (kBufferTicks - 1).
    const int kBufferTicks = 3;
    if(WantDirAssist && bestLastSafeDir >= 0 && bestLastSafeDir <= (kBufferTicks - 1))
    {
        Direction = bestDir;
    }

    // Применяем решение по хуку: отпускаем только если безопасно прямо сейчас
    if(WantHookPlan && bestLastSafeHook >= 0)
    {
        if(bestLastSafeHook == 0)
            Input.m_Hook = 0;
        else
            Input.m_Hook = 1;
    }
}
